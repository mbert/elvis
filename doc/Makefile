NROFF=	groff
HPROFF=	troff
HPDEST= >/dev/hp
PSROFF=	groff -Tps
PSDEST= >/dev/lp
MS=	title.ms index.ms intro.ms visual.ms ex.ms regexp.ms options.ms \
	init.ms cutbufs.ms differ.ms internal.ms makefile.ms cflags.ms \
	termcap.ms environ.ms versions.ms question.ms
MAN=	ctags.man elvis.man elvprsv.man elvrec.man fmt.man ref.man
DOC=	title.doc index.doc intro.doc visual.doc ex.doc regexp.doc options.doc \
	init.doc cutbufs.doc differ.doc internal.doc makefile.doc cflags.doc \
	termcap.doc environ.doc versions.doc question.doc ctags.doc elvis.doc \
	elvprsv.doc elvrec.doc fmt.doc ref.doc
PS=	title.ps index.ps intro.ps visual.ps ex.ps regexp.ps options.ps \
	init.ps cutbufs.ps differ.ps internal.ps makefile.ps cflags.ps \
	termcap.ps environ.ps versions.ps question.ps ctags.ps elvis.ps \
	elvprsv.ps elvrec.ps fmt.ps ref.ps
HP=	title.ps index.ps intro.hp visual.hp ex.hp regexp.hp options.hp \
	init.ps cutbufs.hp differ.hp internal.hp makefile.hp cflags.hp \
	termcap.hp environ.hp versions.hp question.hp ctags.hp elvis.hp \
	elvprsv.hp elvrec.hp fmt.hp ref.hp

.SUFFIXES: .test .doc .ps .hp .ms .man
.ms.doc:
	$(NROFF) -ms ver.ms $*.ms | ./elvcol >$*.doc
.man.doc:
	$(NROFF) -man $*.man | ./elvcol >$*.doc
.ms.ps:
	$(PSROFF) -ms ver.ms $*.ms $(PSDEST)
.man.ps:
	$(PSROFF) -man $*.man $(PSDEST)
.ms.hp:
	$(HPROFF) -ms ver.ms $*.ms $(HPDEST)
.man.hp:
	$(HPROFF) -man $*.man $(HPDEST)
.ms.test:
	$(PSROFF) -ms ver.ms $*.ms >/dev/null
.man.test:
	$(PSROFF) -man $*.man >/dev/null

../Elvisman.txt: $(DOC)
	cat $(DOC) >../Elvisman.txt

Elvisman.ps: $(PS)

Elvisman.hp: $(HP)

clean:
	rm -f $(DOC) Elvisman.txt
