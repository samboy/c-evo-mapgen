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
scn_hard.c

This file generates a C-Evo-map file (Scenario: Hard Fight).
*****************************************************************************
History: (latest change first)
2016-Aug-17: clean log file implementation
2016-Jul-16: adoptions for gcc
2016-Jul-11: replaced ASSERT by an error msg and map_gen_exit
2012-Sep-25: "add_starting_positions" with STARTPOS_PARAMS
2012-Sep-07: Additional info to log file
2012-Jun-09: Debugging and new param "water_width"
2010-Jul-27: additional ASSERTs
2010-Jun-21: removed call to cevo_lib_init
2010-Jun-18: - Changed function "mountain_coast" to not use NULL pointer
             when calling "set_neighborhood_tiles"
             - Replaced TAGGED with PREV_TAGGED to avoid conflict with
               "rate_1st_pos"
2010-May-22: Default values for "basic_probability_*", so these vars
             are no longer needed in "map_gen.ini"
2009-Dec-09: Debugging and additional plausibility checks
2009-Dec-07: make use of "map_gen.ini"
2008-Jul-13: Derived from "scn_nav.c"
*****************************************************************************
Global objects:
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*only for testing purposes*/



/*--  include files  ------------------------------------------------------*/

/*#define DEBUG*/
#include <debug.h>

#include "map_gen.h"
#include "read_ini.h"


/*--  constants  ----------------------------------------------------------*/

#define MIN_RATING_FOR_HUMAN_PLAYER  10


/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

static void mountain_coast( TIDX land_tile ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/

/*static TIDX island_start_tile [MAX_NO_OF_COMP_ISLANDS +1] ;*/
static TIDX island_start_tile [ 3 ] ;
/*static U32 start_pos_mask [ 15 ] ;*/ /*defined in map_gen.c*/


/*-------------------->   scenario_hard_fight   <---------------- 2016-Aug-17
Description of scenario 'hard fight' see "map_gen.ini".
-----------------------------------------------------------------------------
Used functions:
Parameters:	--
Return value:	void
Exitcode:	EXITCODE_UNSPECIFIED_ERR (map_gen_exit)
---------------------------------------------------------------------------*/
void scenario_hard_fight( void )
{
/*THIS_FUNC(scenario_hard_fight)*/
char* _this_func = "scenario_hard_fight" ; /*for WORLD_FLAGS*/
  STARTPOS_PARAMS sp ;
  TIDX tile_index, best_index, continent_index ;
  TIDX temp ;
  TILE tile ;
  U16 island_size ;
  int i ;
  unsigned rating, min_rating ;


     /*check params from "map_gen.ini"*/
  if ( ! found_basic_probability_desert) {
    basic_probability_desert = 50 ;
  }
  if ( ! found_basic_probability_prairie) {
    basic_probability_prairie = 100 ;
  }
  if ( ! found_basic_probability_tundra) {
    basic_probability_tundra = 20 ;
  }

     /*must haves*/
  if ( ! found_big_island_dist) {
    fprintf( log_fp, "Parameter missing: \"big_island_dist\"\n" ) ;
    map_gen_exit() ;
  }
  if ( ! found_water_width) {
    fprintf( log_fp, "Parameter missing: \"water_width\"\n" ) ;
    map_gen_exit() ;
  }

     /*plausibility checks*/
  if (water_width < 200) {
    fprintf( log_fp,
        "Wrong parameter value: \"water_width\" = %lu  (min. 200)\n",
        (unsigned long)water_width
           ) ;
    map_gen_exit() ;
  }
  if (big_island_dist < (water_width << 1) + 200) {
    fprintf( log_fp,
      "Minimum value for 'big_island_dist' is (2 * water_width) + 200\n"
           ) ;
    map_gen_exit() ;
  }

     /*fill map with OCEAN and ARCTIC*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (tile_index < LX || tile_index >= total_no_of_tiles - LX) {
      world [ tile_index ] = ARCTIC ;
    }
    else {
      world [ tile_index ] = OCEAN ;
    }
  }

     /*set zones for no big islands (near poles)*/
  for ( tile_index = 0 ; tile_index < LX3 ; tile_index++ ) {
    world_flags [ tile_index ] = NO_BIG_ISLAND_HERE ;
    world_flags [ total_no_of_tiles -1 - tile_index ] = NO_BIG_ISLAND_HERE ;
  }

     /*island for human player*/
  island_size = random_draw_range( human_island_min_size,
                                   human_island_max_size ) ;
  /*PRT_VAR(island_size,u)*/
  fprintf( log_fp, "island_size = %u\n", island_size ) ;
  island_start_tile [0] = add_island( island_size, human_island_min_size,
           NO_BIG_ISLAND_HERE,   big_island_dist, water_width,
           basic_probability_desert,
           basic_probability_prairie,
           basic_probability_tundra ) ;

     /*islands for computer opponents*/
  for ( i = 0 ; i < 2 ; i++ ) {
    island_size = random_draw_range( comp_island_min_size,
                                     comp_island_max_size ) ;
    PRT_VAR(island_size,u)
    temp = add_island( island_size, comp_island_min_size, NO_BIG_ISLAND_HERE,
           big_island_dist, water_width,
           basic_probability_desert,
           basic_probability_prairie,
           basic_probability_tundra ) ;
    if (i < comp_opponents) {
      island_start_tile [i+1] = temp ;
    }
  }

     /*continent*/
     /*convert every tile in a distance greater than SMALL_ISLAND_DISTANCE*/
     /*to the three islands into a LAND tile*/
  while ((tile_index = draw_on_flag_pattern_match( NO_SMALL_ISLAND_HERE, 0
                                                 )) != 0xffffffff) {
    if ( ! is_in_TTL( tile_index, TTL_ARCTIC )) {
      tile = draw_island_tile( tile_index,
                               basic_probability_desert,
                               basic_probability_prairie,
                               basic_probability_tundra ) ;
      world [ tile_index ] = tile ;
      continent_index = tile_index ; /*remember where continent is*/
    }
    world_flags [ tile_index ] |= NO_SMALL_ISLAND_HERE ;
  }

  mountain_coast( continent_index ) ;


     /*eliminate one-tile-islands, except for 'arctic'*/
  eliminate_one_tile_islands() ;

     /*add rivers*/

     /*convert ocean to coast*/
  set_ocean_and_coast() ;

     /*add bonus resources & plains*/
  set_bonus_resources() ;

     /*add starting positions*/
     /*add special resources and dead lands on the three islands*/
  WORLD_FLAGS_ALLOC( TAGGED )
  for ( i = 0 ; i <= 2 ; i++ ) {
    /*PRT_VAR((int)i,d)*/
    /*clear_flags( 0xff ) ;*/
    clear_flags( TAGGED ) ;
    world_flags [ island_start_tile [i]] |= TAGGED ;
       /*TAGGED is used by "rate_1st_pos"*/
    tag_whole_land() ; /*always uses TAGGED, hard-coded*/
    /*DEB((stderr,"whole island tagged\n")) ;*/

    best_index = best_starting_pos() ; /*always uses TAGGED, hard-coded*/
    world [ best_index ] |= start_pos_mask [i] ;
    /*ASSERT(rate_1st_pos( best_index ) >= 41)*/
    rating = rate_1st_pos( best_index ) ;
    min_rating = (i == 0) ? MIN_RATING_FOR_HUMAN_PLAYER : 41 ;
    if (rating < min_rating) {
      fprintf( log_fp,
          "Could not find a suitable start position on one island\n" ) ;
       printf(
          "Could not find a suitable start position on one island\n" ) ;
      fprintf( log_fp, "Location code=%u, rating=%u (%u or more needed)\n",
                         (unsigned)best_index, rating, min_rating ) ;
      map_gen_exit() ;
    }
    if (i <= 2) {
      set_special_resource( best_index, special_resource_tab [i] ) ;
    }
  }
  WORLD_FLAGS_FREE( TAGGED )

  WORLD_FLAGS_ALLOC( TAGGED )
  /*clear_flags( 0xff ) ;*/
  clear_flags( TAGGED ) ;
  world_flags [ island_start_tile [0] ] = TAGGED ;
  world_flags [ island_start_tile [1] ] = TAGGED ;
  world_flags [ island_start_tile [2] ] = TAGGED ;
  tag_whole_land() ; /*tag all tiles of all three islands*/
  sp.min_rating = startpos_min_rating ;
  sp.number = comp_opponents -2 ;
  sp.city_tag = 0x0 ;
  sp.city_dist = starting_pos_dist ;
  sp.dont_settle_down = TAGGED ;
  sp.exit_if_failing = TRUE ;
  sp.try_irrigation = FALSE ;
  sp.make_worst_pos_human = FALSE ;
  add_starting_positions( & sp ) ;
  /*add_starting_positions( (U8)(comp_opponents -2), starting_pos_dist,*/
                          /*0, TAGGED) ;*/
     /*TAGGED means "don't settle down" here*/
  WORLD_FLAGS_FREE( TAGGED )
}

/*-------------------->   mountain_coast   <--------------------- 2010-Jun-18
This function tags the whole land mass to which "land_tile" 'points' to.
It then converts each WATER tile in neighborhood into a MOUNTAIN tile,
unless the land tile is MOUNTAINS or ARCTIC (no coast city possible).
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
static void mountain_coast( TIDX land_tile_idx )
{
THIS_FUNC(mountain_coast)
  TIDX tile_index ;
  /*NEIGHBORHOOD* Np ;*/
  NEIGHBORHOOD N ;
  TIDX* idxp ;
  /*TIDX idx2 ;*/
  U8 i ; /*loop control*/


  /*PRT_VAR(land_tile_idx,lu)*/
  clear_flags( 0xff ) ;
  world_flags [ land_tile_idx ] = TAGGED ;
  tag_whole_land() ;

  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if ((world_flags [ tile_index ]) & TAGGED) { /*belongs to land mass*/
      if ( ! is_in_TTL( tile_index, TTL_MOUNTAINS | TTL_ARCTIC )) {
        set_neighborhood_tiles( tile_index, & N ) ;
        idxp = & (N.n_idx) ;
        for ( i = 0 ; i < 8 ; i++, idxp++ ) {
          if (is_in_TTL( *idxp, TTL_WATER )) {
            world [ *idxp ] = MOUNTAINS ;
          }
        }
      }
    }
  }
}

/*-------------------->   x   <---------------------------------- 2010-Jun-18
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
