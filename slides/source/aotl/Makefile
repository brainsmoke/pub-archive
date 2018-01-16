
.PHONY: clean

TARGETS=aotl-34c3-slides.pdf

all: aotl-34c3-slides.pdf

include $(patsubst %.pdf, tmp/%.dep, $(TARGETS))

tmp/%.pdf: tmp/%.svg
	inkscape -A $@ $<

tmp/%.svg: %.text templates/text_slide.svg tools/mktextslide.py
	./tools/mktextslide.py templates/text_slide.svg < $< > $@

tmp/%.svg: %.title templates/title_slide.svg tools/mktitleslide.py
	./tools/mktitleslide.py templates/title_slide.svg < $< > $@

tmp/%.svg: %.svg
	cp $< $@

%.pdf: %.deck $(PDFS)
	 pdftk $(PDFS) output "$@"

clean:
	-rm tmp/*

tmp/%.dep: %.deck
	./tools/mkdeps.py $< $@

