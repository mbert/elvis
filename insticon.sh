# This script is used for installing X11 icons into appropriate directories.
# The list of icons, and their directories, appears at the end of this file.
#
# usage: sh insticon.sh [-r], where "-r" implies that they should be removed


# A couple of configurable directories.  The X11-related files are assumed to
# be in a subdirectory named "X11" under this.
xinc=/usr/include
xlib=/usr/X11/lib

# Some directory names, derived from the above.  These are used to abbreviate
# the names that appear in the file list, below
bitmaps=$xinc/X11/bitmaps
pixmaps=$xinc/X11/pixmaps
three_d=$xinc/X11/3dpixmaps
fvwm2=$xlib/X11/fvwm2
fvwm95=$xlib/X11/fvwm95-2
tkdesk=$xlib/TkDesk/images

while read icon filelist
do
	for file in $filelist
	do
		dir=`dirname $file`
		if test -d $dir
		then
			if [ X$1 = X-r ]
			then
				rm $file && echo $file removed
			else
				cp guix11/$icon $file
				chmod 0644 $file
				echo $file installed
			fi
		fi
	done
done <<EOF
    elvis.xbm	$bitmaps/elvis.xbm
    elvis.xpm	$pixmaps/elvis.xpm
    normal.xpm	$three_d/normal/elvis_3d.xpm $three_d/next/elvis_3d.xpm
    small.xpm	$three_d/small/small.elvis_3d.xpm
    mini.xpm	$three_d/mini/mini.elvis_3d.xpm $fvwm95/mini-icons/mini-elvis.xpm $tkdesk/ficons16/elvis.xpm
EOF
exit 0
