#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_

#include <time.h>
#include <exec/types.h>

struct  stat
{
	WORD	st_dev;		/* device number for dev containing file */
	ULONG	st_ino;		/* inode's number */
	ULONG	st_mode;	/* file type and perms */
	WORD	st_nlink;	/* number of hard links for the file */
	WORD	st_uid;		/* user ID of the file's owner */
	WORD	st_gid;		/* group ID of the file's group */
	WORD	st_rdev;	/* major and minor device numbers */
	ULONG	st_size;	/* total size of file in bytes */
	time_t	st_atime;	/* time of last access */
	time_t	st_mtime;	/* time of last modification */
	time_t	st_ctime;	/* time of last status change */
	LONG	st_blksize;	/* optimal blocksize for I/O */
	LONG	st_blocks;	/* actual number of blocks allocated */
	LONG	st_rsize;
	LONG	st_flags;	/* user defined flags for file */
	LONG	st_gen;		/* file generation number */
	LONG	st_attr;	/* protection bits from AZTEC_C */
};

#define S_IFMT  	0170000	/* type of file */
#define	      S_IFDIR	0040000	/* directory */
#define	      S_IFCHR	0020000	/* character special */
#define	      S_IFBLK	0060000	/* block special */
#define	      S_IFREG	0100000	/* regular */
#define	      S_IFLNK	0120000	/* symbolic link */
#define	      S_IFSOCK	0140000	/* socket */
#define	      S_IFIFO	0010000	/* FIFO (named pipe) */
#define S_ISUID		0004000	/* set user id on execution */
#define S_ISGID		0002000	/* set group id on execution */
#define S_ISVTX		0001000	/* save swapped text even after use */
#define S_IREAD		0000400	/* read permission, owner */
#define S_IWRITE	0000200	/* write permission, owner */
#define S_IEXEC		0000100	/* execute/search permission, owner */

#if    __STDC__
extern int stat(const char *, struct stat *);
extern int fstat(int, struct stat *);
#endif /* __STDC__ */

#endif  _SYS_STAT_H_
