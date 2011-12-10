alias shuffle {
  "Randomly unsort all lines in a buffer, or just a range of lines.
  local a=!(1)< z
  let z=!(buflines)>
  while a < z-1
  do {
    eval (a) m (a + rand(z - a))
    let a += 1
  }
}
