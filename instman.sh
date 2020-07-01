#!/bin/sh
#
# This shell script is used on UNIX-like systems to install or remove
# man-pages.  It is run automatically during "make install".
#
#   usage: sh install.sh [-r|-d] [-pprefix] programs...
#
################################################################################

# This is the default prefix.  All installation directories are relative
# to this.  Usually the PREFIX shown here is overridden by a -pprefix flag.
PREFIX=/usr/local

# This is a list of directories where the nroff source for man-pages might
# be installed.  The order is important -- the man-pages will be installed
# in the first existing directory in the list.
MANDIRS="MAN/man.l MAN/man.LOCAL MAN/man.1 MAN/1 MAN/catman.C MAN/manl MAN/man1 MAN/elvis/man"

# Similarly, this is a list of directories where the plaintext form of the
# man-pages might belong.  If, after searching, it is decided that both forms
# should go in the same directory, then only the plain-text version is
# installed.
CATDIRS="MAN/catmanl MAN/catman.l MAN/catman.LOCAL MAN/cat.LOCAL MAN/catman.1 MAN/cat.C MAN/catman.C MAN/catman/1 MAN/1 MAN/cat1"

################################################################################

# Look for a "-r"
if test "$1" = "-d"
then
	shift
	job="install/remove"
elif test "$1" = "-r"
then
	shift
	job="remove"
else
	job="install"
fi

# Look for a "-pprefix" argument
case "$1" in
  -p)
	shift
	PREFIX="$1"
	shift
	;;
  -p*)
	PREFIX=`echo "$1"|sed 's/-p//'`
	shift
	;;
esac

# If MANPATH is unset, then set it to some likely values
if [ "$MANPATH" = "" ]
then
	MANPATH="/usr/local/share/man:/usr/share/man:$PREFIX/share/man"
	MANPATH="$MANPATH:/usr/local/man:/usr/X11/man:/usr/man:$PREFIX/man"
fi

# Choose a man directory under PREFIX, giving priority to "share" directories.
oldIFS="$IFS"
IFS=":"
manprefix="none"
for i in $MANPATH
do
	case $i in
	  "$PREFIX"/share/*) [ -d "$i" -a $manprefix = none ] && manprefix=$i;;
	esac
done
for i in $MANPATH
do
	case $i in
	  "$PREFIX"/*) [ -d "$i" -a $manprefix = none ] && manprefix=$i;;
	esac
done
IFS="$oldIFS"
if  [ "$manprefix" = "none" ]
then
	JOB=`echo $job|tr '[a-z]' '[A-Z]'`
	echo >&2 "::: I CAN'T AUTOMATICALLY $JOB THE MAN PAGES ON THIS SYSTEM"
	echo >&2 "::: BECAUSE I DON'T KNOW WHERE TO PUT THEM!  No directory in"
	echo >&2 "::: your \$MANPATH looks like a man page directory, inside"
	echo >&2 "::: your chosen installation directory of '$PREFIX'"
	echo >&2 "::: You should $job the man pages manually."

	# But "make install" should continue anyway
	exit 0
fi


# Adjust MANDIRS and CATDIRS to be relative to the manprefix
MANDIRS=`echo "$MANDIRS" | sed "s#MAN#$manprefix#g"`
CATDIRS=`echo "$CATDIRS" | sed "s#MAN#$manprefix#g"`

# Complain if no programs were named on the command line
if test $# -lt 1 -a $job \!= "install/remove"
then
	echo >&2 "Usage: sh instman.sh [-r] [-p prefix] {program names...}"
	echo >&2 "     where {program names...} is a list of programs whose"
	echo >&2 "     manual pages are to be installed.  It is assumed that"
	echo >&2 "     the manual pages are currently located inside the 'doc'"
	echo >&2 "     subdirectory."
	echo >&2
	echo >&2 "        -d         Announce the directory (else install)"
	echo >&2 "        -r         Remove man-pages (else install them)"
	echo >&2 "        -p prefix  All possible man page directories are"
	echo >&2 "                   relative to this prefix."
	echo >&2
	echo >&2 "     Note: Normally, this script is executed via 'make install'"
	echo >&2 "           or 'make uninstall'"
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
		elif test -d $dir # -a -w $dir
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
		elif test -d $dir # -a -w $dir
		then
			echo catdir=$dir
			exit 0
		fi
	done`

# Both names should be either in the list derived from "-p prefix" or on the
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
	JOB=`echo $job|tr '[a-z]' '[A-Z]'`
	echo >&2 "::: I CAN'T AUTOMATICALLY $JOB THE MAN PAGES ON THIS SYSTEM"
	echo >&2 "::: BECAUSE I DON'T KNOW WHERE TO PUT THEM!  You can either"
	echo >&2 "::: try editing the \"instman.sh\" script & rerunning it, or"
	echo >&2 "::: simply $job them yourself by hand."

	# exit... but 'make install' should continue anyway
	exit 0
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
		  install)	cp doc/$i.man $mandir/$i$ext
		          	chmod 0644 $mandir/$i$ext
		          	;;
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
				/usr/bin/nroff -man doc/$i.man >$catdir/$i$ext
			elif [ -x ./elvis ]
			then
				ELVISPATH=dummypath; export ELVISPATH
				./elvis -Gquit -c"se bd=man lpt=bs lplines=0|lp! $catdir/$i$ext" doc/$i.man
			else
				# give nroff another chance
				nroff -man doc/$i.man >$catdir/$i$ext
			fi
			chmod 0644 $catdir/$i$ext
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
				rm -f $dir/$i$ext.gz
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
  install/remove)
	echo $mandir
	;;
esac
exit 0
