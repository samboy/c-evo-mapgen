readme_if_you_want_to_understand_the_source_code.txt               2017-Apr-07
==============================================================================

DISCLAIMER:
This is not intended to be a complete documentation of the source code.
It is intended to be a collection of some facts which I think might be helpful
for you who wants to drill into the code.


Contents
--------
1. General remarks
2. Things that might surprise you / things I do in an unusual way
3. Terminology
4. Rough description of what steps the program does to generate a map
5. The hierarchy of files and folders; what is where
6. Rough description of the program structure


1. General remarks
------------------
Map_gen is written completely in ANSI-C.  Some very old functions in the
general purpose library might even be in K & R style.  C++ might have made the
job easier, but when I started the project back in 2006, all I had was an old
DOS C-compiler.  I hate to change stable code without a good reason, so I kept
it that way.

Before trying to understand map_gen, you should study Steffen's documentation
of the map file format (http://www.c-evo.org/mapspec.html).

Moreover, you should browse the whole file tree and read any "readme*.txt" and
"*.pdf" file, you will find additional information there.


2. Things that might surprise you / things I do in an unusual way
-----------------------------------------------------------------
First of all: The map_gen code is a mess.  The reason for this is that I wrote
the code over more than 10 years, and that I added things in later years
I didn't think of in 2006.  E. g. the first scenario, 'Navigation required',
doesn't make use of most of the library routines I wrote later, so if I would
write 'Navigation required' today, it could be coded much leaner.  I plan to
streamline the code in future releases, but that hasn't been done yet.  The
message is: Don't be surprised if you see many lines of code where a single
call to a lib function would do as well.


Some inline comments might be in german, not in english, sorry for that.
This doesn't apply to new code, but some library code map_gen uses dates back
to 199x.  Whenever I update old code, I usually translate german comments into
english, but I'm pretty sure there is still a lot around.
When you hit on a german comment, don't understand it but need to understand
it please feel free to contact me.


All filenames stick to the old MS-DOS "8.3" pattern.
The reason is that I started the project with an old MS-DOS C-compiler, which
was unable to process long filenames.  There is only one exception: The name of
the generated map file, whose extension is now ".cevo map".  I introduced that
exception when I changed to the Tiny-C compiler.


Sometimes I do silly things just to save a few bytes of memory.
Silly  *today*, since main memory capacities grew into the gigabyte scale.
But I worked more than six years as an assembler programmer for embedded
8-bit-microcontrollers.  Typically, these devices had an onboard memory of
128 or 256 bytes (!).  It was not unusual to spend a whole day of work just to
save one or two bytes of memory.
Moreover, when I started to program on 'real' computers at home (and not on
programmable pocket calculators, how I did before), the operating system
was CP/M and the main memory was 8 or 16 kByte.  All this trained me to save
memory whenever possible.
I know this is bad habit when it comes to writing programs for the PC, but
it is hard to avoid it when you did it so many years.  I try to change this
habit, but sometimes I do such silly things nearly automatically, without
thinking of the implications.
To give you an example: When I know a for loop will cycle less than 255 times,
I declare the loop control variable as '8 bit unsigned', just because it
doesn't need to be 16, 32 or 64 bit and just to save the 1, 3 or 7 bytes.
But on a 64-bit machine, the resulting code will probably be slower and bigger
when compared to an implementation with a variable which has the native
64-bit width.  The map_gen code is full of such things.


I don't use the 'bool' data type.
The reason is that I needed a one-bit type before C-compilers had it (at least
before the C-compilers  *I knew*  had it).  So I did what I learned from
specialized compilers for embedded controllers.  These controllers can address
single bits directly, without the overhead of addressing whole bytes and using
bit masks to isolate the target bit.  These specialized compilers used a new
data type called 'bit'.  I took over this naming, and as a compromise between
data space, code space and runtime (to avoid the overhead of bit masks on the
PC) I map bit variables to 'unsigned char'.  It is a typedef, and since all my
typedefs are upper case you will find all my code using
   typedef unsigned char BIT ;
instead of bool.


There is a lot of dead (commented out) code.
When working many years on a project but only from time to time it is important
to remember what you have tried before but what didn't work.  The best place
-- to my mind -- is directly inside the code, because that is the place you go
when you want to change something, so the notes are right under your nose.
Ideally, I place a comment right beside the dead code, saying
    /*this doesn't work because ...*/
Other typical constructs you will encounter are
#ifdef NEVER
  many lines of dead code here
#endif /*NEVER*/
or
#ifdef UNDER_CONSTRUCTION
  Code which I started but didn't finish yet.
  It is commented out to make sure it does not
  - generate compiler errors
  - interfere with other code in an unintended way
#endif /*UNDER_CONSTRUCTION*/

Other reasons for dead code are
- Printf's for debugging.  Sometimes I keep them so that I don't have
  to re-write them when I debug again.
- Code which I changed.  Usually I keep the old code so I can easily switch
  back in case it turns out there is a severe problem with the new code.
  I delete this old code after some time, usually when I regard the new code
  as stable (this may take years in some cases).
- When I write new programs/libraries, I use templates.  I  *uncomment*  what
  I need but leave the commented parts in their place, just in case I will
  need them in future modifications.


You will find all kinds of abstraction layers.
I use abstraction layers from compilers, file systems, operating systems
and some more.  Why?  Simply because I grew tired of working over all my
applications every time I migrated to a new platform/new compilers.
Here is my 'career' as a programmer in a few lines
(in chronological order, leaving out some minor branches):
-> programmable pocket calculators
  -> BASIC on home computers and pocket calculators
    -> assembler on microprocessors (Z80, 6502 etc.)
      -> Fortran and Cobol using punched card batch jobs (really!)
        -> C and assembler under Unix
          -> C++ under Unix
            -> assembler on 8-bit microcontrollers
              -> C under CP/M (again, really!  I lost my access to a Unix
                                           system when I left university)
                -> C under MS-DOS
                  -> C under Windows (3.11 .. W98)
                    -> C, C++ and script languages under Linux

As you can see, there were quite a few platform changes, including clear
downgrades like Unix->CP/M.  I tried to avoid or at least to minimize the
effort needed to port my applications to a new platform.  All I have to do now
(as long as I'm staying with ANSI-C) is to update the abstraction layers.
This concept clearly payed off when I ported map_gen from Windows to Linux.



3. Terminology
--------------
adjacent: the tiles which are adjacent to a tile are the four tiles which
   have a common edge with that tile.
basic tile type: the type of a tile without all modifications like bonus
   resources, river, irrigation, pollution etc.
   According to C-Evo's map specification, there are 11 basic tile types:
   ocean, coast, grassland, prairie, hills, forest, swamp, desert, tundra,
   arctic, mountains.
big island: this does not refer to size, but to shape.  The cluster generating
   algorithm for 'big' and 'small' islands is different, resulting in a
   different shape.  While 'big' islands have a rather smooth coastline (to
   make defense easier), 'small' islands have a ragged coastline, that is,
   their ~10 tiles are spreaded wider over the map, allowing more cities
   on them.  Theoretically, 'small islands' can be bigger than 'big islands',
   if parametrized accordingly.
big river: a ribbon made of water tiles used to split a continent into two
   or more parts.  I call it 'big river' to distinguish it from the regular
   C-Evo rivers, which are a tile modification.
city radius: the 20 tiles surrounding a city.  Map_gen uses this concept
   for non-city center tiles, too.
cluster: a group of tiles which logically belong together somehow.
   In map_gen, the tiles of a cluster don't need to be contiguous; they
   may be scattered over the whole map.
map type: synonym for scenario
neighborhood: the eight tiles surrounding a tile
north pole, south pole: the area above and below the map where
   tile indices are invalid
scenario:  synonym for map type, i. e. each one of the map types
   you can select in the [map_type] section of "map_gen.ini".
shore tile: a land tile which has at least one water tile in its neighborhood
small island: this does not refer to size, but to shape.  See "big island" for
   further explanation.
tagging, tags: map_gen associates eight tags (bits) with each tile on the map.
   These tags are heavily used during map generation, for things like marking 
   continents or lakes or other clusters, or for marking areas forbidden for
   starting positions, just to name a few examples.  Most tags are re-used for
   different purposes in the different stages of the generation process.
   The tags are stored in an array called "world_flags[]".
tile type list (TTL): a list of basic tile types stored in a single
   32-bit value, for easy and fast processing.
water tile: either ocean or coast tile


4. Rough description of what steps the program does to generate a map
---------------------------------------------------------------------
A. Common to all scenarios
  1. Read "map_gen.ini" and put all identifier/value pairs found there in
     corresponding global variables with the same name
  2. Allocate an array "world[]" which will hold the generated map, one tile
     per element.  Allocate an array "world_flags[]" of same size which will
     hold the tags associated with each tile.
  3. Determine which scenario to generate and pass control to the respective
     module (functions "scenario_*()").

B. Specific to a particular scenario (not each step is present in each scenario)
  1. Analyze which parameters have been specified in "map_gen.ini" and check
     for compatibility.  Set scenario-specific default values for parameters
     which did not show up in "map_gen.ini".
  2. Initialize world[] with a basic tile type which is typical for that
     scenario (e. g. ocean, mountains)
     *or*  fill world[] using another algorithm (see "scn_*.c" files)
  3. Modify world[] by placing clusters of tiles according to the scenario rules
  4. Eliminate one-tile-islands, if any.  This is done in C-Evo, too.  I guess
     the reason is that a city on a one-tile-island cannot be conquered before
     an attacker has Ballistics or Flight.
  5. Add rivers, possibly modifying some basic tile types.  Not all basic tile
     types are capable of having rivers.
  6. Add more clusters or other areas according to the scenario rules
  7. Convert ocean tiles to coast where appropriate
  8. Add bonus resources (standard distribution) and convert grassland tiles
     to plains where appropriate
  9. Add special resources according to scenario rules
 10. Add starting positions according to scenario rules
     (sometimes 9. and 10. are swapped for scenario specific reasons)
 11. Pass control back to main()

C. Common to all scenarios
  1. Write the map from "world[]" to a file with the specified name and exit


5. The hierarchy of files and folders; what is where
----------------------------------------------------
Below is a scheme of the source code tree, leaving out all readmes
and other documentation files as well as most build scripts:

map_gen-1.1.0/
| src/
  | build            the Linux master build script for map_gen
  | build.bat        the Windows master build script for map_gen.exe
  | make_dist_clean  Linux script to remove all files which have been
  |                  created during the build process
  | map_gen/
  | | map_gen.c  the 'main' function and other general functions
  | | map_gen.h  project-wide includes for map_gen
  | | read_ini.c functions that read "map_gen.ini"
  | | read_ini.h interface definition for "read_ini.c"
  | | aux_clus.c cluster generating routines
  | | aux_isla.c island generating routines; obsolete soon since it is
  | |            conceptually absorbed by clusters
  | | aux_lake.c lake generating routines; obsolete soon, too
  | | aux_rive.c river generating routines
  | | scn_<4letter_abbrevation_for_a_scenario>.c  the scenario modules
  | | ri/  auxiliary modules for reading the "map_gen.ini" file
  |   | read_ini.ini  definitions for all parameters in "map_gen.ini"
  |   | table.c       includes "read_ini.ini" to generate a name table
  |   |               for "read_ini.c"
  |   | globals.c     includes "read_ini.ini" to generate 'global' statements
  |   |               for "read_ini.c"
  |   | extern.c      includes "read_ini.ini" to generate 'extern' statements
  |                   for "read_ini.h"
  | cevo_lib/  the directory for C-Evo related modules; some might be
  | |          suitable for an AI, too
  | | cevo_map.h     interface definition for map-related functions in cevo_lib
  | | cevo_lib.c     most of the functions that make cevo_lib
  | | resource.c     functions that deal with resources, especially inside
  | |                city radius
  | | startpos.c     functions for choosing and placing player's starting
  |                  positions
  | lib/  the directory tree for my 'private' libs; too complicated to
  |       map it in detail here.  You will hardly need to change anything
  |       there if you want to modify map_gen.
  | include/  the includes for my 'private' libs
  | cpp/  the directory for a C preprocessor, see the readme you'll find there.
  |       You will hardly need to change anything there if you want to modify
  |       map_gen.
  | bin/  empty on delivery.  If you compile map_gen, the binary goes there


6. Rough description of the program structure
---------------------------------------------
The main() function is in src/map_gen/map_gen.c.
It reads all command line arguments (by calling the macro OPTION_GET).
It opens the log file.
It reads "map_gen.ini" (or whatever file was specified with the -i option)
by calling read_ini(). All parameters from "map_gen.ini" are now available
in global variables.

If read_ini() signalled success via its return value, some general
initializations are done.  Some by main() itself, others by cevo_lib_init()
which is called next.

After all general initializations are done, the respective scenario function
scenario_*() is called.  When it returned, the map is written by
write_map_to_file() to a file with the specified name.
The log file is closed and main() returns.

The interesting part is what happens between the call to scenario_*()
and its return.  Alas, the scenario functions use quite different methods
to generate a map.  I'll try to outline below what they do in general.  To
understand a specific scenario, you'll have to look into the source file
(scn_<4letter_abbrevation_for_a_scenario>.c).  Don't worry, scenario code
is typically a few hundred lines only, so this should be feasible.

As I mentioned before, the map is in an array called "world". Its size is
equal to the number of tiles on the map, its elements are of type
unsigned 32 bits, typedef'd to TILE.

To manage all kind of areas (continents, islands, lakes and other clusters),
there is a second array ("world_flags", same size and of type 8 bits).
These flags (aka tags) are heavily re-used during the map generation process.
Using some of the flags twice accidentially at the same time for different
things were the hardest-to-find bugs in map_gen.  Therefore I established a
system which keeps track which flags are currently in use and which are free.
There are two macros, WORLD_FLAGS_ALLOC(mask) and WORLD_FLAGS_FREE(mask)
which I place before and after a code section where I want to use the
world_flags specified by "mask".  A runtime error is raised when the flags
are already in use.  That fixed the problem.  Anyway, the mechanism is still
active in version 1.1.0.

To give you an idea how the scenarios work, let me describe two examples
(slightly simplified):

  Example 1: Navigation required
  The whole map (= world[]) is filled with OCEAN tiles (all uppercase
  terrain types refer to the basic tile types as defined in
  "src/cevo_lib/cevo_map.h").
  The northmost and the southmost lines are filled with ARCTIC.

  Next, we are placing the 'big islands', that is the islands where players
  have their starting positions.  First, we clear the tag with the symbolic
  name NO_BIG_ISLAND_HERE on the whole map. This tag indicates where we don't
  want 'big islands' to be generated.  Next, we set that bit along
  two thin stripes near the poles.  That is to inhibit the big islands to
  connect to the poles, which would establish a land bridge between the
  islands, and we don't want that.  To place an island, add_island()
  (in "aux_isla.c") is called.

  Here is what add_island() does:
  Of all tiles which have their NO_BIG_ISLAND_HERE tag cleared, one is
  randomly chosen as start point for the first island.  From that start point,
  neighborhood tiles are tagged with the ADD_LAND_HERE tags (there are three
  of them, forming a three-bit-counter).  Add_island() determines the
  neighborhood tiles by calling a function in lib_cevo, called
  set_neighborhood_tiles().  This function returns a struct which holds the
  indices of the eight neighborhood tiles.  Now, add_island() adds a second
  tile to the island by randomly choosing a tile that has its ADD_LAND_HERE
  tags set.  It *clears* that tags for the second tile to prevent it from
  beeing added to the island twice.  Furthermore, the tile is tagged with
  the tag with the symbolic name TAGGED to remember this tile is part of the
  new island.  The same is done for the third tile and so on until the island
  reaches its target size.  Finally, add_island() tags the NO_BIG_ISLAND_HERE
  flag for all tiles surrounding the new island in a distance less than or
  equal to big_island_dist (you specified that in "map_gen.ini", and it is now
  available in a global variable).  Add_island() uses another function from
  the library, dist_tile_to_group(), which determines the minimum distance
  between one tile and a group of tiles, where this group is specified
  by a certain tag pattern.

  When add_island() returns to scenario_nav_required(), the first island
  is placed on the map (in world[]), and the island tiles as well as the
  surrounding tiles in "big_island_dist" have their NO_BIG_ISLAND_HERE tags
  set.  Thus, when add_island() is called for the second island, it will be
  placed in a minimum distance of "big_island_dist" to the first island.
  Same for the third island, and so on, until all big islands are placed.

  Next come the 'small islands'; they are created by add_island(), too,
  but with different parameters.

  Eliminate_one_tile_islands(), set_ocean_and_coast() and set_bonus_resources()
  all do what their name tells.  These are general functions which can be used
  in every scenario, you will find them in "map_gen.c".

  Finally, on each player's island, the best starting position is determined
  by the function best_starting_position().  On the first three islands,
  a special resource is placed inside the city radius of the players'
  starting position.


  Example 2: Big river
  Again, the whole map is filled with OCEAN tiles.  Two 'anchor points' for
  the north and south land mass are picked randomly: The northern anchor from
  the top row, the southern from the bottom row.  The idea is to let the
  land masses grow towards each other and stop where they are as close as the
  "water_width" parameter tells.

  The northern anchor is tagged with the ADD_NORTH_HERE flag, the southern
  with ADD_SOUTH_HERE.  Tiles are added to the north and south cluster
  alternately in a while loop.  During the first cycle, there is only one
  tile on the whole map which has its ADD_NORTH_HERE flag set, and another
  one with the ADD_SOUTH_HERE flag, so these two tiles are added to the
  northern respective southern land mass.

  Whenever a northern tile is added to the northern land mass, its
  neighborhood tiles are tagged with the ADD_NORTH_HERE flag to indicate
  where more tiles should be added.  The tile itself is untagged to prevent
  it from being added a second time.  All tiles in a radius according to
  "water_width" are tagged with another flag, called NO_SOUTH.  This tag is
  used to mark all tiles where we don't want to have southern tiles.  The same
  is done when we add a southern tile, and so, southern and northern tiles
  cannot grow closer together than "water_width", leaving a ribbon of water
  tiles (remember? We initialized the whole map with water tiles) between them,
  the 'Big river'.

  In the while loop, when we want to add a northern tile, we randomly pick
  one tile of all tiles which have their ADD_NORTH_HERE flag set and their
  NO_NORTH flag unset.  We add this tile.  Then we do the same for the
  southern area.  The while loop ends when we can neither add more tiles to
  the northern nor to the southern land mass.  Now we have finished the basic
  shape of the map.

  Next is to add rivers to the map, this time 'regular' rivers.  This is done
  by a function called add_rivers(), in "aux_rive.c".  The code for adding
  rivers is too complicated to be covered here (740 lines of code, much more
  than most scenarios have) so let's skip it.  I gave a few hints about river
  generation in "history.txt".

  Finally we call set_ocean_and_coast() and set_bonus_resources()
  which enforce C-Evo map rules.

  Now we place the starting positions for computer opponents, first on the
  northern land mass.  We tag the whole northern land mass with the TAGGED
  flag to mark it for the placing routine.  To do this, we  1. clear all
  TAGGED flags on the whole map  2. tag the northern anchor tile  3. call
  tag_whole_land(), a library function.  Now the whole northern land mass is
  marked with the TAGGED flag.  There is another flag, called DONT_SETTLE_DOWN, 
  which disqualifies a tile as a starting position.  We clear this flag for
  all tiles on the map.

  Now we do a rating for all TAGGED tiles which have their DONT_SETTLE_DOWN
  flag cleared.  For the first city, this is the whole northern land mass.
  The library function best_starting_pos_simple() returns the index of the
  tile with the highest rating of all marked tiles.  You will find this
  function in "cevo_lib/startpos.c".  It marks all tiles in a radius
  "starting_pos_dist" (from "map_gen.ini") with the DONT_SETTLE_DOWN tag
  to prevent further city placement there, thus guaranteeing the mutual
  distance of starting positions.  We place the start position at the returned
  best index since the function doesn't do it, for a reason we will see in the
  next paragraph.  We do the same steps for all the other cities to be placed
  in the northern area.  Then we do the same thing for the cities to be placed
  in the southern area.  Finally we place the human starting position by
  tagging the respective area(s) and calling best_starting_pos_simple()
  a last time.

  Finally, we place the special resources and dead lands, six on each side.
  We start again with the northern area: we tag it like before and call the
  same library function again, best_starting_pos_simple().  Since it does
  *not* place a city marker, we can use the best index in a different way this
  time: We hand it over to another library function, set_special_resource().
  This function places a special resource in the city radius indicated by
  the best index, but not in the central tile.  Thus we have found a place
  for a potential city (at best index) with access to a special resource.
  We do this six times for the northern and six times for the southern area.
  Then we pass control back to main().




When comparing this desription with the source code, please keep in mind that
the examples are slightly simplified.  I concentrated on the "core mechanisms"
because I felt this would help you most.

-----------------------------------------------------
Hope this helps.  If you think something important is
missing, please feel free to contact me.

Ulrich Krueger
