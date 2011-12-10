.\" ==========================================================================
.\" This is the version number, as printed on the front page of the manual.
.\"
.ds V 1.8pl3
.\"
.\" ==========================================================================
.\" These macros change the size of text.  Coherent's troff may have trouble
.\" with the macros as defined; if so, try putting single-quotes around the
.\" sign and number, so "\s-2" becomes "\s'-2'", and "\s+2" becomes "\s'+2'".
.\"
.ds s \s-2
.ds S \s+2
.\"
.\" ==========================================================================
.\" This makes "\*E" be shorthand for "Elvis", maybe with fancy font changes.
.\"
.if t .ds E E\*sLVIS\*S
.if n .ds E Elvis
.\"
.\" ==========================================================================
.\" This is used to start a new section of the manual.  Most of the *.ms files
.\" begin with one of these things.
.\"
.\" usage: .Go <section#> <title>
.\"
.de Go
.ds LH "\\$1-\\\\n%
.ds RH "\\$1-\\\\n%
.ds CH "\\$2
.NH S \\$1
\\$2
.\"if !\\n%=1 .bp 1
.if n .ul 0
..
