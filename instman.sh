#!/bin/sh
#
# This shell script is used on UNIX-like systems to install or remove
# man-pages.  It is run automatically during "make install".
#
#   usage: sh install.sh [-u] [-bbindir] programs...
#
################################################################################

# This is a list of directories where the nroff source for man-pages might
# be installed.  The order is important -- the man-pages will be installed
# in the first existing directory in the list.  If "-b /usr/local/bin" is
# an argument, then /usr/local/man/man1 is added to the front of the list.
MANDIRS="/usr/man/man.l /usr/man/man.LOCAL /usr/man/man.1 /usr/man/1
/usr/catman.C /usr/man/manl /usr/man/man1 /usr/elvis/man"

# Similarly, this is a list of directories where the plaintext form of the
# man-pages might belong.  If, after searching, it is decided that both forms
# should go in the same directory, then only the plain-text version is
# installed.  If "-b /usr/local/bin" is an argument, then /usr/local/man/cat1
# is added to the front of the list.
CATDIRS="/usr/man/catmanl /usr/man/catman.l /usr/man/catman.LOCAL
/usr/man/cat.LOCAL /usr/man/catman.1 /usr/man/cat.C /usr/catman.C
/usr/catman/1 /usr/man/1 /usr/man/cat1"

################################################################################

# Look for a "-r"
if test "$1" = "-r"
then
	shift
	job="remove"
else
	job="install"
fi

# Look for a "-bbindir" argument
case "$1" in
  -b*/bin)
	MANDIRS=`echo "X$1" | sed 's/X-b\(.*\)\/bin$/\1\/man\/man1 \1\/man\/man.1 \1\/man\/man.C/'`" clubland $MANDIRS"
	CATDIRS=`echo "X$1" | sed 's/X-b\(.*\)\/bin$/\1\/man\/cat1 \1\/man\/catman1 \1\/man\/cat.1 \1\/man\/cat.C/'`" clubland $CATDIRS"
	shift
	;;
  -b)
	shift
	MANDIRS=`echo "X$1" | sed 's/X\(.*\)\/bin$/\1\/man\/man1 \1\/man\/man.1 \1\/man\/man.C/'`" clubland $MANDIRS"
	CATDIRS=`echo "X$1" | sed 's/X\(.*\)\/bin$/\1\/man\/cat1 \1\/man\/catman1 \1\/man\/cat.1 \1\/man\/cat.C/'`" clubland $CATDIRS"
	shift
	;;
  -b*)
	shift
	;;
esac

# Complain if no programs were named on the command line
if test $# -lt 1
then
	echo "Usage: sh instman.sh [-r] [-b bindir] {program names...}"
	echo "     where {program names...} is a list of programs whose"
	echo "     manual pages are to be installed.  It is assumed that"
	echo "     the manual pages are currently located inside the 'lib'"
	echo "     subdirectory."
	echo
	echo "        -r         Remove man-pages (else install them)"
	echo "        -b bindir  Take a hint for possible man directory from"
	echo "                   the name of the bin directory"
	echo
	echo "     Note: Normally, this script is executed via 'make install'"
	echo "           or 'make uninstall'"
	exit 1
fi

# The installed manpages should be readable by everybody, but writable only
# by us.
umask 022

# Decide where to put the nroff source docs
manclubland=yes
eval `for dir in $MANDIRS
	do
		if test $dir = clubland
		then
			echo manclubland=no
		elif test -d $dir -a -w $dir
		then
			echo mandir=$dir
			exit 0
		fi
	done`
anydir=$mandir

# Decide where to put the plaintext docs
catclubland=yes
eval `for dir in $CATDIRS
	do
		if test $dir = clubland
		then
			echo catclubland=no
		elif test -d $dir -a -w $dir
		then
			echo catdir=$dir
			exit 0
		fi
	done`

# Both names should be either in the list derived from "-b dir" or on the
# default list.  If we have a mixture, then ignore the catdir.
if test -n "$mandir" -a -n "$catdir" -a $manclubland != $catclubland
then
	catdir=""
elif test -n "$catdir"
then
	anydir=$catdir
fi

# If we didn't find a directory for either of them, then complain & quit
if test -z "$anydir"
then
	echo "::: I CAN'T AUTOMATICALLY INSTALL THE MAN PAGES ON THIS SYSTEM"
	echo "::: BECAUSE I DON'T KNOW WHERE TO PUT THEM!  YOU CAN EITHER"
	echo "::: TRY EDITING THE \"instman.sh\" SCRIPT & RERUNNING IT, OR"
	echo "::: SIMPLY INSTALL THEM YOURSELF BY HAND."
	exit 1
fi

# If they're the same, then we only want to install the plaintext versions
if test "$mandir" = "$catdir"
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
if test -n "$mandir"
then
	for i
	do
		case $job in
		  install)	cp lib/$i.man $mandir/$i$ext ;;
		  remove)	rm $mandir/$i$ext* ;;
		esac
	done
fi

# Sleep briefly, to ensure that the plaintext versions of the files will
# be installed with a newer timestamp than the nroff versions.
sleep 1

# If we're going to be installing the plaintext pages, then do it now
if test -n "$catdir"
then
	for i
	do
		case $job in
		  install)
			if [ -x /usr/bin/nroff ]
			then
				/usr/bin/nroff -man lib/$i.man >$catdir/$i$ext
			elif [ -x ./elvis ]
			then
				ELVISPATH=dummypath; export ELVISPATH
				EXINIT="se lptype=bs"; export EXINIT
				./elvis -gquit -c"se bufdisplay=man|lp! $catdir/$i$ext" lib/$i.man
			else
				# give nroff another chance
				nroff -man lib/$i.man >$catdir/$i$ext
			fi
			;;
		  remove)
			rm $catdir/$i$ext*
			;;
		esac
	done
fi

# Are man pages sometimes compressed or packed on this system?  If so,
# then give these new pages the same treatment.
case $job in
  install)
	for dir in $mandir $catdir
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
				gzip -f $dir/$i$ext
			done
			finalext=$ext.gz
		else
			finalext=$ext
		fi
	done
	;;
esac

# Say what was done
case $job in
  install)
	if test -n "$mandir"
	then
		echo "nroffable manpages installed as $mandir/progname$finalext"
	fi
	if test -n "$catdir"
	then
		echo "plaintext manpages installed as $catdir/progname$finalext"
	fi
	;;
  remove)
	if test -n "$mandir"
	then
		echo "nroffable manpages removed from $mandir"
	fi
	if test -n "$catdir"
	then
		echo "plaintext manpages removed from $catdir"
	fi
	;;
esac
