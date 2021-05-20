#!/bin/sh
# Let's make a desert C-evo map with a start near the equator
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
	echo FindStarts not found
	exit 1
fi

if [ -e "map_gen.ini" ] ; then
	echo map_gen.ini already exists\; will not overwrite
	exit 1
fi

INDEX="$1"
if [ -z "$INDEX" ] ; then
	INDEX=0
fi

DATE="$2"
if [ -z "$DATE" ] ; then
	DATE=$(date +%Y-%m-%d)
fi

cat > map_gen.ini << EOF
[map_type]
map_type = Desert
[Desert]
mapfile = "desert.cevo map"  
map_size = 35 

# The actual size of the map
# Long, narrow desert
map_x = 75
map_y = 34
land_amount = 7

# How many bonus resources to place on the map
map_resources = 5
map_oasis = 12

comp_opponents = 1  # number of computer players: minimum 1, maximum 14
starting_pos_dist = 3200 # guaranteed minimum distance between starting
                        # positions
                        # (old movement point definition: 100/150 MP per tile)

startpos_min_rating = 64
grassland_percentage = 11   
grassland_min_size = 2  
grassland_max_size = 17 
prairie_percentage = 11  
prairie_min_size = 2  
prairie_max_size = 17  
EOF

MAP="MAYBE"

rm -f "desert.cevo map"

while [ "$MAP" != "GOOD" ] ; do
	INDEX=$( expr $INDEX + 1 )
	echo Try number $INDEX
	if ./map_gen.exe -s $DATE-r$INDEX > /dev/null 2>&1 ; then
		$FINDSTARTS "desert.cevo map" 
		MAP=$($FINDSTARTS "desert.cevo map" | awk '
			{if($NF < 30){print "GOOD"}}')
	fi
done

mv "desert.cevo map" "${DATE}-r${INDEX}.cevo map"
echo "${DATE}-r${INDEX}.cevo map" written

rm map_gen.ini
