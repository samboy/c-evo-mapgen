/*  This file is part of map_gen, an external random map generator for C-Evo
 *  Copyright (C) 2017  Ulrich Krueger
 *
 *  Map_gen is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Map_gen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with map_gen.  If not, see <http://www.gnu.org/licenses/>.
 */
/********************************************************** Ulrich Krueger **
aux_lake.c

This lib handles lakes for "map_gen".
*****************************************************************************
History: (latest change first)
2017-Mar-25: changed old fprintf(stderr, ...)  to log file output
2015-Feb-06: ignore error "premature end of lake generation"
2012-Sep-22: now using WOLRD_FLAGS_ALLOC
2012-Sep-21: - dist_tile_to_group with world_flags
             - renamed   lakes.c -> aux_lake.c
2012-Feb-14: changed return type of "add_lake" to TIDX
2010-May-14: no use of internal static return values (NULL pointer arg)
2010-May-02: make use of "map_gen_exit"
2008-Apr-25: make coastlines RAGGED or SMOOTH
2008-Mar-17..20: add functionality to "add_lake"
2008-Mar-01..16: Derived from "map_gen"
*****************************************************************************
Global objects:
-  TIDX add_lake()
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/


#ifdef NEVER
                            /*should be equivalent to "add_lake"*/
                            /*Has to be tested*/
  CLUSTER_PARAMS clus ;
    no_of_tiles_target = (lake_percentage * total_no_of_tiles) / 100 ;
    no_of_tiles = 0 ;
    PRT_VAR(no_of_tiles_target,u)

    WORLD_FLAGS_ALLOC( TAGGED | NO_LAKE_HERE | IS_SHORE )
    clear_flags(       TAGGED | NO_LAKE_HERE | IS_SHORE ) ;

    delta_idx = MOUNTAIN_STRIPE * LX ; /*inhibit lakes in mountain stripe*/
    for ( tile_index = 0 ; tile_index <= delta_idx ; tile_index++ ) {
      world_flags [                     tile_index ] |= NO_LAKE_HERE ;
      world_flags [ total_no_of_tiles - tile_index ] |= NO_LAKE_HERE ;
    }

    clus.method = CLUSTER_METHOD_LAKE_SMOOTH ;
    clus.no_of_clusters = 1 ;
    clus.area_mask = NO_LAKE_HERE ;
    clus.area = 0x0 ;
    clus.area_only_for_start_points = FALSE ;
    /*clus.size = xxx ;*/
    clus.min_size = min_lake_size ;
    clus.tile_func = ocean ;
    clus.set_flags = TAGGED ;
    clus.clear_flags = 0x0 ;
    clus.edge_flag = IS_SHORE ;
    clus.grow_into = TTL_ANY ;
    clus.task_str = "add lakes" ;
    clus.flag1 = NO_LAKE_HERE ;
    clus.dist1 = lake_dist ;
    clus.flag2 = 0x0 ;
    clus.dist2 = 0 ;

    while (no_of_tiles < no_of_tiles_target) {
      clus.size = random_draw_range( min_lake_size, max_lake_size ) ;
      /*add_lake( lake_size, lake_dist, IS_SHORE, SMOOTH ) ;*/
      add_clusters( & clus ) ;
      no_of_tiles += clus.actual_size ;
    } /*end of lake generation*/
    WORLD_FLAGS_FREE( TAGGED | NO_LAKE_HERE | IS_SHORE )
#endif /*NEVER*/

/*--  include files  ------------------------------------------------------*/

#include <misc.h>

/*#define DEBUG*/
#include <debug.h>

#include "map_gen.h"


/*--  constants  ----------------------------------------------------------*/


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} PROBABILITY_VECTOR ;*/


/*--  local function prototypes  ------------------------------------------*/

static void add_tile_to_lake( U32 tile_index, U8 shore_mask ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/



/*-------------------->   add_lake   <--------------------------- 2017-Mar-25
Diese Funktion fuegt einen See hinzu.
Die Startposition wird zufaellig gewaehlt, dabei wird das world_flag
NO_LAKE_HERE beachtet.
Alle See-Tiles sind OCEAN; sie sind TAGGED.
In einer Entfernung von "distance" vom See werden weitere Seen durch
NO_LAKE_HERE Flags verhindert.
Kuesten-Tiles werden mit "shore_mask" markiert.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- size     ... of lake
                - distance    min distance to next lake
                - shore_mask  shore tiles are tagged with mask value
                - mode = RAGGED or SMOOTH
Return value:	Tile index of start tile
Exitcode:	x
---------------------------------------------------------------------------*/
TIDX add_lake( U16 size, U16 distance, U8 shore_mask, U8 mode )
{
THIS_FUNC(add_lake)
char* _this_func = "add_lake" ;
  U32 tile_index ;
  U32 tile_to_add ;
  U32 start_tile ;
  U16 i ;

  WORLD_FLAGS_ALLOC( ADD_HERE_ALL | TAGGED )
  clear_flags( ADD_HERE_ALL | TAGGED ) ;
  start_tile = draw_on_flag_pattern_match( NO_LAKE_HERE, 0x00 ) ;
  world_flags [ start_tile ] |= ADD_HERE0 ;

  for ( i = 1 ; i <= size ; i++ ) {
    tile_to_add = 0xffffffff ; /*default*/
    if (mode == SMOOTH) { /*highest prio*/
      tile_to_add = draw_on_flag_pattern_match( ADD_HERE2, ADD_HERE2 ) ;
    }
    if (tile_to_add == 0xffffffff) { /*no tile with highest prio found*/
      tile_to_add = draw_on_flag_pattern_match( ADD_HERE1, ADD_HERE1 ) ;/*prio*/
      if ((tile_to_add == 0xffffffff) || (random_draw_range( 1, 100 ) > 50)) {
        tile_to_add = draw_on_flag_pattern_match( ADD_HERE0, ADD_HERE0 ) ;
        if (tile_to_add == 0xffffffff) { /*no tile found!*/
          log_with_timestamp( "premature end of lake generation\n" ) ;
          /*map_gen_exit() ;*/
          break ; /*go on, ignore that lake is too small*/
        }
      }
    }
    add_tile_to_lake( tile_to_add, shore_mask ) ;
  }
     /*set NO_LAKE_HERE flags*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles -1 ; tile_index++ ) {
    if ((world_flags [ tile_index ] & NO_LAKE_HERE) == 0) {
      if (dist_tile_to_group( tile_index, TAGGED, TAGGED ) < distance ) {
        world_flags [tile_index] |= NO_LAKE_HERE ;
      }
    }
  }
  WORLD_FLAGS_FREE( ADD_HERE_ALL | TAGGED )
  return start_tile ;
}

/*-------------------->   add_tile_to_lake   <------------------- 2010-May-14
This function adds one OCEAN tile to a lake.
The ADD_HERE flags are cleared to prevent adding the tile twice.
The TAGGED flag is set to mark the tile as a member of the lake (for further
lake-oriented processing).
The NO_LAKE_HERE flag is set to prevent further setting of the ADD_HERE flag.
Finally, the ADD_HERE flags of the eight neighborhood tiles are set unless
they have the TAGGED or NO_LAKE_HERE flags set.
Their ADD_HERE1 flags are set if all 4 adjacent tiles are water (prio).
Their ADD_HERE2 flags are set if 5 or more neighborhood tiles are water (even
higher prio).
Adjacent land tiles are tagged as shore (shore_mask).
-----------------------------------------------------------------------------
Used functions:
Parameters:	- tile_index   This tile is added to the lake
                - shore_mask  shore tiles are tagged with mask value
                              (shore = land tile with water access)
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
static void add_tile_to_lake( U32 tile_index, U8 shore_mask )
{
/*THIS_FUNC(add_tile_to_lake)*/
  NEIGHBORHOOD NT ;
  NEIGHBORHOOD* NTp ;
  U32* tile_idxp ;
  int i ; /*loop control*/

  world [ tile_index ] = OCEAN ;
  world_flags [ tile_index ] &= ~(ADD_HERE_ALL | shore_mask) ;
  world_flags [ tile_index ] |= (TAGGED | NO_LAKE_HERE) ;
  NTp = set_neighborhood_tiles( tile_index, & NT ) ;
  for ( i = 0, tile_idxp = & (NTp->n_idx) ; i < 8 ; i++, tile_idxp++ ) {
    tile_index = *tile_idxp ;
    if (tile_index < total_no_of_tiles) {
      if (world [ tile_index ] != OCEAN) {
        world_flags [ tile_index ] |= shore_mask ;
        if (((world_flags [ tile_index ] ) & NO_LAKE_HERE) == 0) {
          world_flags [ tile_index ] |= ADD_HERE0 ;
          if (count_adjacent_tiles( tile_index, TTL_WATER ) == 4) { /*prio*/
            world_flags [ tile_index ] |= ADD_HERE1 ;
          }
          if (count_neighborhood_tiles( tile_index, TTL_WATER ) > 4) {
               /*even higher prio*/
            world_flags [ tile_index ] |= ADD_HERE2 ;
          }
        }
      } /*end if  *not*  OCEAN*/
    } /*end if inside map area*/
  } /*end for loop (neighborhood tiles)*/
}

/*-------------------->   x   <---------------------------------- 2008-Mar-16
This function x
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
/*x*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/
/***************************************************************************/
