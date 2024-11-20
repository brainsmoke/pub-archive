#!/usr/bin/env python3
# (c) 2017 Erik Bosman
# License: https://opensource.org/licenses/MIT

import sys, os

import hashlib

TMPDIR="tmp/"

try:
	os.mkdir(TMPDIR)
except FileExistsError:
	pass


deck, out = sys.argv[1:3]

f = open(deck, 'r')

name_last = None
layers = None
pgno = 1
show_pgno = False

default_layers = []
slides = []
rules = []

def read_layerexp(s):
	assert(':' not in s)
	layers = [ l.strip() for l in s.split(',') if l.strip() != '' ]
	for layer in layers:
		for c in layer:
			assert( 'a' <= c <= 'z' or 'A' <= c <= 'Z' or '0' <= c <= '9' or c in '_-')
	return layers

for line in f:
	line = line.rstrip('\n')
	if ';' in line:
		line = line[:line.find(';')]
	line = line.strip()
	if line == '':
		continue

	if line[0] == '#':
		command = line[1:]
		if command == 'off':
			show_pgno = False
			command = command[1:]
		else:
			show_pgno = True

		if command[0] == '+':
			pgno += 1

		if command != '':
			pgno = int(command)

		continue

	if line[0] == '@':
		directive = line[1:]
		command,layerexp = directive.split(':', 1)
		if command.strip() == 'always':
			default_layers = read_layerexp(layerexp)
		continue

	slide,layerexp = line.split(':', 1)
	slide = slide.strip()
	layers = read_layerexp(layerexp)
	if len(layers) > 0:
		layers = default_layers + layers

	page_str = '\'\''
	if show_pgno:
		page_str = str(pgno)

	infile = 'tmp/'+slide+'.svg'

	layers.sort()
	uniq = ' '.join([infile, page_str] + layers)
	filename = TMPDIR+hashlib.sha256(uniq.encode('utf-8')).hexdigest()+'.svg'

	deps = filename+': '+infile
	command = './tools/extract_layers.py '+page_str+' '+' '.join(layers)+' < $< > $@'

	slides.append(filename)
	rules.append(deps+'\n\t'+command+'\n')

f.close()

outf = open(out, 'w')
outf.write('SLIDES='+' '.join(slides)+'\n\n\n')
outf.write('PDFS=$(patsubst %.svg, %.pdf, $(SLIDES))\n')
outf.write('.SECONDARY: $(PDFS)\n')
outf.write('\n'.join(rules)+'\n')

