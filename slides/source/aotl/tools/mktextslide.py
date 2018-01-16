#!/usr/bin/env python3
# (c) 2017 Erik Bosman
# License: https://opensource.org/licenses/MIT

import sys
from xml.sax.saxutils import escape

f = open(sys.argv[1], 'r')
template = f.read()
f.close()

header, rest = template.split('TITLE_MARKER', 1)
mid, footer = rest.split('TEXT_MARKER',1)

print(header,end='')

title = sys.stdin.readline()
print(escape(title))
print(mid,end='')
for i,line in enumerate(sys.stdin):
    y = str(351+108*(i-1))+'px'
    print ('<tspan y="'+y+'" x="167px">'+escape(line)+'</tspan>')

print(footer,end='')
