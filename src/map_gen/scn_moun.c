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
scn_moun.c

This file generates a C-Evo-map file (Scenario: Mountains).
*****************************************************************************
History: (latest change first)
2017-Mar-27: debugging (segmentation fault)
2016-Aug-17: clean log file implementation
2015-Nov-19: Option: river_visibility
2015-Aug-28: Option: no special start position for human player
2015-Jun-05: new RIVER_PARAMS initialized correctly
2015-Feb-06: reduced special resources to a minimum of three if not enough
             places are available
2012-Sep-25: "add_starting_positions" with STARTPOS_PARAMS
2012-Sep-24: introduced RIVER_PARAMS
2012-Sep-23: removed code without effect (lakes<->rivers)
2012-Jun-09: make use of WORLD_FLAGS_ALLOC
2012-Jun-08: add_rivers() now with parameter
2012-Feb-12: renamed scenario function from "gebirge" to "mountains"
2010-Jun-21: removed call to cevo_lib_init
2010-May-01: Initial version from "map_gen.c"
*****************************************************************************
Global objects:
- void scenario_mountains( void )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

#ifdef NEVER
                            /*should be equivalent to glaciers*/
                            /*Has to be tested yet*/
  CLUSTER_PARAMS clus ;
    clus.method = CLUSTER_METHOD_ADJACENT ;
    clus.no_of_clusters = 1 ;
    clus.area_mask = xxx ;
    clus.area = xxx ;
    clus.area_only_for_start_points = FALSE ;
    /*clus.size = xxx ;*/
    clus.min_size = 1 ;
    clus.tile_func = return_arctic ;
    clus.set_flags = TAGGED ;
    clus.clear_flags = 0x0 ;
    clus.edge_flag = 0x0 ;
    clus.grow_into = TTL_MOUNTAINS ;
    clus.task_str = "add glaciers" ;
    clus.flag1 = 0x0 ;
    clus.dist1 = 0 ;
    clus.flag2 = 0x0 ;
    clus.dist2 = 0 ;
#endif /*NEVER*/


/*--  include files  ------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include <misc.h>
#include <falloc.h>

/*#define DEBUG*/
#include <debug.h>

#include "map_gen.h"
#include "read_ini.h"


/*--  constants  ----------------------------------------------------------*/



/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

static void add_glacier_tile( TIDX index ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/



/*--  internal variables  -------------------------------------------------*/



/*-------------------->   scenario_mountains   <----------------- 2017-Mar-27
This function generates the 'Mountains' scenario map.
Description see "map_gen.ini".
-----------------------------------------------------------------------------
Used functions:
Parameters:	--
Return value:	void
Exitcode:	x
---------------------------------------------------------------------------*/
void scenario_mountains( void )
{
/*THIS_FUNC(scenario_mountains)*/
char* _this_func = "scenario_mountains" ; /*for WORLD_FLAGS*/
  RIVER_PARAMS riv ;
  STARTPOS_PARAMS sp ;
  TIDX tile_index ;
  TILE tile ;

  U16 no_of_lake_tiles_target ;
  U16 no_of_lake_tiles = 0 ;
  U16 lake_size ;
  U16 no_of_glacier_tiles_target ;
  U16 no_of_glacier_tiles ;
  int i ;
  U16 dice ; /*scratchpad for random number*/

  U8 no_of_glaciers ;
  U8 count ;


     /*check some params*/
  if (found_human_start_pos) {
    if (human_start_pos != 0) {
      fprintf( log_fp,
          "human_start_pos=%u.  In this scenario, only 0 is allowed.\n",
          (unsigned)human_start_pos ) ;
       printf(
          "human_start_pos=%u.  In this scenario, only 0 is allowed.\n",
          (unsigned)human_start_pos ) ;
      map_gen_exit() ;
    }
  }


     /*fill map with MOUNTAINS*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    world [ tile_index ] = MOUNTAINS ;
  }

  WORLD_FLAGS_ALLOC( IS_SHORE )

     /*add lakes*/
  no_of_lake_tiles_target = (lake_percentage * total_no_of_tiles) / 100 ;
  /*PRT_VAR(no_of_lake_tiles_target,u)*/
  fprintf( log_fp, "no_of_lake_tiles_target = %u\n", no_of_lake_tiles_target ) ;

  while (no_of_lake_tiles < no_of_lake_tiles_target) {
    lake_size = random_draw_range( min_lake_size, max_lake_size ) ;
    no_of_lake_tiles += lake_size ;
    add_lake( lake_size, lake_dist, IS_SHORE, SMOOTH ) ;
  } /*end of lake generation*/
  WORLD_FLAGS_FREE( IS_SHORE )


     /*eliminate one-tile-islands, except for 'arctic'*/
  eliminate_one_tile_islands() ;


     /*add rivers*/
  riv.rain_func = rain_on_land_uniform ;
  riv.visibility = 100 ;
  if (found_river_visibility) {
    riv.visibility = river_visibility ;
  }
  riv.change_tiles = TRUE ;
  riv.change_mountains = FALSE ;
  add_rivers( & riv ) ;
  fprintf( log_fp, "rivers added\n" ) ;

     /*add fertile tiles on shores*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_LAND )) {
      if (count_neighborhood_tiles( tile_index, TTL_WATER ) > 0) {
           /*is shore tile*/
        dice = random_draw_range( 1, 100 ) ;
           /* 75% unveraendert, 11% Gras, 11% Wald, 1% Sumpf, 2% Huegel*/
        tile = (world [tile_index]) & ~BASIC_TILE_TYPE_MASK ;
        if (dice < 12) {
          world [tile_index] = tile | GRASSLAND ;
        }
        else if (dice < 23) {
          world [tile_index] = tile | FOREST ;
        }
        else if (dice < 24) {
          world [tile_index] = tile | SWAMP ;
        }
        else if (dice < 26) {
          world [tile_index] = tile | HILLS ;
        }
      }
    }
  }
     /*add more fertile tiles beside shores and rivers*/
  for ( i = 0 ; i < 2 ; i++ ) { /*perform 3 times*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      if (is_in_TTL( tile_index, TTL_MOUNTAINS )) {
        count = count_adjacent_tiles( tile_index,
           TTL_GRASSLAND | TTL_FOREST | TTL_SWAMP | TTL_HILLS ) ;
        if (count > 0) {
             /*94% unveraendert, 3% Wald, 2% Huegel, 1% Gras*/
          dice = random_draw_range( 1, 100 ) ;
          tile = (world [ tile_index ]) & ~BASIC_TILE_TYPE_MASK ;
          if (dice < 4) {
            world [tile_index] = tile | FOREST ;
          }
          else if (dice < 6) {
            world [tile_index] = tile | HILLS ;
          }
          else if (dice < 7) {
            world [tile_index] = tile | GRASSLAND ;
          }
        }
      }
    }
  }
  fprintf( log_fp, "more fertile tiles added\n" ) ;

     /*add glaciers*/
  WORLD_FLAGS_ALLOC( TAGGED | ADD_HERE0 )
  clear_flags(       TAGGED | ADD_HERE0 ) ;
  no_of_glacier_tiles = 0 ;
     /*tag all 'deep in the mountains' tiles*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (count_city_tiles( tile_index, TTL_MOUNTAINS ) == 20) {
      world_flags [tile_index] |= TAGGED ;
    }
  }
     /*initiate glacier starting points*/
  no_of_glaciers = random_draw_range( 1, max_no_of_glaciers ) ;
  for ( i = 0 ; i < no_of_glaciers ; i++ ) {
    tile_index = draw_on_flag_pattern_match( TAGGED, TAGGED ) ;
    if (tile_index == 0xffffffff) {
      fprintf( log_fp, "not enough glacier starting points\n" ) ;
      break ; /*go on anyway*/
    }
    add_glacier_tile( tile_index ) ;
    no_of_glacier_tiles++ ;
  }
     /*add more glacier tiles*/
  no_of_glacier_tiles_target = (glacier_percentage * total_no_of_tiles) / 100 ;
  /*PRT_VAR(no_of_glacier_tiles_target,u)*/
  fprintf( log_fp, "no_of_glacier_tiles_target = %u\n",
                    no_of_glacier_tiles_target ) ;
  while (no_of_glacier_tiles < no_of_glacier_tiles_target) {
    tile_index = draw_on_flag_pattern_match( ADD_HERE0, ADD_HERE0 ) ;
    /*printf("scn: tile_index=0x%08x\n", (unsigned)tile_index) ;*/
    if (tile_index >= total_no_of_tiles) { /*=INVALID*/
      fprintf( log_fp, "premature end of glacier generation\n" ) ;
      break ; /*go on anyway*/
    }
    add_glacier_tile( tile_index ) ;
    no_of_glacier_tiles++ ;
  }
  /*don't forget to remove ivory tiles later !!!*/
  WORLD_FLAGS_FREE( TAGGED | ADD_HERE0 )
  fprintf( log_fp, "glaciers added\n" ) ;

     /*convert ocean to coast*/
  set_ocean_and_coast() ;

     /*add bonus resources & plains*/
  set_bonus_resources() ;

     /*re-convert ivory to mountains*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if ((world [tile_index]) == (ARCTIC | BONUS_RESOURCE_1_MASK)) {
      /*ASSERT(count_city_tiles( tile_index, TTL_WATER ) == 0)*/
      world [ tile_index ] = MOUNTAINS ;
    }
  }
  set_ocean_and_coast() ;
  set_bonus_resources() ;

     /*add special resources and dead lands*/
     /*... somewhere deep in the mountains*/
  WORLD_FLAGS_ALLOC( TAGGED | ADD_HERE0 )
  for ( i = 0 ; i < 12 ; i++ ) {
    clear_flags(     TAGGED | ADD_HERE0 ) ;
       /*tag all 'deep in the mountains' tiles*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      if (count_city_tiles( tile_index, TTL_MOUNTAINS | TTL_ARCTIC ) == 20) {
        world_flags [tile_index] |= TAGGED ;
      }
    }
    tile_index = draw_on_flag_pattern_match( TAGGED, TAGGED ) ;
    if (tile_index == 0xffffffff) {
      fprintf( log_fp, "not enough places for special resources\n" ) ;
      if (i > 2) {
        break ; /*go on anyway, 3 specials are already placed*/
      }
      printf(          "not enough places for special resources\n" ) ;
      map_gen_exit() ;
    }
    tag_city_tiles( tile_index, ADD_HERE0 ) ;
    world       [tile_index] = HILLS ;
    world_flags [tile_index] &= ~ADD_HERE0 ;

    tile_index = draw_on_flag_pattern_match( ADD_HERE0, ADD_HERE0 ) ;
    world       [tile_index] = HILLS ;
    world_flags [tile_index] &= ~ADD_HERE0 ;

    tile_index = draw_on_flag_pattern_match( ADD_HERE0, ADD_HERE0 ) ;
    world [tile_index] |= special_resource_tab [i] ;
  }
  WORLD_FLAGS_FREE( TAGGED | ADD_HERE0 )
  fprintf(log_fp, "special resources added\n" ) ;


     /*add computer starting positions*/
  WORLD_FLAGS_ALLOC( DONT_SETTLE_DOWN | DONT_USE )
  clear_flags(  (U8)(DONT_SETTLE_DOWN | DONT_USE) ) ;
  sp.min_rating = startpos_min_rating ;
  if (found_human_start_pos) { /*always 0 in this scenario*/
    sp.number = comp_opponents +1 ;
    sp.make_worst_pos_human = TRUE ;
  }
  else {
    sp.number = comp_opponents ;
    sp.make_worst_pos_human = FALSE ;
  }
  sp.city_tag = 0x0 ;
  sp.city_dist = starting_pos_dist ;
  sp.dont_settle_down = DONT_SETTLE_DOWN ;
  sp.exit_if_failing = TRUE ;
  sp.try_irrigation = FALSE ;
  add_starting_positions( & sp ) ;
  fprintf(log_fp, "computer starting positions added\n" ) ;

     /*add human starting position*/
     /*... in a faraway valley.  2 GRASSLANDS, 2 FORESTS*/
  if ( ! found_human_start_pos) { /*always 0 in this scenario*/
    WORLD_FLAGS_ALLOC( TAGGED | ADD_HERE0 )
    clear_flags(       TAGGED | ADD_HERE0 ) ;
       /*tag all 'deep in the mountains' tiles*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      if (count_city_tiles( tile_index, TTL_MOUNTAINS | TTL_ARCTIC ) == 20) {
        world_flags [tile_index] |= TAGGED ;
      }
    }
    tile_index = draw_on_flag_pattern_match(
                                TAGGED | DONT_SETTLE_DOWN, TAGGED ) ;
    if (tile_index == INVALID_TIDX) {
      fprintf( log_fp, "no suitable place for human starting position\n" ) ;
       printf(         "no suitable place for human starting position\n" ) ;
      map_gen_exit() ;
    }
    tag_neighborhood_tiles( tile_index, ADD_HERE0 ) ;
    world       [tile_index] = GRASSLAND | HUMAN_STARTPOS ;
       /*CAUTION: Do not call "set_bonus_resources" behind this point*/
       /*         or the four-food GRASSLAND might be converted into*/
       /*         a three-food GRASSLAND (plains)*/
       /*THIS IS A BUG!  IT VIOLATES THE 'STANDARD BONUS DISTRIBUTION' RULE!*/
    world_flags [tile_index] &= ~ADD_HERE0 ;

    tile_index = draw_on_flag_pattern_match( ADD_HERE0, ADD_HERE0 ) ;
    world       [tile_index] = GRASSLAND ;
    world_flags [tile_index] &= ~ADD_HERE0 ;

    tile_index = draw_on_flag_pattern_match( ADD_HERE0, ADD_HERE0 ) ;
    world       [tile_index] = FOREST ;
    world_flags [tile_index] &= ~ADD_HERE0 ;

    tile_index = draw_on_flag_pattern_match( ADD_HERE0, ADD_HERE0 ) ;
    world       [tile_index] = FOREST ;
    /*world_flags [tile_index] &= ~ADD_HERE0 ;*/

    WORLD_FLAGS_FREE( TAGGED | ADD_HERE0 )
  }
  WORLD_FLAGS_FREE( DONT_SETTLE_DOWN | DONT_USE )
}

/*-------------------->   add_glacier_tile   <------------------- 2017-Mar-27
This function adds one glacier tile.
-----------------------------------------------------------------------------
Used functions:
Globals/Internals: world_flags: TAGGED, ADD_HERE0
Parameters:	- index    ... of tile to be added
Return value:	void
Exitcode:	x
---------------------------------------------------------------------------*/
static void add_glacier_tile( TIDX index )
{
THIS_FUNC(add_glacier_tile)
  NEIGHBORHOOD N ;
  NEIGHBORHOOD* Np ;
  TIDX idx2 ;
  U8 i ; /*loop control*/

  /*printf("add_glacier_tile: index=0x%08x\n", (unsigned)index) ;*/
  ASSERT(index < total_no_of_tiles)
  ASSERT(world [index] == MOUNTAINS) /*... and nothing else*/
  world [index] = ARCTIC ;
  world_flags [index] &= ~(TAGGED | ADD_HERE0) ;
  Np = set_neighborhood_tiles( index, & N ) ;
  for ( i = 1 ; i < 8 ; i += 2 ) { /*scan adjacent tiles*/
    idx2 = ( & (Np->n_idx)) [i] ;
    if (idx2 < total_no_of_tiles) {
      if (world [idx2] == MOUNTAINS) { /*grow into mountains only*/
        world_flags [idx2] |= ADD_HERE0 ;
      }
    }
  }
}

/*-------------------->   x   <---------------------------------- 2012-Jun-09
This function x
-----------------------------------------------------------------------------
Used functions:
Globals/Internals:
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
