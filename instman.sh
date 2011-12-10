#!/bin/sh
#
# This shell script is used on UNIX-like systems to install man-pages.
# It is run automatically during "make install".
#
#   usage: sh install.sh {program names}
#
################################################################################

# This is a list of directories where the nroff source for man-pages might
# be installed.  The order is important -- the man-pages will be installed
# in the first existing directory in the list.
MANDIRS="/usr/man/man.l /usr/man/man.LOCAL /usr/man/man.1 /usr/man/1
/usr/catman.C /usr/man/manl /usr/man/man1 /usr/elvis/man"

# Similarly, this is a list of directories where the plaintext form of the
# man-pages might belong.  If, after searching, it is decided that both forms
# should go in the same directory, then only the plain-text version is
# installed.
DOCDIRS="/usr/man/catmanl /usr/man/catman.l /usr/man/catman.LOCAL
/usr/man/cat.LOCAL /usr/man/catman.1 /usr/man/cat.C /usr/catman.C
/usr/catman/1 /usr/man/1 /usr/man/cat1"

################################################################################

# Complain if no programs were named on the command line
if test $# -lt 1
then
	echo "Usage: sh instman.sh {program names...}"
	echo "     where {program names...} is a list of programs whose"
	echo "     manual pages are to be installed.  It is assumed that"
	echo "     the manual pages are currently located inside the 'doc'"
	echo "     subdirectory."
	echo
	echo "     Note: Normally, this script is executed via 'make install'"
	exit 1
fi

# The installed manpages should be readable by everybody, but writable only
# by us.
umask 022

# Decide where to put the nroff source docs
mandir=`for dir in $MANDIRS
	do
		if test -d $dir
		then
			echo $dir
			exit 0
		fi
	done`
anydir=$mandir

# Decide where to put the plaintext docs
docdir=`for dir in $DOCDIRS
	do
		if test -d $dir
		then
			echo $dir
			exit 0
		fi
	done`
if test "$docdir"
then
	anydir=$docdir
fi

# If we didn't find a directory for either of them, then complain & quit
if test -z "$anydir"
then
	echo "::: I CAN'T AUTOMATICALLY INSTALL THE MAN PAGES ON THIS SYSTEM"
	echo "::: BECAUSE I DON'T KNOW WHERE TO PUT THEM!  YOU CAN EITHER"
	echo "::: TRY EDITING THE "instman.sh" SCRIPT & RERUNNING IT, OR"
	echo "::: SIMPLY INSTALL THEM YOURSELF BY HAND."
	exit 1
fi

# If they're the same, then we only want to install the plaintext versions
if test "$mandir" = "$docdir"
then
	mandir=
fi

# Choose the installed filename extension by looking at the name of the
# directory we'll be installing it into.
case "$anydir" in
  *.LOCAL)	ext=.LOCAL	;;
  *.L)		ext=.L		;;
  *l)		ext=.l		;;
  *1)		ext=.1		;;
  *)		ext=		;;
esac

# If we're going to be installing the nroff source, then do it now
if test "$mandir"
then
	for i
	do
		cp doc/$i.man $mandir/$i$ext
	done
fi

# Sleep briefly, to ensure that the plaintext versions of the files will
# be installed with a newer timestamp than the nroff versions.
sleep 1

# If we're going to be installing the plaintext pages, then do it now
if test "$docdir"
then
	for i
	do
		cp doc/$i.doc $docdir/$i$ext
	done
fi

# Are man pages sometimes compressed or packed on this system?  If so,
# then give these new pages the same treatment.
for dir in $mandir $docdir
do
	if test "`echo $dir/*.Z`" != "$dir/*.Z"
	then
		for i
		do
			rm -f $dir/$i$ext.Z
			compress $dir/$i$ext
		done
		finalext=$ext.Z
	elif test "`echo $dir.Z/*`" != "$dir.Z/*"
	then
		for i
		do
			compress <$dir/$i$ext >$dir.Z/$i$ext && rm -f $dir/$i$ext
		done
		finalext=$ext
	elif test "`echo $dir/*.z`" != "$dir/*.z"
	then
		for i
		do
			rm -f $dir/$i$ext.z
			pack $dir/$i$ext
		done
		finalext=$ext.z
	elif test "`echo $dir/*.gz`" != "$dir/*.gz"
	then
		for i
		do
			rm -f $dir/$i$ext.z
			gzip $dir/$i$ext
		done
		finalext=$ext.gz
	else
		finalext=$ext
	fi
done

# Say what was done
if test "$mandir"
then
	echo "nroffable manpages installed as $mandir/progname$finalext"
fi
if test "$docdir"
then
	echo "plaintext manpages installed as $docdir/progname$finalext"
fi
