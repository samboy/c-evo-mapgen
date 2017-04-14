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
scn_fjor.c

This file generates a C-Evo-map file (Scenario: Fjords).
*****************************************************************************
History: (latest change first)
2015-Jul-30..Aug-01: derived from "scn_plai.c"
*****************************************************************************
Global objects:
- void scenario_fjords( void )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include "math.h"

#include "map_gen.h"
#include "read_ini.h"

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

#define DEFAULT_MOUNTAIN_STRIPE 12
#define DEFAULT_MOUNTAIN_STRIPE_UNCOND 2

#define DEFAULT_GRASSLAND_PERCENTAGE 1
#define DEFAULT_GRASSLAND_MIN_SIZE 1
#define DEFAULT_GRASSLAND_MAX_SIZE 6

#define DEFAULT_FOREST_PERCENTAGE 1
#define DEFAULT_FOREST_MIN_SIZE 2
#define DEFAULT_FOREST_MAX_SIZE 8


/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

/*static TILE ocean( TIDX tidx ) ;*/
static TILE grass( TIDX tidx ) ;
static TILE forest( TIDX tidx ) ;

/*static BIT add_one_tile( U8 add_here ) ;*/


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/

   /*minimum distance for area start points*/
static const U16 area_dist [ /*map_size_idx*/ ] = {
  1500, /* 35%*/
  1800, /* 50%*/
  2000, /* 70%*/
  2200, /*100%*/
  2500, /*150%*/
  3000  /*230%*/
} ;


/*-------------------->   scenario_fjords   <-------------------- 2015-Aug-01
Scenario description see "map_gen.ini"
-----------------------------------------------------------------------------
Used functions:
Globals:
Internals:
Parameters:	--
Return value:	void
Exitcode:	x
---------------------------------------------------------------------------*/
void scenario_fjords( void )
{
THIS_FUNC(scenario_fjords)
char* _this_func = "scenario_fjords" ; /*for WORLD_FLAGS_ALLOC*/
  CLUSTER_PARAMS clus ;
  RIVER_PARAMS riv ;
  STARTPOS_PARAMS sp ;
  TIDX tile_index ; /*loop control*/
  TIDX mirror ; /*north-south shortcut*/
  U32  delta_idx ;

  U16 no_of_tiles_target ; /*scratch pad for cluster generation*/
  U16 no_of_tiles ;

  U8 i ; /*loop control*/
  U8 temp ;



     /*check params from "map_gen.ini"*/
  if ( ! found_comp_opponents) {
    comp_opponents = 1 ;
  }

     /*default minimum requirement for startpos*/
  if ( ! found_startpos_min_rating) {
    startpos_min_rating = 41 ;
  }

  if ( ! found_mountain_stripe) {
    /*mountain_stripe = DEFAULT_MOUNTAIN_STRIPE ;*/
    mountain_stripe = (90 * LY) / 100 ;
    PRT_VAR(mountain_stripe,u)
  }
  if ( ! found_mountain_stripe_uncond) {
    /*mountain_stripe_uncond = DEFAULT_MOUNTAIN_STRIPE_UNCOND ;*/
    mountain_stripe_uncond = (20 * LY) / 100 ;
    PRT_VAR(mountain_stripe_uncond,u)
  }

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



     /*fill map with water*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    world [ tile_index ] = OCEAN ;
  }

#ifdef NEVER
     /*add north mountains*/
  delta_idx = mountain_stripe_uncond * LX ;
  for ( tile_index = 0 ; tile_index <= delta_idx ; tile_index++ ) {
    world [ tile_index ] = MOUNTAINS ;
  }

  delta_idx = (mountain_stripe - mountain_stripe_uncond) * LX ;
  start_idx = mountain_stripe_uncond * LX ;
  end_idx = mountain_stripe * LX ;
  for ( tile_index = start_idx ; tile_index <= end_idx ; tile_index++ ) {
    set_neighborhood_tiles( tile_index, & NT ) ;
    /*mountain_prob = (100 * (end_idx - tile_index)) / (delta_idx) ;*/
    temp_db = (double)(end_idx - tile_index) / (double)(delta_idx) ;
    mountain_prob = 100 * sqrt( temp_db ) ;
    /*mountain_prob = 100 * sqrt( temp_db )*/
      /** (double)count_city_tiles( tile_index, TTL_MOUNTAINS ) /9.0 ;*/
    if (mountain_prob > 100) {
      mountain_prob = 100 ;
    }

    if (count_neighborhood_tiles( tile_index, TTL_MOUNTAINS ) == 0) {
      mountain_prob = 0 ;
    }
    dice = random_draw_range( 1, 100 ) ;
    if (dice <= mountain_prob) {
      world [ tile_index ] = MOUNTAINS ;
         /*assure that no WATER tile is disconnected*/
      if (  ((NT.nw_tile) & BASIC_TILE_TYPE_MASK) == OCEAN) {
        if (((NT.w_tile ) & BASIC_TILE_TYPE_MASK) == MOUNTAINS) {
          if (random_draw() & 0x1) {
            world [ tile_index ] = OCEAN ;
          }
          else {
            world [ NT.w_idx ] = OCEAN ;
          }
        }
      }
    }
  }
#endif /*NEVER*/


     /*add north mountains*/
  delta_idx = mountain_stripe * LX ;
  for ( tile_index = 0 ; tile_index <= delta_idx ; tile_index++ ) {
    world [ tile_index ] = MOUNTAINS ;
  }

  DEB((stderr,"adding rivers\n"))
  riv.rain_func = rain_on_land_uniform ;
  riv.visibility = 30 ;
  riv.change_tiles = TRUE ;
  riv.change_mountains = FALSE ;
  add_rivers( & riv ) ;
  DEB((stderr,"rivers added\n"))

  for ( i = 30 ; i > 1 ; i-- ) {
    for ( tile_index = total_no_of_tiles / 8 ; tile_index <= total_no_of_tiles ; tile_index++ ) {
      if (erosion [tile_index] >= i) {
        world [tile_index] = OCEAN ;
      }
    }
  }

  map_gen_exit() ;





     /*set SHORE flags (Lakes *and* BigRivers)*/
  WORLD_FLAGS_ALLOC( IS_SHORE )
  clear_flags(       IS_SHORE ) ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_LAND )) {
      if (count_neighborhood_tiles( tile_index, TTL_WATER ) > 0) {
        world_flags [tile_index] |= IS_SHORE ;
      }
    }
  }


     /*add some GRASSLAND (nearby WATER)*/
  TIME_STAMP("placing grass")
  if (grassland_percentage != 0) { /*add grass*/
    no_of_tiles_target = (grassland_percentage * total_no_of_tiles) / 100 ;
    no_of_tiles = 0 ;
    PRT_VAR(no_of_tiles_target,u)

    /*WORLD_FLAGS_ALLOC( TAGGED )*/
    /*clear_flags(       TAGGED ) ;*/
    WORLD_FLAGS_ALLOC( TAGGED | TEMP_TAG )
    clear_flags(       TAGGED | TEMP_TAG ) ;

    clus.method = CLUSTER_METHOD_CITY_RADIUS ;
    clus.no_of_clusters = 1 ;
    clus.area_mask = (IS_SHORE | TAGGED) ;
    clus.area = IS_SHORE ;
    clus.area_only_for_start_points = TRUE ;
    /*clus.size = xxx ;*/
    /*clus.min_size = grassland_min_size ;*/
    clus.min_size = 1 ;
    clus.tile_func = grass ;
    clus.set_flags = TEMP_TAG ;
    clus.clear_flags = 0x0 ;
    clus.edge_flag = 0x0 ;
    clus.grow_into = TTL_PRAIRIE ;
    clus.task_str = "add grass" ;
    clus.flag1 = 0x0 ;
    clus.dist1 = 0 ;
    clus.flag2 = 0x0 ;
    clus.dist2 = 0 ;

    while (no_of_tiles < no_of_tiles_target) {
      clus.size = random_draw_range( grassland_min_size, grassland_max_size ) ;
      add_clusters( & clus ) ;
      no_of_tiles += clus.actual_size ;
    } /*end of grass generation*/
    WORLD_FLAGS_FREE( TAGGED | TEMP_TAG )
  }
  WORLD_FLAGS_FREE( IS_SHORE )
  TIME_STAMP("grass placed")


     /*add some FOREST*/
  TIME_STAMP("placing forest")
  if (forest_percentage != 0) { /*add forest*/
    no_of_tiles_target = (forest_percentage * total_no_of_tiles) / 100 ;
    no_of_tiles = 0 ;
    PRT_VAR(no_of_tiles_target,u)

    WORLD_FLAGS_ALLOC( TAGGED | TEMP_TAG | IS_SHORE )
    clear_flags(       TAGGED | TEMP_TAG | IS_SHORE ) ;

       /*tag all PRAIRIE tiles with contact to MOUNTAINS*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      if ((world [tile_index] & BASIC_TILE_TYPE_MASK) == PRAIRIE) {
        temp = count_neighborhood_tiles( tile_index, TTL_MOUNTAINS ) ;
        if (temp > 0 && temp < 3) {
          /*PRT_VAR((unsigned)tile_index,u)*/
          world_flags [tile_index] |= IS_SHORE ;
        }
      }
    }

    clus.method = CLUSTER_METHOD_ADJACENT ;
    clus.no_of_clusters = 1 ;
    clus.area_mask = (IS_SHORE | TAGGED) ;
    clus.area = IS_SHORE ;
    clus.area_only_for_start_points = TRUE ;
    /*clus.size = xxx ;*/
    /*clus.min_size = forest_min_size ;*/
    clus.min_size = 1 ;
    clus.tile_func = forest ;
    clus.set_flags = TEMP_TAG ;
    clus.clear_flags = 0x0 ;
    clus.edge_flag = 0x0 ;
    clus.grow_into = TTL_PRAIRIE ;
    clus.task_str = "add forest" ;
    clus.flag1 = 0x0 ;
    clus.dist1 = 0 ;
    clus.flag2 = 0x0 ;
    clus.dist2 = 0 ;

    while (no_of_tiles < no_of_tiles_target) {
      clus.size = random_draw_range( forest_min_size, forest_max_size ) ;
      add_clusters( & clus ) ;
      no_of_tiles += clus.actual_size ;
    } /*end of forest generation*/
    WORLD_FLAGS_FREE( TAGGED | TEMP_TAG | IS_SHORE )
  }
  TIME_STAMP("forest placed")



     /*convert ocean to coast*/
  set_ocean_and_coast() ;

     /*add bonus resources & plains*/
  set_bonus_resources() ;


     /*add special resources in the north and south mountain stripes*/
  DEB((stderr,"adding special resources\n"))
  WORLD_FLAGS_ALLOC( TAGGED | ADD_HERE0 )
  clear_flags(       TAGGED ) ;

     /*tag all north and south stripe tiles*/
     /*but stay away from WATER*/
     /*and make shure to be deep in the mountains*/
  for ( tile_index = 0 ; tile_index < mountain_stripe_uncond * LX ;
        tile_index++ ) {
    mirror = total_no_of_tiles - tile_index ;
    if (count_city_tiles( tile_index, TTL_WATER ) == 0) {
      if (count_city_tiles( tile_index, TTL_MOUNTAINS | TTL_POLE ) > 17) {
        world_flags [ tile_index] |= TAGGED ;
      }
    }
    if (count_city_tiles( mirror, TTL_WATER ) == 0) {
      if (count_city_tiles( mirror, TTL_MOUNTAINS | TTL_POLE ) > 17) {
        world_flags [ mirror ] |= TAGGED ;
      }
    }
  }
  for ( i = 0 ; i < 12 ; i++ ) {
    clear_flags( ADD_HERE0 ) ;
    tile_index = draw_on_flag_pattern_match( TAGGED, TAGGED ) ;
    /*PRT_VAR((unsigned)tile_index,u)*/
    if (tile_index == INVALID_TIDX) {
      if (i >= 3) {
        fprintf( stderr, "could place %u only special resources\n",
                         (unsigned)i) ;
        break ; /*place less than 12 specials*/
      }
      else {
        fprintf( stderr, "not enough places for special resources\n" ) ;
        map_gen_exit() ;
      }
    }
    tag_city_tiles( tile_index, ADD_HERE0 ) ;
    untag_inside_radius( tile_index, 450, TAGGED ) ; /*no other city here*/
    world [tile_index] = ((world [tile_index]) & ~BASIC_TILE_TYPE_MASK )
                       | HILLS ; /*preserve river*/
    world_flags [tile_index] &= ~ADD_HERE0 ;

    tile_index = draw_on_flag_pattern_match( ADD_HERE0, ADD_HERE0 ) ;
    world [tile_index] = ((world [tile_index]) & ~BASIC_TILE_TYPE_MASK )
                       | HILLS ; /*preserve river*/
    world_flags [tile_index] &= ~ADD_HERE0 ;

    tile_index = draw_on_flag_pattern_match( ADD_HERE0, ADD_HERE0 ) ;
    world [tile_index] |= special_resource_tab [i] ;
  }
  WORLD_FLAGS_FREE( TAGGED | ADD_HERE0 )
  DEB((stderr,"special resources added\n"))


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
}

/*-------------------->   ocean   <---------------------------- 2012-Sep-24*/
/*-------------------->   grass   <---------------------------- 2012-Sep-24*/
/*-------------------->   forest   <--------------------------- 2012-Sep-24*/
/*These functions are for cluster generation.
-----------------------------------------------------------------------------
Used functions:
Globals:
Internals:
Parameters:	- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
#ifdef NOT_USED
static TILE ocean( TIDX tidx )
{
THIS_FUNC(ocean)
  /*PRT_VAR((unsigned)tidx,u)*/

     /*may grow into BigRivers*/
  /*ASSERT_ALT((world [tidx] & BASIC_TILE_TYPE_MASK) != OCEAN,*/
                /*world [tidx] = DESERT ;*/
                /*map_gen_exit() ;)*/
  return OCEAN ;
}
#endif /*NOT_USED*/

static TILE grass( TIDX tidx )
{
THIS_FUNC(grass)
  /*PRT_VAR((unsigned)tidx,u)*/
  ASSERT_ALT((world [tidx] & BASIC_TILE_TYPE_MASK) != GRASSLAND,
                world [tidx] = DESERT ;
                map_gen_exit() ;)
  return (world [tidx] & ~BASIC_TILE_TYPE_MASK) | GRASSLAND ;
                                      /*preserve river*/
}

static TILE forest( TIDX tidx )
{
THIS_FUNC(forest)
  /*PRT_VAR((unsigned)tidx,u)*/
  ASSERT_ALT((world [tidx] & BASIC_TILE_TYPE_MASK) != FOREST,
                world [tidx] = DESERT ;
                map_gen_exit() ;)
  return (world [tidx] & ~BASIC_TILE_TYPE_MASK) | FOREST ;
                                      /*preserve river*/
}

/*-------------------->   x   <---------------------------------- 2012-Oct-01
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
