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
aux_isla.c

This file generates islands for a C-Evo map
*****************************************************************************
History: (latest change first)
2017-Feb-18..19: debugging (segmentation fault)
2016-Jul-16: adoptions for gcc
2014-Jul-10: error msg copied to stdout
2014-Oct-07: basic_prob for MOUNTAINS and GRASSLAND
2013-Apr-11: ALLOCated world flags
2012-Sep-21: - dist_tile_to_group mit world_flags
             - renamed   islands.c -> aux_isla.c
2010-May-14: No use of internal static return values (NULL pointer arg)
2010-May-13: Added documentation.  Minor changes (make use of lib funcs)
2008-Jun-01: Derived from "map_gen.c"
*****************************************************************************
Global objects:
- U32 add_island( U16 size, U16 min_size, U8 world_flags_mask,
                  U16 big_island_dist, U16 small_island_dist,
                  U32 bp_desert, U32 bp_prairie, U32 bp_tundra )
- TILE draw_island_tile( U32 tile_index,
         U32 bp_desert, U32 bp_prairie, U32 bp_tundra )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

#ifdef NEVER
                            /*should be equivalent to big island*/
                            /*Has to be tested*/
  CLUSTER_PARAMS clus ;
    clus.method = CLUSTER_METHOD_ISLAND ;
    clus.no_of_clusters = 1 ;
    clus.area_mask = xxx ;
    clus.area = xxx ;
    clus.area_only_for_start_points = FALSE ;
    /*clus.size = xxx ;*/
    clus.min_size = big_island_min_size ;
    clus.tile_func = return_land_tile ;
    clus.set_flags = TAGGED | NO_BIG_ISLAND_HERE | NO_SMALL_ISLAND_HERE ;
    clus.clear_flags = 0x0 ;
    clus.edge_flag = 0x0 ;
    clus.grow_into = TTL_ANY ;
    clus.task_str = "add big island" ;
    clus.flag1 = NO_BIG_ISLAND_HERE ;
    clus.dist1 = big_island_dist ;
    clus.flag2 = NO_SMALL_ISLAND_HERE ;
    clus.dist2 = small_island_dist ;
                            /*should be equivalent to small island*/
    clus.method = CLUSTER_METHOD_ADJACENT ;
#endif /*NEVER*/


/*--  include files  ------------------------------------------------------*/

/*#define DEBUG*/
#include <debug.h>

#include "map_gen.h"
#include "read_ini.h"


/*--  constants  ----------------------------------------------------------*/


/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

static void add_tile_to_island( TIDX tile_idx, U8 mask,
                 U32 bp_desert, U32 bp_prairie, U32 bp_tundra ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/



/*--  internal variables  -------------------------------------------------*/



/*-------------------->   add_island   <------------------------- 2017-Feb-18
This function adds either a so-called big or a small island to the map.
'Big' and 'small' islands are two conceptual classes of islands.
They differ in the ruggedness of the coastline.
They have a defined minimum distance between each class member.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- size    ... of island
                - min_size   If the algorithm cannot find any more tiles to
                             add but min_size is reached, the function stops
                             adding tiles and returns.  If min_size cannot
                             be reached, the function exits.

		- world_flag_mask
		    is NO_BIG_ISLAND_HERE for big islands
		    is NO_SMALL_ISLAND_HERE for small islands
                - big_island_dist     All tiles closer than big_island_dist
                                      are tagged with NO_BIG_ISLAND_HERE
                - small_island_dist   All tiles closer than small_island_dist
                                      are tagged with NO_SMALL_ISLAND_HERE
                - bp_*       Basic probability for these tile types
Return value:	Tile index of start tile
Exitcode:	see "map_gen_exit"
---------------------------------------------------------------------------*/
TIDX add_island( U16 size, U16 min_size, U8 world_flags_mask,
                U16 big_island_dist, U16 small_island_dist,
                U32 bp_desert, U32 bp_prairie, U32 bp_tundra )
{
THIS_FUNC(add_island)
char* _this_func = "add_island" ; /*for WORLD_FLAGS_ALLOC*/
  TIDX tile_index ;
  TIDX tile_to_add_land ;
  TIDX start_tile ;
  U16 i ; /*loop control*/


  /*PRT_VAR((unsigned)size,u)*/
  /*PRT_VAR((unsigned)min_size,u)*/
  /*PRT_VAR((unsigned)world_flags_mask,02x)*/
  /*PRT_VAR((unsigned)big_island_dist,u)*/
  /*PRT_VAR((unsigned)small_island_dist,u)*/
  /*PRT_VAR((unsigned)bp_desert,u)*/
  /*PRT_VAR((unsigned)bp_prairie,u)*/
  /*PRT_VAR((unsigned)bp_tundra,u)*/

  WORLD_FLAGS_ALLOC( ADD_LAND_HERE_ALL | TAGGED )
  clear_flags(       ADD_LAND_HERE_ALL | TAGGED ) ;

     /*start the new island where no "NO_*_ISLAND HERE" flags are set*/
  start_tile = draw_on_flag_pattern_match( world_flags_mask, 0x00 ) ;
  if (start_tile == INVALID_TIDX) {
    fprintf( stderr, "could not find a suitable start point for an island\n" ) ;
    log_with_timestamp( "could not find a suitable start point for an island\n" ) ;
    map_gen_exit() ;
  }

  world_flags [ start_tile ] |= ADD_LAND_HERE0 ;

  for ( i = 1 ; i <= size ; i++ ) {
    /*PRT_VAR((unsigned)i,u)*/
    if (world_flags_mask == NO_BIG_ISLAND_HERE) { /*prio for big islands only*/
      /*DEB((stderr,"big\n"))*/
      tile_to_add_land = draw_on_flag_pattern_match( /* 7 = 1 1 1*/
                            ADD_LAND_HERE_ALL, ADD_LAND_HERE_ALL ) ;
      if (tile_to_add_land == 0xffffffff) { /*no tile found!*/
        tile_to_add_land = draw_on_flag_pattern_match( /* 6 = 1 1 0*/
          ADD_LAND_HERE2 | ADD_LAND_HERE1, ADD_LAND_HERE2 | ADD_LAND_HERE1 ) ;
      }
      if (tile_to_add_land == 0xffffffff) { /*no tile found!*/
        tile_to_add_land = draw_on_flag_pattern_match( /* 5 = 1 0 1*/
          ADD_LAND_HERE2 | ADD_LAND_HERE0, ADD_LAND_HERE2 | ADD_LAND_HERE0 ) ;
      }
      if (tile_to_add_land == 0xffffffff) { /*no tile found!*/
        tile_to_add_land = draw_on_flag_pattern_match( /* 4 = 1 0 0*/
            ADD_LAND_HERE2, ADD_LAND_HERE2 ) ;
      }
      if (tile_to_add_land == 0xffffffff) { /*no tile found!*/
        tile_to_add_land = draw_on_flag_pattern_match( /* 3 = 0 1 1*/
          ADD_LAND_HERE1 | ADD_LAND_HERE0, ADD_LAND_HERE1 | ADD_LAND_HERE0 ) ;
      }
      if (tile_to_add_land == 0xffffffff) { /*no tile found!*/
        tile_to_add_land = draw_on_flag_pattern_match( /* 2 = 0 1 0*/
            ADD_LAND_HERE1, ADD_LAND_HERE1 ) ;
      }
      if (tile_to_add_land == 0xffffffff) { /*no tile found!*/
        tile_to_add_land = draw_on_flag_pattern_match( /* 1 = 0 0 1*/
            ADD_LAND_HERE0, ADD_LAND_HERE0 ) ;
      }
    } /*endif big island*/

    else { /*no prio for small islands*/
      /*DEB((stderr,"small\n"))*/
      tile_to_add_land = draw_on_flag_pattern_match( /* 1 = 0 0 1*/
          ADD_LAND_HERE0, ADD_LAND_HERE0 ) ;
    }

          /*big & small islands*/
    if (tile_to_add_land == 0xffffffff) { /*no tile found!*/
      if (i >= min_size) {
        fprintf( log_fp, "premature end of island generation\n" ) ;
        break ; /*minimum island size is reached, so it's ok*/
      }
      fprintf( stderr, "cannot find tile to add to island\n" ) ;
       printf(         "cannot find tile to add to island\n" ) ;
      map_gen_exit() ;
    }
    /*PRT_VAR((unsigned)tile_to_add_land,u)*/
    add_tile_to_island( tile_to_add_land, world_flags_mask,
             bp_desert, bp_prairie, bp_tundra ) ;
  } /*end for loop 1..size*/

     /*set NO_XXXX_ISLAND_HERE flags*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles -1 ; tile_index++ ) {
    if ((world_flags [ tile_index ] & NO_BIG_ISLAND_HERE) == 0) {
      if (dist_tile_to_group( tile_index, TAGGED, TAGGED ) < big_island_dist ){
        world_flags [tile_index] |= NO_BIG_ISLAND_HERE ;
      }
    }
    if ((world_flags [ tile_index ] & NO_SMALL_ISLAND_HERE) == 0) {
      if (dist_tile_to_group( tile_index, TAGGED,TAGGED) < small_island_dist ){
        world_flags [tile_index] |= NO_SMALL_ISLAND_HERE ;
      }
    }
  }
  WORLD_FLAGS_FREE( ADD_LAND_HERE_ALL | TAGGED )
  return start_tile ;
}

/*-------------------->   add_tile_to_island   <----------------- 2017-Feb-19
This function adds one tile at a given index to an island.
All data structures are updated so that another tile can be added.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- tile_index   This tile is added to the island
                  WARNING: Tile_index is re-used for loop control just to
                  save four bytes (not relevant on today's machines)
		- world_flag_mask
		    is NO_BIG_ISLAND_HERE     for big islands
		    is NO_SMALL_ISLAND_HERE for small islands
Return value:	void
Exitcode:	x
---------------------------------------------------------------------------*/
static void add_tile_to_island( TIDX tile_index, U8 world_flags_mask,
                 U32 bp_desert, U32 bp_prairie, U32 bp_tundra )
{
/*THIS_FUNC(add_tile_to_island)*/
  NEIGHBORHOOD NT ;
  NEIGHBORHOOD* NTp ;
  U32* tile_idxp ;
  int i ; /*loop control*/
  U8 prio_counter ;


  world [ tile_index ] = draw_island_tile( tile_index,
                    bp_desert, bp_prairie, bp_tundra ) ;
  world_flags [ tile_index ] &= ~ADD_LAND_HERE_ALL ;
  world_flags [ tile_index ] |=
            (TAGGED | NO_BIG_ISLAND_HERE | NO_SMALL_ISLAND_HERE) ;
  NTp = set_neighborhood_tiles( tile_index, & NT ) ;
  for ( i = 0, tile_idxp = & (NTp->n_idx) ; i < 8 ; i++, tile_idxp++ ) {
    /*PRT_VAR(i,d)*/
    tile_index = *tile_idxp ;
    /*PRT_VAR((int)tile_index,d)*/
    if (tile_index < total_no_of_tiles) { /*i. e. on the map, not north
                                                          or south pole*/
      if (world [ tile_index ] == OCEAN) {
        if ((world_flags [ tile_index ] & world_flags_mask) == 0) {
          if (world_flags_mask == NO_BIG_ISLAND_HERE) { /*prio for big islands*/
            prio_counter = world_flags [ tile_index ] & ADD_LAND_HERE_ALL ;
            if (prio_counter != ADD_LAND_HERE_ALL) { /*not yet 7 reached*/
              prio_counter++ ;

              /*The following code line has a bug.  The counter in*/
              /*"world_flags" is not blanked out before ORing "prio_counter".*/
              /*Effect: The old counter and the incremented counter are ORed.*/
              /*Anyway, the islands look much less boring *with* the bug,*/
              /*so I hereby decide that it is a feature :-)*/

              world_flags [ tile_index ] |= prio_counter ; /*<-Bug!!*/

              /*world_flags [ tile_index ]*/ /*correct, but boring*/
                /*= ((world_flags [ tile_index ]) & ~ADD_LAND_HERE_ALL)*/
                  /*| prio_counter ;*/
/*----------------------------------------------------------------------
Bug effect analysis
   prio_counter:  correct            bug
                    0->1            0->1
                    1->2            1->3
                    2->3           *2->3
                    3->4            3->7
                    4->5           *4->5
                    5->6           *5->7
                    6->7           *6->7
                    7->7            7->7
   The transitions marked with '*' do never occur,
   so the counter sequence is 0->1->3->7
   The bug effect is that tiles with only three land neighbors yield the highest
   priority, leading to a more rugged coastline.
----------------------------------------------------------------------*/
            }
          }
          else { /*no prio control for small islands*/
            world_flags [ tile_index ] |= ADD_LAND_HERE0 ;
          }
        } /*end 'no inhibit flags'*/
      } /*end if tile is OCEAN*/
    } /*end if on map*/
  } /*end neighborhood for loop*/
}

/*-------------------->   draw_island_tile   <------------------- 2016-Jul-16
This function returns one TILE type to be added to a land mass for a given
tile index (TIDX).
The basic probabilities are modified due to other land tiles in city radius.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- tile_index  ... of tile to draw
		- bp_*      basic probabilities for some tile types
		            (depends on scenario)
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
TILE draw_island_tile( TIDX tile_index,
       U32 bp_desert, U32 bp_prairie, U32 bp_tundra )
{
THIS_FUNC(draw_island_tile)
  CITY_TILES CT ;
  NEIGHBORHOOD NT ;
  PROBABILITY_VECTOR pv ;
  CITY_TILES* CTp ;
  NEIGHBORHOOD* NTp ;
  TILE tile ;
  TILE* Tp ;
  int i ; /*loop control*/


     /*set basic probabilities*/
  pv.water = 0 ;
  pv.grassland = 100 ;
  pv.desert = bp_desert ;
  pv.prairie = bp_prairie ;
  pv.tundra = bp_tundra ;
  pv.arctic = 0 ;
  pv.swamp = 100 ;
  pv.forest = 200 ;
  pv.hills = 100 ;
  pv.mountains = 80 ;

  if (found_basic_probability_mountains) {
    pv.mountains = basic_probability_mountains ;
  }
  if (found_basic_probability_grassland) {
    pv.grassland = basic_probability_grassland ;
  }
  /*printf( "DRAW_ISLAND_TILE\n" ) ;*/
  /*print_pv( & pv ) ;*/

     /*modify probability vector in 2 steps:
       1. according to near tiles in city radius
       2. according to neighborhood tiles*/
  CTp = set_city_tiles( tile_index, & CT ) ;
  Tp = & (CTp->t1) ; /*start at first tile in array*/
  for ( i = 0 ; i < 20 ; i++, Tp++ ) {
    tile = (*Tp) & BASIC_TILE_TYPE_MASK ;
    switch (tile) {
    case OCEAN:
      break ;

    case GRASSLAND:
      pv.grassland += 100 ;
      pv.prairie += 50 ;
      pv.swamp += 10 ;
      pv.forest += 50 ;
      pv.hills += 50 ;
      break ;

    case DESERT:
      pv.tundra = 0 ;
      pv.arctic = 0 ;
      break ;

    case PRAIRIE:
      pv.grassland += 50 ;
      pv.prairie += 100 ;
      pv.swamp += 10 ;
      pv.forest += 50 ;
      pv.hills += 50 ;
      break ;

    case TUNDRA:
      pv.desert = 0 ;
      break ;

    case ARCTIC:
      break ;

    case SWAMP:
      pv.swamp += 10 ;
      pv.grassland += 50 ;
      pv.prairie += 50 ;
      pv.swamp += 10 ;
      pv.forest += 50 ;
      pv.hills += 50 ;
      break ;

    case FOREST:
      pv.grassland += 50 ;
      pv.prairie += 50 ;
      pv.swamp += 10 ;
      pv.forest += 100 ;
      pv.hills += 50 ;
      pv.mountains += 20 ;
      break ;

    case HILLS:
      pv.grassland += 50 ;
      pv.prairie += 50 ;
      pv.swamp += 10 ;
      pv.forest += 50 ;
      pv.hills += 100 ;
      pv.mountains += 20 ;
      break ;

    case MOUNTAINS:
      pv.grassland += 50 ;
      pv.prairie += 50 ;
      pv.swamp += 10 ;
      pv.forest += 50 ;
      pv.hills += 100 ;
      pv.mountains += 180 ;
      break ;

    case 0xf: /*NORTHPOLE*/
    case 0xe: /*SOUTHPOLE*/
      break ;

    default:
      fprintf( stderr, "draw_island_tile: illegal tile type (1)\n" ) ;
      map_gen_exit() ;
    }
  }

  NTp = set_neighborhood_tiles( tile_index, & NT ) ;
  Tp = & (NTp->n_tile) ; /*start at first tile in array*/
  for ( i = 0 ; i < 8 ; i++, Tp++ ) {
    tile = (*Tp) & BASIC_TILE_TYPE_MASK ;
    switch (tile) {
    case OCEAN:
      break ;

    case GRASSLAND:
      pv.grassland += 100 ;
      pv.prairie += 50 ;
      pv.swamp += 10 ;
      pv.forest += 50 ;
      pv.hills += 50 ;
      break ;

    case DESERT:
      pv.desert += 300 ;
      pv.tundra = 0 ;
      pv.arctic = 0 ;
      break ;

    case PRAIRIE:
      pv.grassland += 50 ;
      pv.prairie += 100 ;
      pv.swamp += 10 ;
      pv.forest += 50 ;
      pv.hills += 50 ;
      break ;

    case TUNDRA:
      pv.tundra += 80 ;
      pv.desert = 0 ;
      break ;

    case ARCTIC:
      pv.arctic += 200 ;
      pv.desert = 0 ;
      break ;

    case SWAMP:
      pv.grassland += 50 ;
      pv.prairie += 50 ;
      pv.swamp += 10 ;
      pv.forest += 50 ;
      pv.hills += 50 ;
      pv.desert = 0 ;
      break ;

    case FOREST:
      pv.grassland += 50 ;
      pv.prairie += 50 ;
      pv.swamp += 10 ;
      pv.forest += 100 ;
      pv.hills += 50 ;
      pv.mountains += 20 ;
      break ;

    case HILLS:
      pv.grassland += 50 ;
      pv.prairie += 50 ;
      pv.swamp += 10 ;
      pv.forest += 50 ;
      pv.hills += 100 ;
      pv.mountains += 20 ;
      break ;

    case MOUNTAINS:
      pv.grassland += 50 ;
      pv.prairie += 50 ;
      pv.swamp += 10 ;
      pv.forest += 50 ;
      pv.hills += 100 ;
      pv.mountains += 200 ;
      break ;

    case 0xf: /*NORTHPOLE*/
    case 0xe: /*SOUTHPOLE*/
      break ;

    default:
      fprintf( stderr, "draw_island_tile: illegal tile type (2)\n" ) ;
      map_gen_exit() ;
    }
  }

  tile = draw_tile( & pv ) ;
  if (tile == OCEAN) {
    PRT_VAR((unsigned long)pv.water,lu)
    PRT_VAR((unsigned long)pv.grassland,lu)
    PRT_VAR((unsigned long)pv.desert,lu)
    PRT_VAR((unsigned long)pv.prairie,lu)
    PRT_VAR((unsigned long)pv.tundra,lu)
    PRT_VAR((unsigned long)pv.arctic,lu)
    PRT_VAR((unsigned long)pv.swamp,lu)
    PRT_VAR((unsigned long)pv.forest,lu)
    PRT_VAR((unsigned long)pv.hills,lu)
    PRT_VAR((unsigned long)pv.mountains,lu)
    PRT_VAR((unsigned long)tile,08lx)
    PRT_VAR((unsigned long)tile_index,lu)
    fprintf( stderr, "draw island tile: unexp. OCEAN tile\n" ) ;
    map_gen_exit() ;
  }
  return tile ;
}

/*-------------------->   x   <---------------------------------- 2008-Jun-01
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
