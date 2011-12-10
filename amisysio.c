/* amisysio.c */

/*-
 *	Mike Rieser 				Dale Rahn
 *	2410 Happy Hollow Rd. Apt D-10		540 Vine St.
 *	West Lafayette, IN 47906 		West Lafayette, IN 47906
 *	riesermc@mentor.cc.purdue.edu		rahn@sage.cc.purdue.edu
 */

#if AZTEC_C

/*
 * This file is only to supply behavior a little closer to UNIX for the Aztec
 * Library functions stat() and creat().
 * 
 * If you don't have a decent stat() function move the above #if to the
 * end of the stat() function.
 *
 * The creat() function is pretty Aztec specific.
 */

#include "amistat.h"
#include <fcntl.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#if AZTEC_C
#include <pragmas/exec_lib.h>
#include <pragmas/dos_lib.h>
#else
#include <pragmas/exec.h>
#include <pragmas/dos.h>
#endif

/*-
 *  struct InfoData {
 *     LONG   id_NumSoftErrors;     // number of soft errors on disk
 *     LONG   id_UnitNumber;        // Which unit disk is (was) mounted on
 *     LONG   id_DiskState;         // See defines below
 *     LONG   id_NumBlocks;         // Number of blocks on disk
 *     LONG   id_NumBlocksUsed;     // Number of block in use
 *     LONG   id_BytesPerBlock;
 *     LONG   id_DiskType;          // Disk Type code
 *     BPTR   id_VolumeNode;        // BCPL pointer to volume node
 *     LONG   id_InUse;             // Flag, zero if not in use
 * }; // InfoData
 * returned by Info(), must be on a 4 byte boundary
*/
/*-
 *  struct FileInfoBlock {
 *     LONG   fib_DiskKey;
 *     LONG   fib_DirEntryType;  // Type of Directory. If < 0, then a plain file.
 *		  		 // If > 0 a directory
 *     char   fib_FileName[108]; // Null terminated. Max 30 chars used for now
 *     LONG   fib_Protection;    // bit mask of protection, rwxd are 3-0.
 *     LONG   fib_EntryType;
 *     LONG   fib_Size;          // Number of bytes in file
 *     LONG   fib_NumBlocks;     // Number of blocks in file
 *     struct DateStamp fib_Date;// Date file last changed
 *     char   fib_Comment[80];   // Null terminated comment associated with file
 *     char   fib_Reserved[36];
 *  }; // FileInfoBlock
 * filled by Examin(), must be on a 4 byte boundary
*/

int
stat(char   *path, struct stat *statbuf)
{
    struct FileLock *lock;
    struct FileInfoBlock *pFIB;
    struct InfoData *pID;
    int          success = 0;

    /* Zero the stat buffer. */
    memset(statbuf, '\000', sizeof(struct stat));

    /* Get Lock */
    lock = (struct FileLock *) Lock((UBYTE *) path, ACCESS_READ);
    if ((struct FileLock *) 0 == lock)
    {					/* Lock() fails if file is a softlink */
	if (ERROR_IS_SOFT_LINK == IoErr())
	{
	    statbuf->st_mode |= S_IFLNK;/* symbolic link */
	    return 0;
	} else
	    return -1;
    }
    /* Allocate InfoData */
    pID = (struct InfoData *) AllocMem(sizeof(struct InfoData), 0);

    if ((struct InfoData *) 0 == pID)
    {
	UnLock((BPTR) lock);
	return -1;
    }
    /* Allocate FileInfoBlock */
    pFIB = (struct FileInfoBlock *) AllocMem(sizeof(struct FileInfoBlock), 0);

    if ((struct FileInfoBlock *) 0 == pFIB)
    {
	FreeMem(pID, sizeof(struct InfoData));

	UnLock((BPTR) lock);
	return -1;
    }
    /* Fill InfoData */
    if (DOSFALSE == Info((BPTR) lock, pID))
    {					/* Not critical */
	FreeMem(pID, sizeof(struct InfoData));

	pID = (struct InfoData *) 0;
    }
    /* Fill FileInfoBlock */
    if (DOSFALSE == Examine((BPTR) lock, pFIB))
    {
	FreeMem(pID, sizeof(struct InfoData));
	FreeMem(pFIB, sizeof(struct FileInfoBlock));

	UnLock((BPTR) lock);
	return -1;
    }
    statbuf->st_ino = pFIB->fib_DiskKey;/* inode's number */
    ++statbuf->st_nlink;

    if (pFIB->fib_DirEntryType < 0)
    {					/* plain file */
	statbuf->st_mode |= S_IFREG;
    } else
    {					/* >= 0 then directory */
	statbuf->st_mode |= S_IFDIR;
    }
    if (pFIB->fib_DirEntryType == ST_SOFTLINK)
    {
	statbuf->st_mode |= S_IFLNK;
    }
    if (pFIB->fib_DirEntryType == ST_SOFTLINK
	|| pFIB->fib_DirEntryType == ST_LINKDIR
	|| pFIB->fib_DirEntryType == ST_LINKFILE)
    {
	++statbuf->st_nlink;
    }
    statbuf->st_flags = statbuf->st_attr = pFIB->fib_Protection;

    /* mask off arwed -> rwx and shift to owner bits */
    statbuf->st_mode |= ((~0 ^ pFIB->fib_Protection) & 016) << 5;

    statbuf->st_size = pFIB->fib_Size;
    if (pID)
	statbuf->st_blksize = pID->id_BytesPerBlock;	/* optimal blocksize for
							 * I/O */
    statbuf->st_blocks = pFIB->fib_NumBlocks;	/* actual number of blocks
						 * allocated */
    statbuf->st_atime =
	statbuf->st_mtime =
	statbuf->st_ctime = pFIB->fib_Date.ds_Days * 24 * 60 * 60 +
	pFIB->fib_Date.ds_Minute * 60 +
	pFIB->fib_Date.ds_Tick / TICKS_PER_SECOND;

    UnLock((BPTR) lock);
    if (pID)
	FreeMem(pID, sizeof(struct InfoData));
    FreeMem(pFIB, sizeof(struct FileInfoBlock));

    return 0;
}


/*
 * Aztec creat() replacement.
 * 
 * This one doesn't delete the file, thereby preserving the original file
 * protection bits!
 */
int
creat(const char *name, int mode)
{
    int          c = 0;
    BPTR         fh;

    if (isOldDOS())
	return (_creat(name, mode));

    fh = Open((UBYTE *) name, MODE_READWRITE);
    if ((BPTR) 0 == fh)
	return (_open(name, O_WRONLY | O_TRUNC | O_CREAT, mode));
    SetFileSize(fh, 0, OFFSET_BEGINNING);	/* Set back to beginning */
    Close(fh);				/* Truncate at the start */

    return (_open(name, O_WRONLY, mode));	/* actually get a fd */
}

#endif
