
.PHONY: clean

TARGETS=promotion.pdf

all: promotion.pdf

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

clean:
	-rm tmp/*

tmp/%.dep: %.deck
	./tools/mkdeps.py $< $@

