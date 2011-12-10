/* session.c */
/* Copyright 1995 by Steve Kirkendall */

char id_session[] = "$Id: session.c,v 2.15 1995/10/19 21:35:10 steve Exp steve $";

#include "elvis.h"

/* This file contains the session functions, which implement a cache and
 * block-locking semantics on the session file.
 */

/*----------------------------------------------------------------------------*/
/* session block cache                                                        */

typedef struct blkcache_s
{
	struct blkcache_s *next;	/* another block with same hash value */
	struct blkcache_s *older,*newer;/* the next-older block in the cache */
	COUNT		  locks;	/* lock counter */
	BOOLEAN		  dirty;	/* does the block need to be rewritten? */
	BLKNO		  blkno;	/* block number of this block */
	BLKTYPE		  blktype;	/* type of data in this block */
	BLK		  *buf;		/* contents of the block */
#ifdef DEBUG_SESSION
	char		  *lockfile[5];	/* name of source file that locked block */
	int		  lockline[5];	/* line number in source file */
#endif
} CACHEENTRY;


#if USE_PROTOTYPES
static void delcache(CACHEENTRY *item, BOOLEAN thenfree);
static void addcache(CACHEENTRY *item);
static CACHEENTRY *findblock(_BLKNO_ blkno);
static void flushblock(CACHEENTRY *bc);
#endif


static long	oldblkhash;	/* previous value of o_blkhash option */
static CACHEENTRY **hashed;	/* hash table */
static CACHEENTRY *newest;	/* pointer to newest item in cache */
static CACHEENTRY *oldest;	/* pointer to oldest item in cache */
static int	ncached;	/* number of items in cache */
static COUNT	*alloccnt;	/* array of allocation counts per block */
static int	nblocks;	/* size of alloccnt array */


/* This function deletes an item from the block cache.  Optionally, it will
 * also free the item.
 */
static void delcache(item, thenfree)
	CACHEENTRY	*item;		/* cache item to be removed from cache */
	BOOLEAN		thenfree;	/* if True, item is freed after removal */
{
	CACHEENTRY *scan, *lag;
	int	 i;

	assert(item != NULL && (item->locks == 0 || !thenfree));

	/* if the item is dirty & we're completely deleting it, then flush it */
	if (thenfree && item->dirty)
	{
		blkwrite(item->buf, item->blkno);
	}

	/* delete the item from the newest/oldest list */
	if (item == newest)
	{
		newest = item->older;
	}
	else
	{
		item->newer->older = item->older;
	}
	if (item == oldest)
	{
		oldest = item->newer;
	}
	else
	{
		item->older->newer = item->newer;
	}

	/* delete the item from the hashed list */
	i = item->blkno % o_blkhash;
	if (hashed[i] == item)
	{
		hashed[i] = item->next;
	}
	else
	{
		for (lag = hashed[i], scan = lag->next; scan != item; lag = scan, scan = scan->next)
		{
			assert(scan != NULL);
		}
		lag->next = scan->next;
	}

	/* if we're supposed to free it, do that */
	if (thenfree)
	{
		safefree(item->buf);
		safefree(item);
	}
	else
	{
		item->older = item->newer = NULL;
	}

	/* and count it */
	ncached--;
}

/* This function adds an item to the "newest" end of the block cache, and also
 * the hash list.  Before it does this, it checks the overall size of the cache
 * and if it has reached the maximum, it tries to delete the oldest unlocked
 * item.
 */
static void addcache(item)
	CACHEENTRY	*item;	/* item to be added to cache */
{
	CACHEENTRY *scan;
	int	 i;

	/* if this would push the cache past its limit, then try to delete
	 * the oldest unlocked block.
	 */
	while (ncached >= o_blkcache)
	{
		for (i = 0, scan = oldest; scan && scan->locks > 0; scan = scan->newer, i++)
		{
		}
		if (scan)
		{
			delcache(scan, True);
		}
		else
		{
			/* cache size will exceed blkcache... no big deal */
#ifdef DEBUG_SESSION
			fprintf(stderr, "%d blocks locked\n", i);
#endif
			break;
		}
	}

	/* if this is the first item in the cache, then this is "oldest" */
	if (!oldest)
	{
		oldest = item;
	}

	/* insert this item at the "newest" end of the cache list */
	item->older = newest;
	if (newest) newest->newer = item;
	newest = item;

	/* also insert it into the hash table list */
	i = item->blkno % o_blkhash;
	item->next = hashed[i];
	hashed[i] = item;

	/* also count it */
	ncached++;
}

/* Find a block in the cache.  If the block isn't in the cache, return NULL */
static CACHEENTRY *findblock(blkno)
	_BLKNO_	blkno;	/* physical block numbe of block to find */
{
	CACHEENTRY *scan;
	int	 i;

	/* if newest item in cache, return it in a hurry! */
	if (newest && newest->blkno == blkno)
	{
		return newest;
	}

	/* reallocate the hash table if first call or blkhash has changed */
	if (!hashed || o_blkhash != oldblkhash)
	{
		/* free the old hash table, if any */
		if (hashed)
		{
			safefree(hashed);
		}

		/* allocate a new hash table */
		hashed = (CACHEENTRY **)safekept((int)o_blkhash, sizeof(CACHEENTRY *));

		/* put each cached block into the appropriate hash slot */
		for (scan = oldest; scan; scan = scan->newer)
		{
			i = scan->blkno % o_blkhash;
			scan->next = hashed[i];
			hashed[i] = scan;
		}

		oldblkhash = o_blkhash;
	}

	/* search for the block */
	for (scan = hashed[blkno % o_blkhash];
	     scan && scan->blkno != blkno;
	     scan = scan->next)
	{
	}

	/* if found, move it to the newest end of the cache list */
	if (scan)
	{
		delcache(scan, False);
		addcache(scan);
		return scan;
	}

	return NULL;
}

/*----------------------------------------------------------------------------*/
/* Open a session file.  For any error, issue an error message and
 * exit without ever returning.
 */
void sesopen(force)
	BOOLEAN	force;		/* if True, open even if "in use" flag is set */
{
	BLK	*tmp;

	/* allocate a temporary buffer for the superblock */
	tmp = (BLK *)safealloc((int)o_blksize, sizeof(char));
	tmp->super.magic = SESSION_MAGIC;
	tmp->super.blksize = (COUNT)o_blksize;

	/* open the session file */
	if (!blkopen(force, tmp))
	{
		msg(MSG_FATAL, "already in use");
	}

	/* use the block size denoted in the superblock */
	o_blksize = tmp->super.blksize;
	o_blkfill = SES_MAXCHARS * 9/10;
	/* ToDo: limit range of blkfill values to "1:SES_MAXCHARS" */

	/* free the superblock buffer */
	safefree(tmp);

	/* allocate an initial allocation count table.  It'll grow later. */
	alloccnt = (COUNT *)safealloc(1, sizeof(COUNT));
	nblocks = 1;
	alloccnt[0] = 1; /* so superblock is always allocated */
}

/* Flush all dirty blocks in the cache, and then close the session
 * file.  Elvis calls this just before exiting.
 */
void sesclose()
{
	BLK	 *tmp;

	/* flush any dirty blocks */
	sessync();

	/* close the session file */
	tmp = (BLK *)safealloc((int)o_blksize, sizeof(char));
	blkclose(tmp);
	safefree(tmp);
}

/*----------------------------------------------------------------------------*/
/* Read the requested block into the cache, and lock it.  Return
 * a pointer to the block's data in the cache.
 */
#ifdef DEBUG_SESSION
BLKNO _seslock(file, line, blkno, forwrite, blktype)
	char	*file;		/* name of source file that called this func */
	int	line;		/* line number of source file */
#else
BLKNO seslock(blkno, forwrite, blktype)
#endif
	_BLKNO_	blkno;		/* BLKNO of block to be locked */
	BOOLEAN	forwrite;	/* if True, lock it for writing */
	BLKTYPE	blktype;	/* type of data in the block */
{
	CACHEENTRY *bc, *newp;

	assert((int)blkno < nblocks && alloccnt[blkno] > 0);

	/* try to find the block in the cache */
	bc = findblock(blkno);

	/* if not in the cache, then read it into a new cache item */
	if (!bc)
	{
		bc = (CACHEENTRY *)safekept(1, sizeof(CACHEENTRY));
		bc->buf = (BLK *)safekept((int)o_blksize, sizeof(char));
		bc->blkno = blkno;
		bc->blktype = blktype;
		blkread(bc->buf, bc->blkno);
		addcache(bc);
	}

	/* if for write, and its allocation count is greater than 1, then
	 * "copy on write" means we have to copy this block right now.
	 */
	if (forwrite && alloccnt[blkno] > 1)
	{
		/* copy-on-write should only be necessary for CHARS blocks */
		assert(blktype == SES_CHARS);

		/* decrement the allocation count of the old block */
		alloccnt[blkno]--;

		/* allocate a new block */
		blkno = sesalloc(0);
		newp = findblock(blkno);
		if (!newp)
		{
			newp = (CACHEENTRY *)safealloc(1, sizeof(CACHEENTRY));
			newp->buf = (BLK *)safealloc((int)o_blksize, sizeof(char));
			newp->blkno = blkno;
			newp->blktype = blktype;
			addcache(newp);
		}

		/* copy the old block's contents into the new block */
		memcpy(newp->buf, bc->buf, (size_t)o_blksize);
		newp->dirty = True;
		bc = newp;
	}

	/* mark it as being locked */
#ifdef DEBUG_SESSION
	bc->lockfile[bc->locks] = file;
	bc->lockline[bc->locks] = line;
#endif
	bc->locks++;

	/* NOTE: I'd like to trap cases where a single block is locked
	 * repeatedly.  Unfortunately, the recursive nature of the regexp
	 * matcher causes a scanned block to be locked once for each
	 * metacharacter so I can't put a hard limit on it.  I'll pretend
	 * I can, and I'll make it a large limit so nobody is likely to
	 * trip it up with a complex regexp.
	 */
	assert(bc->locks <= 30);

	/* return the data */
	return blkno;
}

/* Return a pointer to the start of a block's data in the cache */
BLK *sesblk(blkno)
	_BLKNO_	blkno;	/* BLKNO of desired block */
{
	CACHEENTRY *bc;

	bc = findblock(blkno);
	assert(bc != NULL && bc->locks > 0);
	return bc->buf;
}


/* Release the lock on a block, so that it can be flushed out to the
 * session file.
 */
void sesunlock(blkno, forwrite)
	_BLKNO_	blkno;		/* BLKNO of block to be unlocked */
	BOOLEAN	forwrite;	/* if True, set the block's "dirty" flag */
{
	CACHEENTRY *bc;

	bc = findblock(blkno);
	assert(bc != NULL && bc->locks > 0 && bc->blkno == blkno);
	bc->dirty |= forwrite;
	bc->locks--;
#ifdef DEBUG_SESSION
	if (forwrite)
	{
		blkwrite(bc->buf, bc->blkno);
	}
#endif
}

/*----------------------------------------------------------------------------*/

static void flushblock(bc)
	CACHEENTRY	*bc;	/* cache item to be flushed */
{
	blkwrite(bc->buf, bc->blkno);
	bc->dirty = False;
}

/* If the block is dirty, write it out to the session file. */
void sesflush(blkno)
	_BLKNO_	blkno;	/* BLKNO of a block to be flushed */
{
	CACHEENTRY *bc;

	/* find the block */
	bc = findblock(blkno);
	assert(bc != NULL && bc->locks == 0);

	/* if BYTES or BLKLIST, and not dirty, then don't bother */
	if ((bc->blktype == SES_CHARS || bc->blktype == SES_BLKLIST)
	    && !bc->dirty)
	{
		return;
	}

	flushblock(bc);
}

/* flush every dirty block in the cache */
void sessync()
{
	CACHEENTRY *bc;

	safeinspect();

	/* for each block... */
	for (bc = oldest; bc; bc = bc->newer)
	{
		assert(bc->locks == 0);

		/* if dirty, flush it */
		if (bc->dirty)
		{
			flushblock(bc);
		}
	}

	/* maybe force the changes out to disk */
	if (o_sync)
	{
		blksync();
	}
}

/*----------------------------------------------------------------------------*/


/* Allocate a new block (if blkno is 0) or increment the allocation
 * count on an existing block (if blkno is not 0).  Returns its BLKNO.
 */
BLKNO sesalloc(blkwant)
	_BLKNO_	blkwant;	/* 0 usually, else BLKNO to use */
{
	BLKNO	blkno;
	int	newsize;
	COUNT	*newarray;
	BLK	*tmp;
	int	i;

	/* if we're supposed to choose a block, then choose one */
	if (blkwant == 0)
	{
		for (blkno = 1; blkno < nblocks && alloccnt[blkno] > 0; blkno++)
		{
		}
	}
	else
	{
		blkno = blkwant;
	}

	/* if past the end of the current alloccnt array, then grow */
	if (blkno >= nblocks)
	{
		/* reallocate the alloccnt array */
		newsize = blkno + o_blkgrow - (blkno % o_blkgrow);
		assert(newsize > blkno);
		newarray = (COUNT *)safekept(newsize, sizeof(COUNT));
		for (i = 0; i < nblocks; i++)
		{
			newarray[i] = alloccnt[i];
		}
		safefree(alloccnt);
		alloccnt = newarray;
		nblocks = newsize;

		/* if new block requested, write dummy data into the session file */
		if (blkwant == 0)
		{
			tmp = (BLK *)safealloc((int)o_blksize, sizeof(char));
			for (i = blkno; i < nblocks; i++)
			{
				blkwrite(tmp, (BLKNO)i);
			}
			safefree(tmp);
		}

		/* increment the allocation counter for the chosen block */
		alloccnt[blkno]++;
	}
	else /* recycling an old block */
	{
		/* increment the allocation counter for the chosen block */
		alloccnt[blkno]++;

		/* if block is supposed to be new, then zero it */
		if (blkwant == 0)
		{
			(void)seslock(blkno, True, SES_NEW);
			tmp = sesblk(blkno);
			memset((char *)tmp, 0, (size_t)o_blksize);
			sesunlock(blkno, True);
		}
	}

	return blkno;
}

/* Decrement the allocation count of a block.  If the block's count
 * is decremented to 0, it becomes available for reuse by sesalloc.
 */
void sesfree(blkno)
	_BLKNO_	blkno;	/* BLKNO of a block to be returned to free pool */
{
	assert((int)blkno < nblocks && alloccnt[blkno] > 0);
	alloccnt[blkno]--;
}
