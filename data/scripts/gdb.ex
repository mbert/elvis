" Don't load this file yourself -- You should just run ":gdb programname".
"
" This file contains aliases commands that implement the GDB interface.
" The ":gdb" alias (which is defined in elvis.ali) loads this automatically,
" so you should never need to run ":load gdb" although it is harmless to do so.
" All of the aliases defined here are defined via ":alias!" (with a ! suffix)
" so they won't be saved by ":mkexrc" even though they are never unloaded,
" not even when the external gdb process terminates.

if gui != "x11"
then error The gdb interface only works with elvis' "x11" user interface

alias! tellgdb {
   " Send a command to GDB
   !elvgdb -t '!*'
}

alias! markgdb {
   "Set the 'g mark to a given file:line, and also move the cursor there
   local f l w
   let f="!1"[:1]
   let l="!1"[:2]
   if !isnumber(l) || !exists(f)
   then error Usage: markgdb filename:linenumber
   if filename == f
   then {
      (=f)=l mark g
      'g
   }
   else {
      let w = window(f)
      if w
      then {
	 eval window (w)
	 (=f)=l mark g
	 eval (w): 'g
      }
      else {
	 if !buffer(f)
	 then {
	    e (f)
	    "if bufchars <= 1
	    "then e (f)
	    "else split (f)
	 }
	 (=f)=l mark g
	 'g
      }
   }
}

alias highlightgdb {
   "Mark a given line using a region
   local f l
   let f="!1"[:1]
   let l="!1"[:2]
   if !isnumber(l) || !exists(f)
   then error Usage: highlightgdb filename:linenumber face [comment]
   try (=f)=l region !2*
   else {
      if bufchars <= 1
      then e +"=l region !2*" (f)
      else split +"=l region !2*" (f)
   }
}

alias! breakgdb {
   " Toggle a breakpoint on the current line
   if current("region") == "breakpoint"
   then eval tellgdb server delete (current("rcomment")[2])
   else eval tellgdb server break (filename):(current("line"))
}

alias! initgdb {
   " Set up the GUI for GDB

   " Colors.
   color! breakpoint on red

   " Buttons.  Note that the button that tells gdb to do a "next" command can't
   " be named [Next] because there's already a [Next] button.  So we use
   " MS-style [Step Over] for "next", and [Step In] for "step".
   gui gap
   gui Step In:tellgdb step
   gui Step In"Single-step.  For function calls, go into the function
   gui Step Over:tellgdb next
   gui Step Over"Single-step.  Treat function calls as one big step
   gui Until:eval tellgdb until (filename):(current("line"))
   gui Until"Execute to a point at (or below) the cursor
   gui Finish:tellgdb finish
   gui Finish"Execute to the end of the current function
   gui Continue:tellgdb cont
   gui Continue"Execute until the next breakpoint
   gui Break Here:breakgdb
   gui Break Here=current("region")=="breakpoint"
   gui Break Here"Set or clear a breakpoint at the cursor
   gui Break At...:eval tellgdb break (a)
   gui Break At...;"Where to break" (file)a; ="line or file:line or function"
   gui Break At..."Set a breakpoint at a specified address
   gui Up:tellgdb up
   gui Up"Move up the stack, to the function that called this one
   gui Down:tellgdb down
   gui Down"Move back down the stack
   gui Where:tellgdb where
   gui Where"List the whole stack
   gui Locals:tellgdb info locals
   gui Locals"List values of local variables
   gui Whatis:eval tellgdb whatis (current("word"))
   gui Whatis?current("word") != ""
   gui Whatis"Describe the symbol that the cursor is on
   gui Print:eval tellgdb print (current("word"))
   gui Print?current("word") != ""
   gui Print"Print the value of the variable that the cursor is on
   gui Run...:eval tellgdb run (g)
   gui Run...;"Command line arguments" (file)g
   gui Run...?state=="" || state<<6=="KILLED" || state<<6=="EXITED"
   gui Run..."Start execution
   gui End GDB:tellgdb quit
   gui End GDB"Stop debugging, and exit GDB

   " Shift-K should print value of variable
   let k=keywordprg
   set keywordprg="tellgdb print $1"

   " The info shown at bottom of screen should include "state"
   let s=show
   set show="state region tag"

   " Discourage users from running :mkexrc while in GDB mode
   alias mkexrc error You probably don't want to save settings while in GDB mode.
}

alias! termgdb {
   " Clean up after GDB terminates
   all %unregion breakpoint
   gui ~Step In
   gui ~Step Over
   gui ~Until
   gui ~Finish
   gui ~Continue
   gui ~Break Here
   gui ~Break At...
   gui ~Up
   gui ~Down
   gui ~Where
   gui ~Locals
   gui ~Whatis
   gui ~Print
   gui ~Run...
   gui ~End GDB
   let keywordprg=k
   let show=s
   set secret=0 state=""
   unalias mkexrc
}

alias! fromgdb {
   " Receive information from gdb
   switch "!1"
   case elvgdb-init initgdb
   case elvgdb-term termgdb
   case source markgdb !2
   case breakpoints-invalid {
      all %unregion breakpoint
      tellgdb server info breakpoints
   }
   case breakpoints-table {
      if "!3" == "breakpoint" && "!4" == "y"
      then highlightgdb !8 breakpoint Breakpoint !2
   }
   case starting set state=RUNNING
   case breakpoint set state="BREAKPOINT=!2"
   case watchpoint set state="WATCHPOINT=!2"
   case signal set state="CAUGHT=!2"
   case signalled set state="KILLED=!2"
   case exited set state="EXITED=!2"
   case value-begin echo !3*
   default echo fromgdb !*
}

alias! print {
   "Use gdb to print the value of an expression.  (Doesn't affect :p)
    tellgdb output !*
}
