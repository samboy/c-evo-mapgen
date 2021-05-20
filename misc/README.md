# What this is

With smaller C-evo maps, the map_gen.exe program can frequently fail.
This is a shell script (which runs fine under MSYS, even ancient MSYS)
which remakes a C-evo map with map_gen.exe until it succeeds, and
a .c program and .exe which tells us how far north or south the player
starts on the map, allowing the shell script to reject maps where
the player starts close to a pole.
