
import sys, os

orig = [ 'fill:#929292;', 'fill:#c7c7c7;', 'fill:#dfdfdf;' ]

name = sys.argv[1]

colors = {
#                  shade              top              front
    'red'   : [ 'fill:#ac2c2c;', 'fill:#ff8080;', 'fill:#ff3939;' ],
    'blue'  : [ 'fill:#093972;', 'fill:#81aae5;', 'fill:#638ef7;' ],
    'green' : [ 'fill:#2cac37;', 'fill:#aff6b5;', 'fill:#67ed72;' ],
    'lblue' : [ 'fill:#2c74ac;', 'fill:#afd6f6;', 'fill:#67b2ed;' ],
    'gold'  : [ 'fill:#90531c;', 'fill:#a46410;', 'fill:#edc467;' ],

    'black' : [ 'fill:#202020;', 'fill:#373737;', 'fill:#6c6c6c;' ],
    'white' : [ 'fill:#d7d7d7;', 'fill:#ffffff;', 'fill:#ffffff;' ],
}

orig_svg = open(name,'r').read()

for c,arr in colors.items():
    svg = orig_svg
    for i in range(3):
        svg = svg.replace(orig[i], colors[c][i])
    with open('grid_'+c+'.svg',"w") as f:
        f.write(svg)

