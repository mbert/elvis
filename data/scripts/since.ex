"Defines some aliases and autocmds for highlighting lines that have changed.

alias since {
  " Highlight the differences between current buffer and its file
  local a s=since f t=/tmp/diff report=0
  let f = "!*" || filename
  if !exists(filename)
  then error usage: since [filename]
  eval unr (s)
  try w! (t)
  then {
    let a = "diff" f t
    let a = a;"| sed -n 's/^[0-9,]*a\\([0-9,]*\\)$/\\1reg" s "added/p;s/^[0-9,]*c\\([0-9,]*\\)$/\\1reg" s "changed/p'"
    eval safely source !!(a)
    eval !!rm (t)
  }
}

alias rcssince {
  " Highlight the differences between the current buffer and an RCS version
  local s=rcssince t=/tmp/diff report=0
  eval unr (s)
  if exists("RCS"/filename;",v") || exists(filename;",v")
  then {
    try w! (t)
    then {
      let a = "co -p !1 2>/dev/null" filename "| diff - " t "| sed -n '"
      let a = a;"s/^[0-9,]*a\\([0-9,]*\\)$/\\1reg" s "added/p;"
      let a = a;"s/^[0-9,]*c\\([0-9,]*\\)$/\\1reg" s "changed/p'"
      eval safely source !!(a)
      eval !!rm (t)
    }
  }
}

aug since
  au!
  au Edit * {
    if filename
    then '[,']reg unsaved
  }
  au BufWritePost * %unr unsaved
  au BufReadPost * rcssince
aug END

switch gui
  case windows
  case x11 {
    if color("since") == ""
    then color since on orange
    if color("rcssince") == ""
    then color rcssince on orange
    if color("unsaved") == ""
    then color unsaved on tan
  }
  case vio
  case termcap {
    if color("since") == ""
    then color since on brown
    if color("rcssince") == ""
    then color rcssince on brown
    if color("unsaved") == ""
    then color unsaved on red
  }
