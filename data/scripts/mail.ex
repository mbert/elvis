"This defines a :Mail alias.  This is intended mostly to be used when elvis
"is invoked as an external editor by a mail program such as Kmail or Sylpheed.
"The idea is that you'd configure the mail editor to run "elvis -cMail" as the
"external editor.  To make that work, you'd need to run ":load mail" and
" ":mkexrc" first though.
alias Mail {
	set bufdisplay="syntax email" equalprg="elvfmt -M"
	display syntax email
	if color("signature") == ""
	then color signature italic red on gray
	try $;?^-- *$?,$ region signature
}
