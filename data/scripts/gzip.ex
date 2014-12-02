" This file contains a set of autocmds which allow elvis to read & write
" files that are compressed via gzip.
augroup gzip
  au!
  au BufReadPre,FileReadPre	*.gz set reol=binary
  au BufReadPost		*.gz %!gunzip
  au FileReadPost		*.gz '[,']!gunzip
  au BufReadPost,FileReadPost	*.gz set reol=text nomodified bufdisplay=normal
  au BufReadPost		*.gz {
					local s
					let s = knownsyntax(basename(filename))
					if s
					then eval set bufdisplay="syntax (s)"
				     }
  au BufWritePost,FileWritePost	*.gz eval !mv (afile) (basename(afile))
  au BufWritePost,FileWritePost	*.gz eval !gzip (basename(afile))
  au FileAppendPre		*.gz eval !gunzip (afile)
  au FileAppendPre		*.gz eval !mv (basename(afile)) (afile)
  au FileAppendPost		*.gz eval !mv (afile) (basename(afile))
  au FileAppendPost		*.gz eval !gzip (basename(afile))
augroup END
