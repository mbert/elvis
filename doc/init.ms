.Go 6 "INITIALIZATION"
.PP
Many features of \*E are configurable at runtime.
There are commands for assigning actions to keys (:map),
defining abbreviations (:abbr),
non-ASCII keying sequences (:digraph),
setting screen colors (:color),
and miscellaneous other options (:set).
.PP
All of these commands can be issued interactively.
Experienced vi users generally prefer to have some options set
every time they run they execute \*E,
and \*E has ways to support this.
.LP
.SH
Start-up Initialization
.PP
When \*E starts, it executes the following algorithm in an attempt
to locate initialization commands:
.ID

If this version of \*E supports a system-wide initialization
file and that file exists,
    Interpret that file's contents as a series of "ex" commands

If the EXINIT environment variable is set
    Interpret the value of EXINIT as an "ex" command line.
Else if the home directory contains a file named ".exrc"
    Interpret that file's contents as a series of "ex" commands

If the "exrc" option is set, and the current directory contains
a file named ".exrc"
    Interpret that file's contents as a series of "ex" commands

If a tag was specified via a "-t" command line argument,
    Execute a tag look-up, and load file if successful

If no tag was specified, or the specified tag wasn't found,
    Load the first file named on the command line, or
    start empty buffer.

If a command was specified via "+command" or "-c command"
    Execute the given command.
.DE
.PP
Note that most of this initialization occurs before the first file is loaded.
Consequently, commands which examine or change the edit buffer can't be used
there.
Only "+command" or "-c command" is executed after the text file has been loaded.
.PP
On non-UNIX systems, ".exrc" is usually an invalid filename so the file is
called "ELVIS.RC" instead.
Also, the home directory is the directory named by the HOME environment
variable;
on DOS and a few other systems, if HOME is unset then \*E will use the
directory which contains the executable file (ELVIS.EXE) as your home
directory.
.LP
.SH
File Initialization
.PP
Loading a file, too, can cause commands to be executed.
Each time any file is loaded into the edit buffer, the following
algorithm is used to locate file-specific initialization commands.
.ID

Fill the edit buffer with the file's contents, and set various options
and variables accordingly.

If the home directory contains a file involves  named ".exfilerc"
    Interpret the contents of that file as a series of "ex" commands.

If the "modelines" option is set,
    Search the first 5 & last 5 lines of the text for lines which
    contain "ex:<command>:" or "vi:<command>:", and interpret any
    <command> as an "ex" command line.
.DE
.PP
On non-UNIX systems, ".exfilerc" is usually an invalid filename, so the file
is called "EXFILE.RC" instead.
.LP
.SH
The :mkexrc Command
.PP
\*E has a special command, ":mkexrc [filename]", to help you create ".exrc"
files.
It creates a file which sets all nonstandard options, maps, and so on.
.PP
By default, the created file's name will be ".exrc" in the current directory.
You can either add ":set exrc" to the .exrc file in your home directory
to force \*E to read this new .exrc in your current directory,
or you can move this new .exrc file into your home directory.
.PP
Alternatively, you can supply an explicit filename as an argument to :mkexrc.
.PP
Afterward, you may wish to edit the created file.
For example, some options may be conditional;
the :mkexrc file doesn't distinguish between options which were set 
unconditionally from those that were set in a text file's modelines
or other conditional context.
.PP
Warning: the :mkexrc command will happily overwrite any file that you tell
it to, if your operating system permits.
.LP
.SH
Other Techniques
.PP
\*E has commands for conditional execution, but the standard vi doesn't.
If you often use the real vi, you may want to avoid \*E's extensions.
To have a terminal-dependent initialization file, you can add
":so $HOME/.exrc.$TERM" to the end of your .exrc file, and then create
files with names like ".exrc.vt100" and ".exrc.ansi" in your home directory
which contain the terminal-dependent commands.
.PP
Another good technique is to write a shell-script "wrapper" around \*E/vi.
Here's one of my favorites.
It uses "grep" to locate files containing a given regular expression,
and then starts vi on those files
with the cursor positioned on the first occurrence in the first file.
I call this script "vg".
.ID

#!/bin/sh
case "$#" in
  0)	echo "usage: vg regexp [files]..." >&2; exit;;
  1)	set -- "$1" *.[ch];;
esac

regexp="$1"
shift

vi +/"$regexp" `grep -l "$regexp" "$@"`
.DE
