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
scn_navi.c

This file generates a C-Evo-map file (Scenario: Navigation required).
*****************************************************************************
History: (latest change first)
2017-Feb-18..19: debugging (segmentation fault)
2016-Aug-09: clean log file implementation
2016-Jul-10:temporarily removed "find_small_areas" for release 1.0
2013-Apr-14..Jun-14: added "find_small_areas"
2012-Feb-12: removed "_inseln" from scenario function name
2010-Jun-21: removed call to cevo_lib_init
2010-May-14: Default values for "basic_probability_*", so these vars
             are no longer needed in "map_gen.ini"
2009-Jan-11: Implementation of ini file
2008-Jul-13: make use of new lib function "best_starting_pos"
2008-Jun-01: Derived from "map_gen.c"
*****************************************************************************
Global objects:
- void scenario_nav_required( void )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include <stdio.h>

/*#define DEBUG*/
#include <debug.h>

#include "map_gen.h"
#include "read_ini.h"


/*--  constants  ----------------------------------------------------------*/



/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

/*void find_small_areas( U8 area_mask, U8 area_flags,*/
                        /*U8 tag_mask, U8 untag_mask,*/
                        /*U32 min_size, CLUSTER_METHOD method,*/
                        /*U8 temp_flag, U8 processed ) ;*/


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/



/*--  internal variables  -------------------------------------------------*/

static TIDX island_start_tile [20] ;
/*static U32 start_pos_mask [20] ;*/


/*-------------------->   scenario_nav_required   <-------------- 2016-Aug-09
Scenario description see "map_gen.ini"
-----------------------------------------------------------------------------
Used functions:
Globals:   x
Internals: x
Parameters:     --
Return value:   --
Exitcode:       x
---------------------------------------------------------------------------*/
void scenario_nav_required( void )
{
/*THIS_FUNC(scenario_nav_required)*/
char* _this_func = "scenario_nav_required" ; /*for WORLD_FLAGS*/
  TIDX tile_index, best_index ;
  U32 temp ;
  int i ; /*loop control*/

  U16 no_of_comp_islands ;
  U16 no_of_small_islands ;
  U16 island_size ;


     /*plausibility checks for params*/
  if (min_no_of_comp_islands < comp_opponents) {
    fprintf( stderr,
"Parameter 'min_no_of_comp_islands' must be greater or equal"
" to parameter 'comp_opponents'\n"
"Your value for 'min_no_of_comp_islands': %u\n"
"Your value for 'comp_opponents': %u\n",
                    (unsigned)min_no_of_comp_islands,
                    (unsigned)comp_opponents
           ) ;
     printf(
"Parameter 'min_no_of_comp_islands' must be greater or equal"
" to parameter 'comp_opponents'\n"
"Your value for 'min_no_of_comp_islands': %u\n"
"Your value for 'comp_opponents': %u\n",
                    (unsigned)min_no_of_comp_islands,
                    (unsigned)comp_opponents
           ) ;
    map_gen_exit() ;
  }

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

  no_of_comp_islands
      = random_draw_range( min_no_of_comp_islands, max_no_of_comp_islands ) ;
  /*PRT_VAR(no_of_comp_islands,u)*/
  fprintf( log_fp, "scenario_nav_required: no_of_comp_islands = %u\n",
                                           no_of_comp_islands ) ;
  no_of_small_islands
      = random_draw_range( min_no_of_small_islands, max_no_of_small_islands ) ;
  /*PRT_VAR(no_of_small_islands,u)*/
  fprintf( log_fp, "scenario_nav_required: no_of_small_islands = %u\n",
                                           no_of_small_islands ) ;


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
     /*to avoid land connection via poles*/
     /*This feature was needed for old C-Evo version;*/
     /*newer version no longer have the 'hard-coded' ARCTIC tiles.*/
  WORLD_FLAGS_ALLOC( NO_BIG_ISLAND_HERE )
  for ( tile_index = 0 ; tile_index < LX3 ; tile_index++ ) {
    world_flags [ tile_index ] = NO_BIG_ISLAND_HERE ;
    world_flags [ total_no_of_tiles -1 - tile_index ] = NO_BIG_ISLAND_HERE ;
  }
  /*PRT_VAR((unsigned)(world_flags [299]),02x)*/
  /*PRT_VAR((unsigned)(world_flags [300]),02x)*/

     /*island for human player*/
  island_size = random_draw_range( human_island_min_size,
                                   human_island_max_size ) ;
  /*PRT_VAR(island_size,u)*/
  fprintf( log_fp, "scenario_nav_required: island_size = %u\n", island_size ) ;
  island_start_tile [0] = add_island( island_size, human_island_min_size,
           NO_BIG_ISLAND_HERE,   big_island_dist, small_island_dist,
           basic_probability_desert,
           basic_probability_prairie,
           basic_probability_tundra ) ;

     /*islands for computer opponents*/
  WORLD_FLAGS_ALLOC( TEMP_TAG | PREV_TAGGED )
  for ( i = 0 ; i < no_of_comp_islands ; i++ ) {
    /*find_small_areas( NO_BIG_ISLAND_HERE, 0x0,*/
                      /*NO_BIG_ISLAND_HERE, 0x0,*/
                      /*comp_island_min_size, CLUSTER_METHOD_NEIGHBORHOOD,*/
                      /*TEMP_TAG, PREV_TAGGED ) ;*/
    fprintf( log_fp, "Adding big island %d\n", i) ;
    island_size = random_draw_range( comp_island_min_size,
                                     comp_island_max_size ) ;
    temp = add_island( island_size, comp_island_min_size, NO_BIG_ISLAND_HERE,
           big_island_dist, small_island_dist,
           basic_probability_desert,
           basic_probability_prairie,
           basic_probability_tundra ) ;
    if (i < comp_opponents) {
      island_start_tile [i+1] = temp ;
    }
    /*write_map_to_file( "test1.cevo map" ) ;*/
  }
  WORLD_FLAGS_FREE( TEMP_TAG | PREV_TAGGED )

     /*small islands*/
  /*PRT_VAR((unsigned)no_of_small_islands,u)*/
  for ( i = 0 ; i < no_of_small_islands ; i++ ) {
    /*PRT_VAR(i,d)*/
    fprintf( log_fp, "Adding small island %d\n", i) ;
    island_size = random_draw_range(
                        small_island_min_size, small_island_max_size ) ;
    /*PRT_VAR((unsigned)island_size,u)*/
    add_island( island_size, small_island_min_size, NO_SMALL_ISLAND_HERE,
           big_island_dist, small_island_dist,
           basic_probability_desert,
           basic_probability_prairie,
           basic_probability_tundra ) ;
  }
  WORLD_FLAGS_FREE( NO_BIG_ISLAND_HERE )


     /*eliminate one-tile-islands, except for 'arctic'*/
  /*write_map_to_file( "one_tile.cevo map" ) ;*/
  eliminate_one_tile_islands() ;
  /*write_map_to_file( "one_tile2.cevo map" ) ;*/

     /*add rivers*/

     /*convert ocean to coast*/
  set_ocean_and_coast() ;

     /*add bonus resources & plains*/
  set_bonus_resources() ;

     /*add starting positions*/
     /*add special resources and dead lands*/
  for ( i = 0 ; i <= comp_opponents ; i++ ) {
    /*PRT_VAR((int)i,d)*/
    clear_flags( 0xff ) ; /*clear all world_flags*/
    world_flags [ island_start_tile [i]] |= TAGGED ;
    tag_whole_land() ;
    /*DEB((stderr,"whole island tagged\n")) ;*/

    best_index = best_starting_pos() ;
    world [ best_index ] |= start_pos_mask [i] ;
    if (i <= 2) {
      set_special_resource( best_index, special_resource_tab [i] ) ;
    }
  }
}

#ifdef BUGGY
/*-------------------->   find_small_areas   <------------------ 2013-Jun-14
This function works on all tiles which match a given flag pattern.
It identifies all contiguous areas on the map which are <= min_size
and tags/untags those tiles.
-----------------------------------------------------------------------------
Used functions:
Globals:   total_no_of_tiles
Internals: --
Parameters:     - area_mask, area_flags  define area(s) on the map
                                         to operate on
                - min_size   further specifies the areas: operate only
                             those tiles who belong to a contiguous area
                             less than or equal min_size
                - tag_mask   tag all specified tiles with tag_mask
                - untag_mask untag all specified tiles with untag_mask
                - method     can be CLUSTER_METHOD_NEIGHBORHOOD
                             or CLUSTER_METHOD_ADJACENT
                             (defines 'contiguous')
                - temp_flag,   internal use, tell "find_small_areas" which
                  processed   flags it may use
            ALL  FLAGS  MUST  BE  ALLOCATED  OUTSIDE  THIS  FUNCTION
Return value:   x
Exitcode:       x
---------------------------------------------------------------------------*/
void find_small_areas( U8 area_mask, U8 area_flags,
                        U8 tag_mask, U8 untag_mask,
                        U32 min_size, CLUSTER_METHOD method,
                        U8 temp_flag, U8 processed )
{
THIS_FUNC(find_small_areas)
  TIDX tidx ; /*loop control*/
  U32 area_size ;
  U32 readback ; /*assert size*/


  PRT_VAR(area_mask,02x)
  ASSERT(method == CLUSTER_METHOD_NEIGHBORHOOD) /*~ADJACENT not yet impl.*/

     /*shortcuts*/
  U8 unprocessed_match = area_mask | processed ;
  U8 set_flags =   tag_mask | processed ;
  U8 clr_flags = untag_mask | temp_flag ;


  clear_flags( temp_flag | processed ) ;
  for ( tidx = 0 ; tidx < total_no_of_tiles ; tidx++ ) {
    PRT_VAR((unsigned long)tidx,lu)
    if ((world_flags [tidx] & unprocessed_match) == area_flags) {
      world_flags [tidx] |= temp_flag ;
      area_size = 1 + tag_whole_area(
                       temp_flag | area_mask, area_flags,
                       TTL_ANY,
                       temp_flag, 0x0 ) ;
      PRT_VAR((unsigned long)area_size,lu)
/*for ( tidx = 0 ; tidx < total_no_of_tiles ; tidx++ ) {*/
  /*if (world_flags [tidx] & temp_flag) {*/
    /*world [tidx] = DESERT ;*/
  /*}*/
/*}*/
/*set_ocean_and_coast() ;*/
/*write_map_to_file( "test1.cevo map" ) ;*/
/*exit(1) ;*/
      if (area_size <= min_size) {
        readback = tag_on_flag_pattern_match( temp_flag, temp_flag,
                                              set_flags, clr_flags ) ;
      }
      else {
        readback = tag_on_flag_pattern_match( temp_flag, temp_flag,
                                              0x0, temp_flag ) ;
      }
      ASSERT(readback == area_size)
    }
  }
  EXIT
}
#endif /*BUGGY*/

/*-------------------->   x   <---------------------------------- 2013-Jun-14
This function x
-----------------------------------------------------------------------------
Used functions:
Globals:   x
Internals: x
Parameters:     - x
                - x
Return value:   x
Exitcode:       x
---------------------------------------------------------------------------*/
/*x*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/
/***************************************************************************/
