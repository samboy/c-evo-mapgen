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
scn_volc.c

This file generates a C-Evo-map file (Scenario: Volcano Islands).
*****************************************************************************
History: (latest change first)
2017-Mar-25: renamed parameters for release 1.1.0
2016-Aug-17: clean log file implementation
2016-Jul-16: adoptions for gcc
2015-Feb-13..14: derived from "scn_plai.c"
*****************************************************************************
Global objects:
- void scenario_volcano_islands( void )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include "map_gen.h"
#include "read_ini.h"

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

/*#define DEFAULT_GRASSLAND_PERCENTAGE 1*/
/*#define DEFAULT_GRASSLAND_MIN_SIZE 1*/
/*#define DEFAULT_GRASSLAND_MAX_SIZE 6*/

/*#define DEFAULT_FOREST_PERCENTAGE 1*/
/*#define DEFAULT_FOREST_MIN_SIZE 2*/
/*#define DEFAULT_FOREST_MAX_SIZE 8*/


/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

/*static TILE ocean( TIDX tidx ) ;*/
/*static TILE grass( TIDX tidx ) ;*/
/*static TILE forest( TIDX tidx ) ;*/
static TILE mountains( TIDX tidx ) ;

/*static BIT add_one_tile( U8 add_here ) ;*/


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/



/*-------------------->   scenario_volcano_islands   <----------- 2017-Mar-25
Scenario description see "map_gen.ini"
-----------------------------------------------------------------------------
Used functions:
Globals:   x
Internals: x
Parameters: --
Return value: void
Exitcode:     x
---------------------------------------------------------------------------*/
void scenario_volcano_islands( void )
{
/*THIS_FUNC(scenario_volcano_islands)*/
char* _this_func = "scenario_volcano_islands" ; /*for WORLD_FLAGS*/
  CLUSTER_PARAMS clus ;
  /*RIVER_PARAMS riv ;*/
  STARTPOS_PARAMS sp ;
  TIDX tile_index ; /*loop control*/
  TIDX best_index ; /*for starting position*/
  TILE tile ;

  U16 no_of_tiles_target ; /*scratch pad for cluster generation*/
  U16 no_of_tiles ;

  U16 dice ;

  U8 i ; /*loop control*/
  U8 count ;
  U8 rating, best_rating ;
  U8 flags ;



     /*check params from "map_gen.ini"*/
  if ( ! found_comp_opponents) {
    comp_opponents = 1 ;
  }

     /*default minimum requirement for startpos*/
  if ( ! found_startpos_min_rating) {
    startpos_min_rating = 8 ;
  }

#ifdef NEVER
  if ( ! found_grassland_percentage) {
    grassland_percentage = DEFAULT_GRASSLAND_PERCENTAGE ;
  }
  if ( ! found_grassland_min_size) {
    grassland_min_size = DEFAULT_GRASSLAND_MIN_SIZE ;
  }
  if ( ! found_grassland_max_size) {
    grassland_max_size = DEFAULT_GRASSLAND_MAX_SIZE ;
  }

  if ( ! found_forest_percentage) {
    forest_percentage = DEFAULT_FOREST_PERCENTAGE ;
  }
  if ( ! found_forest_min_size) {
    forest_min_size = DEFAULT_FOREST_MIN_SIZE ;
  }
  if ( ! found_forest_max_size) {
    forest_max_size = DEFAULT_FOREST_MAX_SIZE ;
  }
#endif /*NEVER*/


     /*fill map with OCEAN */
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    world [ tile_index ] = OCEAN ;
  }

  log_with_timestamp( "placing volcanos" ) ;
  no_of_tiles_target = (island_percentage * total_no_of_tiles) / 100 ;
  no_of_tiles = 0 ;
  /*PRT_VAR(no_of_tiles_target,u)*/
  fprintf( log_fp, "no_of_tiles_target = %u\n", no_of_tiles_target ) ;

  WORLD_FLAGS_ALLOC( TAGGED | NO_LAKE_HERE | IS_SHORE )
  clear_flags(       TAGGED | NO_LAKE_HERE | IS_SHORE ) ;

  /*clus.method = CLUSTER_METHOD_LAKE_SMOOTH ;*/
  /*clus.method = CLUSTER_METHOD_CITY_RADIUS ;*/
  /*clus.method = CLUSTER_METHOD_NEIGHBORHOOD ;*/
  clus.method = CLUSTER_METHOD_ADJACENT ;
  /*clus.method = CLUSTER_METHOD_ISLAND ;*/
  /*clus.no_of_clusters = computer_opponents ;*/
  clus.no_of_clusters = 1 ;
  clus.area_mask = NO_LAKE_HERE ;
  /*clus.area_mask = 0x0 ;*/
  clus.area = 0x0 ;
  clus.area_only_for_start_points = FALSE ;
  /*clus.size = no_of_tiles_target ;*/
  /*clus.min_size = island_min_size ;*/
  clus.min_size = 5 ;
  clus.tile_func = mountains ;
  clus.set_flags = TAGGED ;
  clus.clear_flags = 0x0 ;
  /*clus.edge_flag = IS_SHORE ;*/
  clus.edge_flag = 0x0 ;
  clus.grow_into = TTL_ANY ;
  clus.task_str = "add volcanos" ;
  clus.flag1 = NO_LAKE_HERE ;
  clus.dist1 = island_dist ;
  clus.flag2 = 0x0 ;
  clus.dist2 = 0 ;

  while (no_of_tiles < no_of_tiles_target) {
    clus.size = random_draw_range( island_min_size, island_max_size ) ;
    add_clusters( & clus ) ;
    no_of_tiles += clus.actual_size ;
  } /*end of volcano generation*/
  WORLD_FLAGS_FREE( TAGGED | NO_LAKE_HERE | IS_SHORE )
  log_with_timestamp( "volcanos placed" ) ;


     /*eliminate one-tile-islands, except for 'arctic'*/
     /*Not in this scenario*/
  /*eliminate_one_tile_islands() ;*/


     /*add rivers*/


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



     /*convert ocean to coast*/
  set_ocean_and_coast() ;

     /*add bonus resources & plains*/
  set_bonus_resources() ;


     /*add starting positions*/
  sp.min_rating = startpos_min_rating ;
  sp.number = comp_opponents +1 ;
  sp.city_tag = DONT_USE ;
  sp.city_dist = starting_pos_dist ;
  sp.dont_settle_down = DONT_SETTLE_DOWN ;
  sp.exit_if_failing = TRUE ;
  sp.try_irrigation = TRUE ;
  sp.make_worst_pos_human = TRUE ;
  add_starting_positions( & sp ) ;


     /*add special resources*/
     /*Try to add them at start_pos_idx[1..12]*/


     /*The following is a takeover from Micronesia*/
     /*add special resources and dead lands*/
     /*All special resources are placed outside starting positions*/
  for ( i = 0 ; i < 12 ; i++ ) {
    best_rating = 0 ;
    best_index = 0 ;
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      flags = world_flags [ tile_index ] ;
      if ( ! (flags & DONT_USE)) { /*no city here*/
        rating = rate_1st_pos( tile_index ) ;
        if (rating > best_rating) {
          best_rating = rating ;
          best_index = tile_index ;
        }
      }
    }
    /*PRT_VAR(best_rating,u)*/
    fprintf( log_fp, "best_rating = %u\n", best_rating ) ;
    /*PRT_VAR((unsigned long)best_index,lu)*/
    fprintf( log_fp, "best_index = %lu\n", (unsigned long)best_index ) ;

    if (best_rating == 0) { /*put special resources at 'hard' places*/
      fprintf( log_fp, "Can place only %u special resources\n", (unsigned)i ) ;
      if (i > 2) { /*essential resources are already placed*/
        /*continue ;*/
        break ;
      }
      printf(          "Can place only %u special resources\n", (unsigned)i ) ;
      map_gen_exit() ;
    }
    set_special_resource( best_index, special_resource_tab [i] ) ;
    tag_city_tiles( best_index, DONT_USE ) ;
  }
  fprintf( log_fp, "special resources added\n" ) ;
}

/*-------------------->   mountains   <-------------------------- 2015-Feb-13
This function is for cluster generation.
-> move to aux_clus.c
-----------------------------------------------------------------------------
Used functions:
Globals:
Internals:
Parameters:	- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
static TILE mountains( TIDX tidx )
{
THIS_FUNC(mountains)
  /*PRT_VAR((unsigned)tidx,u)*/
  ASSERT_ALT((world [tidx] & BASIC_TILE_TYPE_MASK) != MOUNTAINS,
                world [tidx] = DESERT ;
                map_gen_exit() ;)
  return (world [tidx] & ~BASIC_TILE_TYPE_MASK) | MOUNTAINS ;
                                      /*preserve river*/
}

/*-------------------->   x   <---------------------------------- 2015-Feb-13
This function x
-----------------------------------------------------------------------------
Used functions:
Globals:
Internals:
Parameters: - x
            - x
Return value: x
Exitcode:     x
---------------------------------------------------------------------------*/
/*x*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/
/***************************************************************************/
