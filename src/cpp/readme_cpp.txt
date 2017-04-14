Cpp is a 'torso' of a C preprocessor I once started for fun but never
finished.  It comes in handy for map_gen because it can concatenate
identifier names, and that is what is needed for map_gen.

It seems that nowadays compilers no longer have that ability (some had it
in 198x if I remember correct).  GNU General Public License requires to supply
all non-standard tools needed to build the project from sources, and so I had
to ship my version of cpp along with map_gen.  Anyway, I don't feel happy to
release an incomplete cpp (a torso) and I strongly discourage you to use it
for anything else than compiling map_gen.
