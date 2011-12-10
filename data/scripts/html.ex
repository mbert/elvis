"This file contains some maps that are useful when editing XML or HTML code.

alias htmlmap {
  " Load maps for "html" display mode

  " Remove the special handing for >
  unmap input mode=html >

  " Protect some characters when inserting in html display mode
  map input mode=html noremap & &amp;
  map input mode=html noremap < &lt;
  map input mode=html noremap > &gt;

  " No tag highlighting -- let tags do their own highlighting
  set hlobject=""
}
alias htmlunmap {
  " Load maps for "syntax html" display mode

  " Remove maps that protect some special HTML characters
  unmap input mode=html &
  unmap input mode=html <
  unmap input mode=html >

  " When a > is input at the end of a tag, automatically add the closing tag
  map input mode=html > noremap ><Esc>F<yef>pa><Esc>bi/<Esc>hi

  " Highlight the innermost tag */
  set hlobject="ix,ax"
}
aug edithtml
au DispMapEnter html htmlmap
au DispMapLeave html htmlunmap
aug END
if display == "html"
then htmlmap
else htmlunmap

" Add ax to matchchar -- lets you use % to find matching tag pairs
set matchchar="{}()[]ax"

" Arrange for innermost tag to be highlighted later, when hlobject is set */
if hllayers == 0
then {
  set hllayers=2
  if color("hlobject1") == ""
  then color hlobject1 boxed
  if color("hlobject2") == ""
  then color hlobject2 bold
}

" For X11, set up some buttons
if gui == "x11"
then {
  gui gap
  gui Heading: htmlblock pair h1
  gui Heading?current("selection")=="line"
  gui Subheading: htmlblock pair h2
  gui Subheading?current("selection")=="line"
  gui Left: htmlblock div left
  gui Left?current("selection")=="line"
  gui Right: htmlblock div right
  gui Right?current("selection")=="line"
  gui Center: htmlblock div center
  gui Center?current("selection")=="line"
  gui Bullet: htmlblock list ul
  gui Bullet?current("selection")=="line"
  gui Number: htmlblock list ol
  gui Number?current("selection")=="line"
  gui Preformat: htmlblock pair pre
  gui Preformat?current("selection")=="line"
  gui Other: eval htmlblock pair (t)
  gui Other?current("selection")=="line"
  gui Other;"HTML tag to add" t
  gui gap
  gui [Bold]
  gui Bold?current("selection")=="line" || current("selection")=="character"
  gui [Italic]
  gui Italic?current("selection")=="line" || current("selection")=="character"
  gui [Code]
  gui Code?current("selection")=="line" || current("selection")=="character"
  gui gap
  gui Undo: u
  alias htmlblock {
    switch "!1"
    case pair {
      !> a </!2>
      !< i <!2>
    }
    case div {
      !> a </div>
      !< i <div align=!2>
      if "!2" != "left"
      then warning Elvis doesn't display text aligned to the !2
    }
    case list {
      !% s/^/<li>/
      !> a </!2>
      !< i <!2>
    }
  }
  map select mode=html noremap [Bold] c<strong<C-V>><C-P></strong<C-V>><Esc>
  map select mode=html noremap [Italic] c<em<C-V>><C-P></em<C-V>><Esc>
  map select mode=html noremap [Code] c<code<C-V>><C-P></code<C-V>><Esc>
}
