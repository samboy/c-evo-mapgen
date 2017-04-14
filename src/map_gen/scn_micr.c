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
scn_micr.c

This file generates a C-Evo-map file (Scenario: Micronesia sea).
*****************************************************************************
History: (latest change first)
2016-Aug-17: clean log file implementation
2016-Jul-16: adoptions for gcc
2012-Sep-25: "add_starting_positions" with STARTPOS_PARAMS
2012-Mar-13: - Debugging (2 special resources on same tile)
             - Human player on worst starting position
2012-Feb-12: renamed scenario to "_micronesia"
2010-Jun-21: removed call to cevo_lib_init
2010-May-22: Default values for "basic_probability_*", so these vars
             are no longer needed in "map_gen.ini"
2010-Apr-18..22: Initial version from file "map_gen.c"
*****************************************************************************
Global objects:
- 
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include "map_gen.h"
#include "read_ini.h"

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/



/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/



/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/



/*--  internal variables  -------------------------------------------------*/



/*-------------------->   scenario_micronesia   <---------------- 2016-Aug-17
This function generates the 'Micronesia' scenario map.
Description see "map_gen.ini".
-----------------------------------------------------------------------------
Used functions:
Parameters:	--
Return value:	void
Exitcode:	x
---------------------------------------------------------------------------*/
void scenario_micronesia( void )
{
THIS_FUNC(scenario_micronesia)
  STARTPOS_PARAMS sp ;
  U32 no_of_land_tiles, no_of_land_tiles_limit ;
  U32 tile_index ;
  U16 island_size ;
  U8 i ;
  U8 flags ;
  U16 rating, best_rating ;
  U32 best_index ;


     /*check params from "map_gen.ini"*/
  if ( ! found_basic_probability_desert) {
    basic_probability_desert = 0 ;
  }
  if ( ! found_basic_probability_prairie) {
    basic_probability_prairie = 0 ;
  }
  if ( ! found_basic_probability_tundra) {
    basic_probability_tundra = 0 ;
  }

  no_of_land_tiles_limit = (land_percentage * total_no_of_tiles) / 100 ;
  /*PRT_VAR((unsigned long)no_of_land_tiles_limit,lu)*/
  fprintf( log_fp, "no_of_land_tiles_limit = %lu\n",
     (unsigned long)no_of_land_tiles_limit ) ;
  no_of_land_tiles = 0 ;

     /*fill map with OCEAN */
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    world [ tile_index ] = OCEAN ;
  }

     /*set zones for no small islands (near poles)*/
  for ( tile_index = 0 ; tile_index < LX2 ; tile_index++ ) {
    world_flags [ tile_index ] = NO_SMALL_ISLAND_HERE ;
    world_flags [ total_no_of_tiles -1 - tile_index ] = NO_SMALL_ISLAND_HERE ;
  }

     /*place the small islands*/
  while (no_of_land_tiles < no_of_land_tiles_limit) {
    island_size = random_draw_range( small_island_min_size,
                                     small_island_max_size ) ;
    no_of_land_tiles += island_size ;
    /*PRT_VAR(no_of_land_tiles,lu)*/
    add_island( island_size, small_island_min_size, NO_SMALL_ISLAND_HERE,
        big_island_dist, small_island_dist,
        basic_probability_desert,
        basic_probability_prairie,
        basic_probability_tundra ) ;
  }

     /*eliminate one-tile-islands, except for 'arctic'*/
  eliminate_one_tile_islands() ;

     /*add rivers*/

     /*convert ocean to coast*/
  set_ocean_and_coast() ;

     /*add bonus resources & plains*/
  set_bonus_resources() ;


     /*add starting positions*/
  clear_flags( 0xff ) ; /*clear all world_flags*/
  sp.min_rating = startpos_min_rating ;
  sp.number = comp_opponents +1 ; /*no special pos for human*/
  sp.city_tag = DONT_USE ;
  sp.city_dist = starting_pos_dist ;
  sp.dont_settle_down = DONT_SETTLE_DOWN ;
  sp.exit_if_failing = TRUE ;
  sp.try_irrigation = FALSE ;
  sp.make_worst_pos_human = FALSE ;
  add_starting_positions( & sp ) ;

      /*Choose the worst of all starting positions*/
      /*and make it the human player's position*/
      /*"add_starting_positions" writes to slots 1..comp_opponents+1*/
  tile_index = start_pos_idx [comp_opponents +1] ; /*the last is the worst*/
  fprintf( log_fp, "changing index %lu (slot %u) to human starting position\n",
           (unsigned long)tile_index, (unsigned)(comp_opponents +1) ) ;
  /*ASSERT(world [tile_index] & STARTPOS_MASK == NORMAL_STARTPOS)*/
      /*above line is not handled correctly by prepocessor!?!*/
  if ((world [tile_index] & STARTPOS_MASK) != NORMAL_STARTPOS) {
    fprintf( log_fp, "scenario_micronesia: assert failed (changing startpos)\n" ) ;
    exit( EXITCODE_UNEXPECTED_ERR ) ;
  }
  world [tile_index] =
               (world [tile_index] & ~NORMAL_STARTPOS) | HUMAN_STARTPOS ;

     /*add special resources and dead lands*/
     /*All special resources are placed outside starting positions*/
  for ( i = 0 ; i < 12 ; i++ ) {
    best_rating = 0 ;
    best_index = 0 ;
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      flags = world_flags [ tile_index ] ;
      if ( ! (flags & DONT_USE)) { /*no city here*/
        /*rating = rateXXXX_start_pos( tile_index ) ;*/
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

    if (best_rating == 0) {
      fprintf( log_fp, "Can place only %u special resources\n", (unsigned)i ) ;
      if (i > 2) { /*essential resources are already placed*/
        /*continue ;*/
        break ;
      }
      fprintf( log_fp, "Can place only %u special resources\n", (unsigned)i ) ;
       printf(         "Can place only %u special resources\n", (unsigned)i ) ;
      map_gen_exit() ;
    }
    set_special_resource( best_index, special_resource_tab [i] ) ;
    tag_city_tiles( best_index, DONT_USE ) ;
  }
}

/*-------------------->   x   <---------------------------------- 2010-Apr-22
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
