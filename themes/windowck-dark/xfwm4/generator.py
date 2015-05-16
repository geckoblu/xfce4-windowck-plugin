#!/usr/bin/env python3
'''
    simple-gtk xpm generator
    
    Copyright (C) 2012  Felipe A. Hernandez <spayder26@gmail.com>
    Portions adapted by Cedric Leporcq.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

'''

from os import linesep


def gendeg2(color1, color2, steps):
    ''' Generates a list of colors being a gradient from color1 to color2 on
        given steps number. '''
    def fix(c):
        if c < 0: return 0
        elif c > 255: return 255
        return int(round(c))
    a = [0]*steps
    c1 = (int(color1[1:3],16),int(color1[3:5],16),int(color1[5:7],16))
    c2 = (int(color2[1:3],16),int(color2[3:5],16),int(color2[5:7],16))
    ci = (float(c2[0]-c1[0])/(steps+1),float(c2[1]-c1[1])/(steps+1),float(c2[2]-c1[2])/(steps+1))
    tr = [c1]
    for i in range(steps):
        tr.append((fix(tr[-1][0]+ci[0]),fix(tr[-1][1]+ci[1]),fix(tr[-1][2]+ci[2])))
    tr.append(c2)
    return [("#%2s%2s%2s" % (hex(i[0])[2:],hex(i[1])[2:],hex(i[2])[2:])).replace(" ", "0") for i in tr]

def gendeg3(color1, color2, color3, steps):
    ''' Generates a list of colors being a gradient from color1 to color3, with
        color2 at middle, with given steps number between each color. '''
    return gendeg(color1,color2,steps)[:-1]+gendeg(color2,color3,steps)

def gendeg(*args):
    ''' Interface between gendeg2 or gendeg3 depending on arg number '''
    if len(args) == 3: return gendeg2(*args)
    elif len(args) == 4: return gendeg3(*args)
    raise NotImplemented("Bad arguments, see gendeg2 and gendeg3 documentation.")

def genmap(dmap, chars, *gendeg_args):
    ''' 

    '''
    r = dict(zip(chars,gendeg(*gendeg_args)))
    r.update(dmap)
    return r

def generate(name, txt, dic, x0=0, y0=0, w=None, h=None):
    ''' Creates xpm file with given name, given draw as string, colors as dict.
        Extra args are for generate parts of xpm.
    '''
    if w is None:
        w = len(txt.split("\n")[0])
    if h is None:
        h = len(txt.split("\n"))
    x1 = x0 + w
    y1 = y0 + h
    colors = {}
    lines = [i[x0:x1] for i in txt.split("\n")[y0:y1]]
    for i in lines:
        for j in i:
            if j not in colors:
                colors[j] = dic[j]
    xpmlines = [
        "/* XPM */",
        "static char * %s = {" % name.replace("-", "_"),
        "\"%d %d %d 1\", " % (w, h, len(colors))
        ]
    xpmlines.extend(
        "\"%s\tc %s\", " % i for i in list(colors.items())
        )
    xpmlines.extend(
        "\"%s\", " % i for i in lines
        )
    xpmlines.append(
        "};"
        )
    with open("%s.xpm" % name,"w") as f: f.write(linesep.join(xpmlines))

def holePos(txt):
    ''' Detects a hole on a xpm string, used to find border sizes.'''
    lines = txt.split("\n")
    for i in range(len(lines)):
        if " " in lines[i]:
            return (lines[i].find(" "),i)
    raise ValueError

def holeSize(txt):
    ''' Detects hole on a xpm string, used to find border sizes.'''
    lastwidth = 0
    inhole = 0
    for line in txt.split("\n"):
        if " " in line:
            lastwidth = line.count(" ")
            inhole += 1
        elif inhole > 0:
            return (lastwidth, inhole)
    raise ValueError

def build():
    gvar = globals()
    for i in ("close", "hide", "maximize", "maximize_toggled", "menu", "shade", "shade_toggled", "stick", "stick_toggled"):
        for j in ("active", "pressed", "inactive", "prelight"):
            name = "%s_%s" % (i,j)
            if name in gvar:
                generate(name.replace("_", "-"), gvar[name], gvar["%s_map" % name])
    
    alw, alh = (len(active.split("\n")[0]),len(active.split("\n")))
    ilw, ilh = (len(inactive.split("\n")[0]),len(inactive.split("\n")))

    ''' Find corner length on a xpm string.'''
    acw= len(active.split("+")[0])
    icw= len(inactive.split("+")[0])

    ahx, ahy = holePos(active)
    ihx, ihy = holePos(inactive)
    ahw, ahh = holeSize(active)
    ihw, ihh = holeSize(inactive)
    abw, abh = (alw-ahx-ahw, alh-ahy-ahh)
    ibw, ibh = (ilw-ihx-ihw, ilh-ihy-ihh)
    
    #top-left
    generate("top-left-active", active, amap, 0, 0, acw, ahy)
    generate("top-left-inactive", inactive, imap, 0, 0, icw, ihy)
    #left
    generate("left-active", active, amap, 0, ahy, ahx, ahh)
    generate("left-inactive", inactive, imap, 0, ihy, ihx, ihh)
    #bottom-left
    generate("bottom-left-active", active, amap, 0, ahy+ahh, ahx, abh)
    generate("bottom-left-inactive", inactive, imap, 0, ihy+ihh, ihx, ibh)

    #top-right
    generate("top-right-active", active, amap, alw-acw, 0, acw, ahy)
    generate("top-right-inactive", inactive, imap, ilw-icw, 0, icw, ihy)
    #right
    generate("right-active", active, amap, ahx+ahw, ahy, abw, ahh)
    generate("right-inactive", inactive, imap, ihx+ihw, ihy,  ibw, ihh)
    #bottom-right
    generate("bottom-right-active", active, amap, ahx+ahw, ahy+ahh, abw, abh)
    generate("bottom-right-inactive", inactive, imap, ihx+ihw, ihy+ihh, ibw, ibh)

    #top
    for i in range(1,6):
        generate("title-%d-active" % i, active, amap, acw, 0, alw-2*acw, ahy)
        generate("title-%d-inactive" % i, inactive, imap, icw, 0, alw-2*icw, ihy)

    #bottom
    generate("bottom-active", active, amap, ahx, ahy+ahh, ahw, ibh)
    generate("bottom-inactive", inactive, imap, ihx, ihy+ihh, ihw, ibh)

#close
close_active = '''
..................
..................
==================
=====@@====@@=====
====@++@==@++@====
====@+++@@+++@====
=====@++++++@=====
======@++++@======
======@++++@======
=====@++++++@=====
====@+++@@+++@====
====@++@==@++@====
=====@@====@@=====
==================
'''.strip()

chars = ["+", "@", "#", "=", "-"]
dmap = [(".", "None")]
close_active_map = {
    "." : "None",
    "=" : "#3C3C3C",
    "+" : "#E6E6E6",
    "@" : "#3C3C3C",
    "#" : "None",
}

close_prelight = close_active
close_prelight_map = {
    "." : "None",
    "=" : "#3C3C3C",
    "+" : "#D92626",
    "@" : "#000000",
    "#" : "None",
}

close_pressed = close_active
close_pressed_map = {
    "." : "None",
    "=" : "#3C3C3C",
    "+" : "#D92626",
    "@" : "#3C3C3C",
    "#" : "#000000",
}

close_inactive = close_active
close_inactive_map = {
    "." : "None",
    "=" : "#3C3C3C",
    "+" : "#A1A1A1",
    "@" : "#3C3C3C",
    "#" : "None",
}

#hide
hide_active = '''
..................
..................
==================
==================
==================
==================
==================
==================
==================
===@@@@@@@@@@@@===
===@++++++++++@===
===@++++++++++@===
===@@@@@@@@@@@@===
==================
'''.strip()
hide_active_map = close_active_map
hide_prelight = hide_active
hide_prelight_map = {
    "." : "None",
    "=" : "#3C3C3C",
    "+" : "#FFFFFF",
    "@" : "#000000",
}

hide_pressed = hide_active
hide_pressed_map = {
    "." : "None",
    "=" : "#3C3C3C",
    "+" : "#FFFFFF",
    "@" : "#3C3C3C",
}

hide_inactive = hide_active
hide_inactive_map = close_inactive_map

#maximize
maximize_active = '''
..................
..................
===@@@@@@@@@@@@===
===@++++++++++@===
===@++++++++++@===
===@+@@@@@@@@+@===
===@+@======@+@===
===@+@======@+@===
===@+@======@+@===
===@+@======@+@===
===@+@@@@@@@@+@===
===@++++++++++@===
===@@@@@@@@@@@@===
==================
'''.strip()
maximize_active_map = close_active_map
maximize_prelight = maximize_active
maximize_prelight_map = hide_prelight_map
maximize_pressed = maximize_active
maximize_pressed_map = hide_pressed_map
maximize_inactive = maximize_active
maximize_inactive_map = close_inactive_map

#maximize-toggled
maximize_toggled_active = '''
..................
..................
==================
==================
====@@@@@@@@@@====
====@++++++++@====
====@++++++++@====
====@+@@@@@@+@====
====@+@====@+@====
====@+@====@+@====
====@+@@@@@@+@====
====@++++++++@====
====@@@@@@@@@@====
==================
'''.strip()
maximize_toggled_active_map = close_active_map
maximize_toggled_prelight = maximize_toggled_active
maximize_toggled_prelight_map = hide_prelight_map
maximize_toggled_pressed = maximize_toggled_active
maximize_toggled_pressed_map = hide_pressed_map
maximize_toggled_inactive = maximize_toggled_active
maximize_toggled_inactive_map = close_inactive_map

#menu
menu_active = '''
..................
..................
==================
==================
==================
====@@@@@@@@@@====
====@++++++++@====
=====@++++++@=====
======@++++@======
=======@++@=======
========@@========
==================
==================
==================
'''.strip()
menu_active_map = close_active_map
menu_prelight = menu_active
menu_prelight_map = hide_prelight_map
menu_pressed = menu_active
menu_pressed_map = hide_prelight_map
menu_inactive = menu_active
menu_inactive_map = close_inactive_map

#shade
shade_active = '''
..................
..................
==================
========@@========
=======@++@=======
======@++++@======
=====@++++++@=====
====@++++++++@====
====@++@++@++@====
=====@@@++@@@=====
=======@++@=======
=======@++@=======
========@@========
==================
'''.strip()
shade_active_map = close_active_map
shade_prelight = shade_active
shade_prelight_map = hide_prelight_map
shade_pressed = shade_active
shade_pressed_map = hide_pressed_map
shade_inactive = shade_active
shade_inactive_map = menu_inactive_map

#shade-toggled
shade_toggled_active = '''
..................
..................
==================
========@@========
=======@++@=======
=======@++@=======
=====@@@++@@@=====
====@++@++@++@====
====@++++++++@====
=====@++++++@=====
======@++++@======
=======@++@=======
========@@========
==================
'''.strip()
shade_toggled_active_map = close_active_map
shade_toggled_prelight = shade_toggled_active
shade_toggled_prelight_map = hide_prelight_map
shade_toggled_pressed = shade_toggled_active
shade_toggled_pressed_map = hide_pressed_map
shade_toggled_inactive = shade_toggled_active
shade_toggled_inactive_map = menu_inactive_map

#stick
stick_active = '''
..................
..................
==================
=======@@@@=======
======@++++@======
=====@++++++@=====
====@++@@@@++@====
====@++@==@++@====
====@++@==@++@====
====@++@@@@++@====
=====@++++++@=====
======@++++@======
=======@@@@=======
==================
'''.strip()
stick_active_map = close_active_map
stick_prelight = stick_active
stick_prelight_map = hide_prelight_map
stick_pressed = stick_active
stick_pressed_map = hide_pressed_map
stick_inactive = stick_active
stick_inactive_map = menu_inactive_map

#stick-toggled
stick_toggled_active = '''
..................
..................
==================
=======@@@@=======
======@++++@======
=====@++++++@=====
====@++@@@@++@====
====@++@++@++@====
====@++@++@++@====
====@++@@@@++@====
=====@++++++@=====
======@++++@======
=======@@@@=======
==================
'''.strip()
stick_toggled_active_map = close_active_map
stick_toggled_prelight = stick_toggled_active
stick_toggled_prelight_map = hide_prelight_map
stick_toggled_pressed = stick_toggled_active
stick_toggled_pressed_map = hide_pressed_map
stick_toggled_inactive = stick_toggled_active
stick_toggled_inactive_map = menu_inactive_map


active = '''
..+++++++++..
.+%%%%%%%%%+.
+%%%%%%%%%%%+
+%%%%%%%%%%%+
+%%%%%%%%%%%+
+%%%%%%%%%%%+
+%%%%%%%%%%%+
+%%%%%%%%%%%+
+%%%%%%%%%%%+
+%%%%%%%%%%%+
+%%%%%%%%%%%+
+%%%%%%%%%%%+
+%%%%%%%%%%%+
+%%%%%%%%%%%+
+%         %+
+%%%%%%%%%%%+
+%%%%%%%%%%%+
.+++++++++++. 
'''.strip()
amap = {
    "=" : "#000000",
    "+" : "#000000",
    "%" : "#3C3C3C",
    "#" : "#000000",
    "." : "None",
}
inactive = active
imap = amap

if __name__ == "__main__":
    build()
