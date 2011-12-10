" This file contains a simple :note alias and some commands that use it.

alias note {
  "Add/change/remove a note for a given range of lines
  if "!*" = ""
  then !%unr note
  else !%reg note !*
}
alias nextnote {
  "Search forward for the next line with a note
  local l
  let l = current("line")
  try +
  else error no more notes
  while current("line") < buflines && current("region") != "note"
  do +
  if current("region") != "note"
  then {
	=l
	error no notes below
  }
}
alias prevnote {
  "Search backward for the previous line with a note
  local l
  let l = current("line")
  try -
  else error no more notes
  while current("line") > 1 && current("region") != "note"
  do -
  if current("region") != "note"
  then {
    =l
    error no notes above
  }
}

"Some maps to make searching easier
map gn :nextnote<Enter>
map gN :prevnote<Enter>

"Notes are only useful if persistfile stores regions
if persistfile=""
then warning You should set persistfile to make notes be permanent
if !persist.region
then {
  warning You should run :mkexrc to add "regions" to the persist option.
  let persist |= "regions"
}

"I deliberately don't define any attribute for the "note" face.  You can if
"you want, outside of this file, but my personal opinion is that coloring
"lines with notes would make them too distracting.
