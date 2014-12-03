"For shell scripts, use a region to highlight herefiles.
if color("herefile") == ""
then color herefile like comment black or white
aug herefile
au OptChanged bufdisplay {
  if bufdisplay == "syntax sh" || knownsyntax(filename) == "sh"
  then try %s#<<-\?\s*\(\S\+\)#+,/^\s*\1$/-region herefile \1#x
}
au BufDelete all try %unr herefile
aug END
let bufdisplay=(bufdisplay)
