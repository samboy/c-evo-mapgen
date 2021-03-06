# Sam Trenholme's notes ##################################################

I have modified map_gen.exe to:

* Compile under msys
* Use RadioGatun32 for the random number generator

Using the supplied map_gen.ini, running "./map_gen.exe -1"
should generated a file, nav_req.cevo map, with the MD5
sum of 7753c0263aabd4d304de6eb98d1e4cfa, as long as
the file seed2.txt is not present.

GPL compliance notes:

As per the GPL: I, Sam Trenholme, have modified the source code.

# map_x and map_y ########################################################

I have added the variables map_x and map_y which allow the map size
to be set by the user (this map generator does not have C-evo’s
pesky (x*y)+1 must be prime issue).  Note that this may cause
issues with the Great_plains and undocumented Fjords map; if using
one of those map types, use the map_size parameter instead.

The minimum map_x value is 23; the minimum map_y value is 34.
If both of these values are not set, we use map_size to set the map 
size instead.

# map_resources and map_oasis ############################################

I have also added the parameters map_resources and map_oasis.
map_resources, if 0, uses the old resource generation code.  If
between 1 and 100, this is the percentage of squares with bonus
resources.  map_oasis allows a second higher percentage of oasis
squares to be added to deserts (if 0, map_oasis does not do anything).

Note that, even if map_resources or map_oasis are set to 100, we
do not allow adjacent tiles to have resources.

# seed2.txt ##############################################################

If there is a file named seed2.txt in the same directory as 
map_gen.exe, this file will be read.  Any number in the file will
be used to modify the seed used by map_gen.exe.  The file may also
have comments, as long as no numerals are in the comments.

To use this feature:

	echo 12345 > seed2.txt
	./map_gen

# Year 2038 compliance ###################################################

This program is year 2038 compliant.  Mostly.  After the January 19, 2038
cutoff, this map generator will still happily make maps.  The maps will
vary, but, depending on which OS it is being run on, the maps may or
may not use a changing timestamp as part of the seed.  If the seed is
not specified on the command line, we use a combination of the timestamp
and 32 random bits to genrate a seed.  Even if the timestamp stops
changing (as it will in Windows 10 after the 2038 cutoff), each map
will still use a 32-bit random number as part of its seed.

There will be some warnings when generating the maps, but the map
generation will still work as before.

# Building this ##########################################################

To build this:

* Get an ancient version of mingw and msys
* Using GCC 3.2.3 from 2003, enter the `src/` directory
* Type in `./build`
* The binary will build
* The binary does not build in modern versions of Cygwin

# Original readme ########################################################

Welcome to "map_gen" version 1.1.0, an external map generator
for the turn based strategy game C-Evo.



## What map_gen is:

Map_gen is not an editor (which would show you the map while you are editing),
but a configurable random map generator which does *not* show you the map that
is being generated.  So as a human player you can play the generated map
without knowing what it looks like.



### Differences to C-Evo's built-in map generator and built-in map editor

(why I made an external map generator)

* C-Evo's built-in random map generator allows you to make standard maps.
  The only parameters under your control are 'Size' and 'Landmass'.
  It does not create non-standard maps, e. g. extreme waterworlds or large
  deserts.
* Map_gen, on the other hand, can create several types of maps (also called
  'scenarios'), each of which can be tuned to your desire by many parameters.
* C-Evo's built-in map editor allows you to create any (valid) map you can
  imagine.  But it  *shows*  you the map you are editing (well, it ought to,
  after all it is an editor).  Later, when you play the map as a human
  player, you will remember what the map looks like and that takes out a bit
  of the thrill (unless you suffer from amnesia or schizophrenia ;-).
* Map_gen does not show you the map it has generated.

### Supported platforms and C-Evo versions:

Map_gen has been developed and tested thoroughly under Linux and Windows98.
A short test confirmed that it runs under Windows 7 Starter without any
problems.  Since it uses nothing but file I/O, console I/O and the system time
(for time stamps in the log file and as seed for the random generator)
I'm pretty sure it will run on  *all*  Windows platforms.

C-Evo's map format has never changed so the generated maps work with
all C-Evo versions I know (0.8 - 1.2.0).

### How to use map_gen:

Map_gen consists of two files:

1. "map_gen.exe" (the program)
   For Linux, the executable is called "map_gen" (if you compile it
   from sources)
   or "map_gen32" (precompiled binary for 32-bit architecture)
   or "map_gen64" (precompiled binary for 64-bit architecture)
2. "map_gen.ini" (a sample configuration file for "map_gen.exe")

Copy both files to a directory of your choice (no installation required).

Edit "map_gen.ini" with a text editor, e. g. with notepad.  By editing it,
you determine the type of map (=scenario) which is to be generated as well
as all the related parameters.  You will find further instructions
inside "map_gen.ini".

@Linux users:
You might give the precompiled binaries map_gen32 or map_gen64 a try.  If they
work with your distro -- fine.  If they don't, just compile your own map_gen
from the sources.

### License

Map_gen 1.1.0 is distributed under the GNU General Public License (GPL).
See file "gpl-3.0.txt" for details.

    Copyright (C) 2017  Ulrich Krueger

    Map_gen is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Map_gen is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with map_gen.  If not, see <http://www.gnu.org/licenses/>.

### Feedback

If you wish to send me feedback and/or bug reports you will find my
email address at the bottom of this file.  When reporting a bug, please be
sure to attach

1. Your edited "map_gen.ini"
2. The log file "map_gen.log" which is generated along with the map file
  (I do not need the map file itself)

If you send me an email, it would be nice if you put the word 'map_gen'
somewhere in the subject.

Enjoy playing C-Evo with non-standard maps!




Contact:

```
ulrich.krueger.2                   nonsense.stuff.here.to.fool.clever.address
                  __at__           recovery.programs
                             t-online.de

2017-Mar-31  Ulrich Krueger


