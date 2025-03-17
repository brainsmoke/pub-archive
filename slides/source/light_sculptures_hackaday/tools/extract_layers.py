#!/usr/bin/env python3
# (c) 2017 Erik Bosman
# License: https://opensource.org/licenses/MIT

import sys
import xml.etree.ElementTree as ET

pageno = sys.argv[1]
layers = sys.argv[2:]

tree = ET.parse(sys.stdin)

for e in tree.findall(".//*[@{http://www.inkscape.org/namespaces/inkscape}groupmode='layer']"):
    style = 'display:none'
    name = e.get('{http://www.inkscape.org/namespaces/inkscape}label')
    if name in layers or layers == []:
        style = 'display:inline'
    e.set('style', style)

    if name == 'pagenumber':
        for num in e.findall(".//{http://www.w3.org/2000/svg}tspan"):
            num.text = pageno

tree.write(sys.stdout.buffer)
