$ ! Command file to build ELVIS on a VMS machine.
$ ! To use the debugger, change the line below to read debug = 1
$ debug = 0
$ define sys sys$library   ! To resolve #include <sys/types.h> etc.
$ if debug .ne. 1 then goto start
$ debstr := /debug
$ optstr := /noopt
$ start:
$ defines := /define=NO_ERRLIST
$ set nocontrol=y
$ On Error Then Goto The_Exit
$      cc  'defines''optstr''debstr' blk.c
$      cc  'defines''optstr''debstr' cmd1.c
$      cc  'defines''optstr''debstr' cmd2.c
$      cc  'defines''optstr''debstr' ctype.c
$      cc  'defines''optstr''debstr' curses.c
$      cc  'defines''optstr''debstr' cut.c
$      cc  'defines''optstr''debstr' ex.c
$      cc  'defines''optstr''debstr' input.c
$      cc  'defines''optstr''debstr' main.c
$      cc  'defines''optstr''debstr' misc.c
$      cc  'defines''optstr''debstr' modify.c
$      cc  'defines''optstr''debstr' move1.c
$      cc  'defines''optstr''debstr' move2.c
$      cc  'defines''optstr''debstr' move3.c
$      cc  'defines''optstr''debstr' move4.c
$      cc  'defines''optstr''debstr' move5.c
$      cc  'defines''optstr''debstr' opts.c
$      cc  'defines''optstr''debstr' recycle.c
$      cc  'defines''optstr''debstr' redraw.c
$      cc  'defines''optstr''debstr' regexp.c
$      cc  'defines''optstr''debstr' regsub.c
$      cc  'defines''optstr''debstr' system.c
$      cc  'defines''optstr''debstr' tio.c
$      cc  'defines''optstr''debstr' tmp.c
$      cc  'defines''optstr''debstr' vars.c
$      cc  'defines''optstr''debstr' vcmd.c
$      cc  'defines''optstr''debstr' vi.c
$      cc  'defines''optstr''debstr' tinytcap.c
$      cc  'defines''optstr''debstr' vmsio.c
$ link/exe=elvis.exe main,vmslink.opt/opt/nomap  'debstr'
$      cc  'debstr' ctags.c
$ link ctags,sys$input/opt 'debstr'
sys$share:vaxcrtl.exe/share
$      cc  'debstr' fmt.c
$ link fmt,sys$input/opt 'debstr'
sys$share:vaxcrtl.exe/share
$      cc  'debstr' ref.c
$ link ref,sys$input/opt 'debstr'
sys$share:vaxcrtl.exe/share
$ The_Exit:
