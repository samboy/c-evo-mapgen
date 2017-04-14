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
scn_rive.c

This file generates a C-Evo-map file (Scenario: The Big River).
*****************************************************************************
History: (latest change first)
2016-Aug-17: clean log file implementation
2015-Jun-05: new RIVER_PARAMS initialized correctly
2012-Sep-24: introduced RIVER_PARAMS
2012-Aug-17: Free placement of starting psitions
2012-Aug-16: - Invalid values for "water_width" refused
             - no cities near poles
2012-Aug-05: River width is now controlled by "water_width"
2012-Jun-08: add_rivers() now with parameter
2012-Feb-13..15: derived from "scn_nav.c"
*****************************************************************************
Global objects:
- void scenario_big_river( void )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include "map_gen.h"
#include "read_ini.h"

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

#define DEFAULT_RIVER_WIDTH  1350

    /*number of rows near poles were the river cannot flow*/
#define MIN_LAND_WIDTH  6


/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

static BIT big_river_add_one_tile(
           U8 own_add_flag, U8 own_no_flag, U8 other_no_flag,
           U32 bp_desert, U32 bp_prairie, U32 bp_tundra ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/



/*-------------------->   scenario_big_river   <----------------- 2016-Aug-17
Scenario description see "map_gen.ini"
-----------------------------------------------------------------------------
Used functions:
Parameters:	--
Return value:	--
Exitcode:	x
---------------------------------------------------------------------------*/
void scenario_big_river( void )
{
/*THIS_FUNC(scenario_big_river)*/
char* _this_func = "scenario_big_river" ; /*for WORLD_FLAGS*/
  RIVER_PARAMS riv ;
  TIDX north_anchor, south_anchor ; /*anchor points for land generation*/
  TIDX tile_index ; /*loop control*/
  TIDX best_index ; /*for starting position*/
  U16 effective_width ; /*eff. width for no_north/no_south stripes*/
  U8 i ; /*loop control*/
  BIT ongoing_north = TRUE, ongoing_south = TRUE ; /*while loop control*/



     /*check params from "map_gen.ini"*/
  if ( ! found_basic_probability_desert) {
    basic_probability_desert = 0 ;
  }
  if ( ! found_basic_probability_prairie) {
    basic_probability_prairie = 100 ;
  }
  if ( ! found_basic_probability_tundra) {
    basic_probability_tundra = 0 ;
  }


  if ( ! found_water_width) { /*controls river width here*/
    water_width = DEFAULT_RIVER_WIDTH ;
  }
  /*PRT_VAR((unsigned)water_width,u)*/
  fprintf( log_fp, "water_width = %u\n", (unsigned)water_width ) ;
  if (water_width < 150) {
    fprintf( log_fp,  "scenario 'big river': \"water_width\" must be >= 150\n" ) ;
     printf(          "scenario 'big river': \"water_width\" must be >= 150\n" ) ;
    exit( EXITCODE_WRONG_PARAM ) ;
  }
  effective_width = MIN_LAND_WIDTH + (water_width / 150) ;
  /*PRT_VAR((unsigned)effective_width,u)*/
  fprintf( log_fp, "effective_width = %u\n", (unsigned)effective_width ) ;




     /*fill map with OCEAN */
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    world [ tile_index ] = OCEAN ;
  }

  north_anchor = random_draw_range( 0, LX -1 ) ;
  south_anchor = random_draw_range( total_no_of_tiles - LX,
                                    total_no_of_tiles -1 ) ;
  /*PRT_VAR((unsigned long)north_anchor,lu)*/
  fprintf( log_fp, "north_anchor = %lu\n", (unsigned long)north_anchor ) ;
  /*PRT_VAR((unsigned long)south_anchor,lu)*/
  fprintf( log_fp, "south_anchor = %lu\n", (unsigned long)south_anchor ) ;

  if (water_width > dist_tile_to_tile( north_anchor, south_anchor ) -200) {
    fprintf( log_fp, "scenario 'big river': \"water_width\" too high\n" ) ;
     printf(         "scenario 'big river': \"water_width\" too high\n" ) ;
    map_gen_exit() ;
  }

/*
Flags used to shape the map:
.......x   add south land here
....x...   don't add south land here
...x....   add north land here
x.......   don't add north land here
*/
#define ADD_SOUTH_HERE  0x01
#define NO_SOUTH        0x08
#define ADD_NORTH_HERE  0x10
#define NO_NORTH        0x80

  clear_flags( 0xff ) ;
  WORLD_FLAGS_ALLOC(  ADD_SOUTH_HERE
                    | NO_SOUTH
                    | ADD_NORTH_HERE
                    | NO_NORTH
                   )

     /*set zones for no river (near poles)*/
     /*to avoid land interruption by river touching a pole*/
  for ( tile_index = 0 ; tile_index < effective_width * LX ; tile_index++ ) {
    world_flags [ tile_index ] = NO_SOUTH ;
    world_flags [ total_no_of_tiles -1 - tile_index ] = NO_NORTH ;
  }


  SET_WORLD_FLAGS( north_anchor, ADD_NORTH_HERE )
  SET_WORLD_FLAGS( south_anchor, ADD_SOUTH_HERE )
  while (ongoing_north || ongoing_south) {

    if (ongoing_north) { /*add one tile north*/
      if ( big_river_add_one_tile( ADD_NORTH_HERE, NO_NORTH, NO_SOUTH,
                                   basic_probability_desert,
                                   basic_probability_prairie,
                                   basic_probability_tundra
                                 )
         ) {
        ongoing_north = FALSE ;
      }
    }

    if (ongoing_south) { /*add one tile south*/
      if ( big_river_add_one_tile( ADD_SOUTH_HERE, NO_SOUTH, NO_NORTH,
                                   basic_probability_desert,
                                   basic_probability_prairie,
                                   basic_probability_tundra
                                 )
         ) {
        ongoing_south = FALSE ;
      }
    }
  } /*end while*/

  WORLD_FLAGS_FREE(   ADD_SOUTH_HERE
                    | NO_SOUTH
                    | ADD_NORTH_HERE
                    | NO_NORTH
                   )


     /*eliminate one-tile-islands, except for 'arctic'*/
  /*eliminate_one_tile_islands() ;*/ /*not in this scenario*/

     /*add rivers*/
  fprintf( log_fp, "adding rivers\n" ) ;
  riv.rain_func = rain_on_land_uniform ;
  riv.visibility = 30 ;
  riv.change_tiles = TRUE ;
  riv.change_mountains = FALSE ;
  add_rivers( & riv ) ;
  fprintf(log_fp, "rivers added\n" ) ;

     /*convert ocean to coast*/
  set_ocean_and_coast() ;

     /*add bonus resources & plains*/
  set_bonus_resources() ;




  clear_flags( 0xff ) ; /*does rate_1st_pos() use flags???*/


     /*add starting positions*/
  WORLD_FLAGS_ALLOC( DONT_SETTLE_DOWN )
  clear_flags( DONT_SETTLE_DOWN ) ;

     /*no start positions near poles*/
  for ( tile_index = 0 ; tile_index < LX3 ; tile_index++ ) {
    SET_WORLD_FLAGS(                        tile_index, DONT_SETTLE_DOWN )
    SET_WORLD_FLAGS( total_no_of_tiles -1 - tile_index, DONT_SETTLE_DOWN )
  }

  WORLD_FLAGS_ALLOC( TAGGED )

  clear_flags( TAGGED ) ;
  SET_WORLD_FLAGS( north_anchor, TAGGED )
  tag_whole_land() ;
  for ( i = 0 ; i < comp_opponents_area1 ; i++ ) {
    best_index = best_starting_pos_simple( TAGGED | DONT_SETTLE_DOWN, TAGGED,
                     0x00, DONT_SETTLE_DOWN, starting_pos_dist ) ;
    if (best_index == INVALID_TIDX) {
      fprintf( log_fp, "cannot find a suitable start position (north)\n" ) ;
       printf(         "cannot find a suitable start position (north)\n" ) ;
      map_gen_exit() ;
    }
    /*PRT_VAR((unsigned long)best_index,lu)*/
    world [ best_index ] |= NORMAL_STARTPOS ; /*comp opponents on north side*/
    ASSERT(((world_flags [best_index]) & DONT_SETTLE_DOWN) == DONT_SETTLE_DOWN)
  }

  clear_flags( TAGGED ) ;
  SET_WORLD_FLAGS( south_anchor, TAGGED )
  tag_whole_land() ;
  for ( i = 0 ; i < comp_opponents_area2 ; i++ ) {
    best_index = best_starting_pos_simple( TAGGED | DONT_SETTLE_DOWN, TAGGED,
                     0x00, DONT_SETTLE_DOWN, starting_pos_dist ) ;
    if (best_index == INVALID_TIDX) {
      fprintf( log_fp, "cannot find a suitable start position (south)\n" ) ;
       printf(         "cannot find a suitable start position (south)\n" ) ;
      map_gen_exit() ;
    }
    world [ best_index ] |= NORMAL_STARTPOS ; /*comp opponents on south side*/
  }

  if (human_start_pos != 0) { /*do generate a human startpos*/
    clear_flags( TAGGED ) ;
    if (human_start_pos & 0x01) {
      SET_WORLD_FLAGS( north_anchor, TAGGED )
    }
    if (human_start_pos & 0x02) {
      SET_WORLD_FLAGS( south_anchor, TAGGED )
    }
    tag_whole_land() ;
    best_index = best_starting_pos_simple( TAGGED | DONT_SETTLE_DOWN, TAGGED,
                     0x00, DONT_SETTLE_DOWN, starting_pos_dist ) ;
    if (best_index == INVALID_TIDX) {
      fprintf( log_fp, "cannot find a suitable start position (human)\n" ) ;
       printf(         "cannot find a suitable start position (human)\n" ) ;
      map_gen_exit() ;
    }
    world [ best_index ] |= HUMAN_STARTPOS ;
  }

  WORLD_FLAGS_FREE( TAGGED )


     /*add special resources and dead lands*/
  WORLD_FLAGS_ALLOC( TAGGED )

  clear_flags( TAGGED ) ;
  SET_WORLD_FLAGS( north_anchor, TAGGED )
  tag_whole_land() ;
  for ( i = 0 ; i < 12 ; i+= 2 ) { /*even resource indices*/
    best_index = best_starting_pos_simple( TAGGED | DONT_SETTLE_DOWN, TAGGED,
                                           0x00, DONT_SETTLE_DOWN, 600 ) ;
    if (best_index != INVALID_TIDX) {
      set_special_resource( best_index, special_resource_tab [i] ) ;
    }
    else {
      if (i <= 2) {
        fprintf( log_fp, "cannot place enough special resources (north)\n" ) ;
         printf(         "cannot place enough special resources (north)\n" ) ;
        map_gen_exit() ;
      }
      else {
        break ; /*place less than 6 resources on this side --- it's ok*/
      }
    }
  }

  clear_flags( TAGGED ) ;
  SET_WORLD_FLAGS( south_anchor, TAGGED )
  tag_whole_land() ;
  for ( i = 1 ; i < 12 ; i+= 2 ) { /*odd resource indices*/
    best_index = best_starting_pos_simple( TAGGED | DONT_SETTLE_DOWN, TAGGED,
                                           0x00, DONT_SETTLE_DOWN, 600 ) ;
    if (best_index != INVALID_TIDX) {
      set_special_resource( best_index, special_resource_tab [i] ) ;
    }
    else {
      if (i <= 2) {
        fprintf( log_fp, "cannot place enough special resources (south)\n" ) ;
         printf(         "cannot place enough special resources (south)\n" ) ;
        map_gen_exit() ;
      }
      else {
        break ; /*place less than 6 resources on this side --- it's ok*/
      }
    }
  }

  WORLD_FLAGS_FREE( TAGGED )
  WORLD_FLAGS_FREE( DONT_SETTLE_DOWN )
}

/*-------------------->   big_river_add_one_tile   <------------- 2012-Feb-15
This function adds one north resp south tile to the continent.
All world_flags are updated.
-----------------------------------------------------------------------------
Used functions: draw_on_flag_pattern_match, draw_island_tile, SET_WORLD_FLAGS,
                tag_neighborhood_tiles, tag_inside_radius
Globals:        world, world_flags
Internals:
Parameters:	- own_add_flag    flag used to tag where 'own' land is to
                                  be added
		- own_no_flag     flag used to tag where no 'own' land shall
		                  be added
		- other_no_flag   flag used to tag where no 'other' land
		                  shall be added
		- bp_*            base probabilities for desert, prairie
		                  and tundra
Return value:	TRUE if no tile could be added, FALSE else
Exitcode:	--
---------------------------------------------------------------------------*/
static BIT big_river_add_one_tile(
           U8 own_add_flag, U8 own_no_flag, U8 other_no_flag,
           U32 bp_desert, U32 bp_prairie, U32 bp_tundra )
{
THIS_FUNC(big_river_add_one_tile)
  TIDX tile_index ; /*scratch pad*/


  tile_index = draw_on_flag_pattern_match(
                own_add_flag | own_no_flag,    /*mask ...*/
                own_add_flag                  /*... matches this*/
                                         ) ;
  /*PRT_VAR((unsigned long)tile_index,lu)*/
  if (tile_index == INVALID_TIDX) {
    return TRUE ;
  }
     /*add at "tile_index"*/
  world [tile_index] = draw_island_tile( tile_index,
                                         bp_desert, bp_prairie, bp_tundra
                                       ) ;
  SET_WORLD_FLAGS(        tile_index, own_no_flag ) /*do not add twice*/
  tag_neighborhood_tiles( tile_index, own_add_flag ) ;
  tag_inside_radius(      tile_index, water_width, other_no_flag ) ;

  return FALSE ;
}

/*-------------------->   x   <---------------------------------- 2012-Feb-14
This function x
-----------------------------------------------------------------------------
Used functions:
Globals:
Internals:
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
