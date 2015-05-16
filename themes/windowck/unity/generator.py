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
import os
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
    for i in ("close", "maximize", "minimize", "menu", "unmaximize"):
        for j in ("focused_normal", "focused_prelight", "focused_pressed", "unfocused"):
            name = "%s_%s" % (i,j)
            if name in gvar:
                generate(name, gvar[name], gvar["%s_map" % name])
                os.system("convert %s.xpm %s.png" % (name,name))
        os.system("cp %s_focused_normal.png %s.png" % (i,i))

#close
close_focused_normal = '''
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
close_focused_normal_map = {
    "." : "None",
    "=" : "None",
    "+" : "#202020",
    "@" : "None",
    "#" : "None",
}

close_focused_prelight = close_focused_normal
close_focused_prelight_map = {
    "." : "None",
    "=" : "None",
    "+" : "#D92626",
    "@" : "#000000",
    "#" : "None",
}

close_focused_pressed = close_focused_normal
close_focused_pressed_map = {
    "." : "None",
    "=" : "None",
    "+" : "#D92626",
    "@" : "None",
    "#" : "None",
}

close_unfocused = close_focused_normal
close_unfocused_map = {
    "." : "None",
    "=" : "None",
    "+" : "#606060",
    "@" : "None",
    "#" : "None",
}

#hide
minimize_focused_normal = '''
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
minimize_focused_normal_map = close_focused_normal_map
minimize_focused_prelight = minimize_focused_normal
minimize_focused_prelight_map = {
    "." : "None",
    "=" : "None",
    "+" : "#000000",
    "@" : "#FFFFFF",
    "#" : "#000000",
}

minimize_focused_pressed = minimize_focused_normal
minimize_focused_pressed_map = {
    "." : "None",
    "=" : "None",
    "+" : "#000000",
    "@" : "None",
    "#" : "#000000",
}

minimize_unfocused = minimize_focused_normal
minimize_unfocused_map = close_unfocused_map

#maximize
maximize_focused_normal = '''
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
maximize_focused_normal_map = close_focused_normal_map
maximize_focused_prelight = maximize_focused_normal
maximize_focused_prelight_map = minimize_focused_prelight_map
maximize_focused_pressed = maximize_focused_normal
maximize_focused_pressed_map = minimize_focused_pressed_map
maximize_unfocused = maximize_focused_normal
maximize_unfocused_map = close_unfocused_map

#maximize-toggled
unmaximize_focused_normal = '''
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
unmaximize_focused_normal_map = close_focused_normal_map
unmaximize_focused_prelight = unmaximize_focused_normal
unmaximize_focused_prelight_map = minimize_focused_prelight_map
unmaximize_focused_pressed = unmaximize_focused_normal
unmaximize_focused_pressed_map = minimize_focused_pressed_map
unmaximize_unfocused = unmaximize_focused_normal
unmaximize_unfocused_map = close_unfocused_map

#menu
menu_focused_normal = '''
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
menu_focused_normal_map = close_focused_normal_map
menu_focused_prelight = menu_focused_normal
menu_focused_prelight_map = minimize_focused_prelight_map
menu_focused_pressed = menu_focused_normal
menu_focused_pressed_map = minimize_focused_prelight_map
menu_unfocused = menu_focused_normal
menu_unfocused_map = close_unfocused_map

if __name__ == "__main__":
    build()
