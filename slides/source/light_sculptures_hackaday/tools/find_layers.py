#!/usr/bin/env python3
# (c) 2017 Erik Bosman
# License: https://opensource.org/licenses/MIT

import sys
import xml.etree.ElementTree as ET

root = ET.fromstring(sys.stdin.read())

for e in root.findall(".//*[@{http://www.inkscape.org/namespaces/inkscape}groupmode='layer']"):
	print (e.attrib['{http://www.inkscape.org/namespaces/inkscape}label']+','+e.attrib['id'])
