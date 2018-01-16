#!/usr/bin/env python3
# (c) 2017 Erik Bosman
# License: https://opensource.org/licenses/MIT

import sys
from xml.sax.saxutils import escape

f = open(sys.argv[1], 'r')
template = f.read()
f.close()

header, footer = template.split('TITLE_MARKER', 1)

print(header,end='')

title = sys.stdin.readline()
print(escape(title))
print(footer,end='')
