$! VMSSETUP.COM         Last modified   2-AUG-1991/JDC
$! Define foreign commands needed for Elvis running on a VMS machine
$!
$!
$ front = f$logical( "SYS$DISK" ) + f$directory()
$!
$ vi == "$" + f$parse( front, "elvis.exe" ) - ";"
$ view == "$" + f$parse( front, "elvis.exe" ) - ";" + " ""-R"""
$ ex == "$" + f$parse( front, "elvis.exe" ) - ";" + " ""-e"""
$ input == "$" + f$parse( front, "elvis.exe" ) - ";" + " ""-i"""
$ ctags == "$" + f$parse( front, "ctags.exe" ) - ";"
$ ref  == "$" + f$parse( front, "ref.exe" ) - ";"
$ fmt  == "$" + f$parse( front, "fmt.exe" ) - ";"
