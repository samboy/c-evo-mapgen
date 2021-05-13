#!/bin/sh

# Here is an example script where we try a lot of times to make a good
# small desert map.  This requires a compiled copy of c-evo-mapgen and
# a compiled copy of findStarts.c, with the name cEvo-findStarts

# Make sure to delete map_gen.ini *before* running this script

# It's hard to both make a good small desert map and have the player start
# near the center

# This command is the compiled version of finsStarts.c
FINDSTARTS="./findStarts"
MAPGEN="./map_gen"

if [ ! -e "$MAPGEN" ] ; then
	cp ../map_gen.exe .
fi
if [ ! -e "$MAPGEN" ] ; then
	echo map_gen not found
	exit 1
fi

if [ ! -e "$FINDSTARTS" ] ; then
	echo FindStarts not found\; compiling
	cc -o ./findStarts findStarts.c
	FINDSTARTS="./findStarts"
fi
if [ ! -e "$FINDSTARTS" ] ; then
	echo FindStaets not found
	exit 1
fi

if [ -e "map_gen.ini" ] ; then
	echo map_gen.ini already exists\; will not overwrite
	exit 1
fi

cat > map_gen.ini << EOF
[map_type]
map_type = Desert
[Desert]
mapfile = "desert.cevo map"  # the name of the map file to be generated
map_size = 35  # Map_size may be 35, 50, 70, 100, 150 or 230 (percent)

# The actual size of the map
map_x = 25
map_y = 34

# How many bonus resources to place on the map
map_resources = 5
map_oasis = 12

comp_opponents = 1  # number of computer players: minimum 1, maximum 14
starting_pos_dist = 1600 # guaranteed minimum distance between starting
                        # positions
                        # (old movement point definition: 100/150 MP per tile)

startpos_min_rating = 64

   # Add some grassland clusters near the shore
grassland_percentage = 11   # amount of grassland tiles as percentage of
                           # total tiles on the map
grassland_min_size = 2  # minimum cluster size in tiles
grassland_max_size = 17  # maximum cluster size in tiles

   # Add some prairie clusters near the shore and rivers
prairie_percentage = 11   # amount of prairie tiles as percentage of
                         # total tiles on the map
prairie_min_size = 2  # minimum cluster size in tiles
prairie_max_size = 17  # maximum cluster size in tiles
EOF

MAP="MAYBE"
TRY=1

while [ "$MAP" != "GOOD" ] ; do
	echo Try number $TRY
	TRY=$( expr $TRY + 1 )
	if ./map_gen.exe > /dev/null 2>&1 ; then
		$FINDSTARTS "desert.cevo map" 
		MAP=$($FINDSTARTS "desert.cevo map" | awk '
			{if($NF < 30){print "GOOD"}}')
	fi
done

INDEX=1
DATE=$(date +%Y-%m-%d)
while [ -e "${DATE}-${INDEX}.cevo map" ] ; do
	INDEX=$( expr $INDEX + 1 )
done
mv "desert.cevo map" "${DATE}-${INDEX}.cevo map"
echo "${DATE}-${INDEX}.cevo map" written

