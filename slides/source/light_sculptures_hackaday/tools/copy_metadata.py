#!/usr/bin/env python3
# (c) 2017 Erik Bosman
# License: https://opensource.org/licenses/MIT

import sys
import xml.etree.ElementTree as ET

src, dst = sys.argv[1:3]

namespaces = (
    ('dc', "http://purl.org/dc/elements/1.1/"),
    ('cc', "http://creativecommons.org/ns#"),
    ('rdf', "http://www.w3.org/1999/02/22-rdf-syntax-ns#"),
    ('svg', "http://www.w3.org/2000/svg"),
    ('xlink', "http://www.w3.org/1999/xlink"),
    ('sodipodi', "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"),
    ('inkscape', "http://www.inkscape.org/namespaces/inkscape"),
    ('', "http://www.w3.org/2000/svg"),
)

for k, v in namespaces:
	ET.register_namespace(k, v)

src_tree = ET.parse(src)
dst_tree = ET.parse(dst)

src_e = src_tree.find(".//{http://www.w3.org/1999/02/22-rdf-syntax-ns#}RDF")
dst_e = dst_tree.find(".//{http://www.w3.org/1999/02/22-rdf-syntax-ns#}RDF")
dst_parent = dst_tree.find(".//{http://www.w3.org/1999/02/22-rdf-syntax-ns#}RDF/..")
#dst_parent = dst_e.find("..") # why doesn't this work?

dst_parent.remove(dst_e)
dst_parent.insert(0, src_e)

dst_tree.write(dst, xml_declaration=True, encoding='utf-8')
