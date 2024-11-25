
.PHONY: clean

#TARGETS=$(patsubst %.svg, %.pdf, *.deck)
TARGETS=slides.pdf

all: $(TARGETS)

include $(patsubst %.pdf, tmp/%.dep, $(TARGETS))

tmp/%.pdf: tmp/%.svg
	inkscape  --export-type=pdf --export-ignore-filters -o $@ $<

tmp/%.svg: %.text templates/text_slide.svg tools/mktextslide.py
	./tools/mktextslide.py templates/text_slide.svg < $< > $@

tmp/%.svg: %.title templates/title_slide.svg tools/mktitleslide.py
	./tools/mktitleslide.py templates/title_slide.svg < $< > $@

tmp/%.svg: %.svg
	cp $< $@

%.pdf: %.deck $(PDFS)
	 pdftk $(PDFS) output "$@"

# would be nice, but it degrades images by re-encoding them
#
#%.pdf: tmp/%.pdf
#	gs -o $@ -sDEVICE=pdfwrite -dDetectDuplicateImages=true -dCompressFonts=true -dAutoRotatePages=/None -f $^
#
#tmp/%.pdf: %.deck $(PDFS)
#	 pdftk $(PDFS) output "$@"
#

clean:
	-rm tmp/*

tmp/%.dep: %.deck
	./tools/mkdeps.py $< $@

