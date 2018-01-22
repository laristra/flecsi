#------------------------------------------------------------------------------#

all:
	$(eval test := $(shell pwd | sed 's,^.*\/,,g;s,[0-9][0-9]-,,g'))
	@echo "/*------------------------------------------------------------* "
	@echo " * FleCSI Tutorial"
	@echo " *------------------------------------------------------------*/"
	@echo ""
	@echo "To build the tutorial example, execute:"
	@echo "   $$ flecsit compile $(test).cc"
	@echo ""
	@echo "You make need to load the flecsi-tutorial environment module:"
	@echo "   $$ module load flecsi-tutorial"
	@echo ""
	@echo "OR source the bash or csh configuration file:"
	@echo "   $$ source INSTALL_PATH_TO_FLECSI/bin/flecsi-tutorial.{sh,csh}"
	@echo ""
	@echo "For convenience, this Makefile can be used to generate pdf"
	@echo "files from the tutorial output by running:"
	@echo "   $$ make pdfs"
	@echo ""
	@echo "This Makefile can also be used to clean-up after you have"
	@echo "built an example by running:"
	@echo "   $$ make clean"
	@echo ""

texfiles = $(wildcard *.tex)
pdffiles = $(patsubst %.tex, %.pdf, $(texfiles))

%.pdf: %.tex
	pdflatex $<

pdfs: $(pdffiles)

clean:
	$(eval test := $(shell pwd | sed 's,^.*\/,,g;s,[0-9][0-9]-,,g'))
	rm -rf *.tex *.aux *.log *.pdf $(test)
