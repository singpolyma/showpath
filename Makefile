include build-config.mk

.PHONY: all clean doc

all: showpath
doc: showpath.pdf

showpath.ps: showpath.1
	groff -Tps -mdoc $< > $@
showpath.pdf: showpath.ps
	ps2pdf $< $@

clean:
	$(RM) *.o showpath showpath.ps showpath.pdf
