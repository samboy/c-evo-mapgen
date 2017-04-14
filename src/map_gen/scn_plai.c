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
scn_plai.c

This file generates a C-Evo-map file (Scenario: Great Plains).
*****************************************************************************
History: (latest change first)
2016-Aug-08: clean log file implementation
2015-Oct-19: allow lakes near poles as an option
2015-Jun-05: new RIVER_PARAMS initialized
2015-May-14: Debugging (seed point distance)
2013-Apr-13: clus.min_size = 1 for GRASSLAND and FOREST
2013-Apr-12: used TEMP_TAG for clusters to avoid ASSERT problem
2012-Dec-19: Added "effective_area_dist" to fix bug (water_width>area_dist)
2012-Nov-01: Place less than 12 special resources (minimum three)
2012-Sep-22..Oct-01: still initial version
2012-Jun-06..08: derived from "scn_rive.c"
*****************************************************************************
Global objects:
- void scenario_great_plaines( void )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include <stdio.h>

/*#define DEBUG*/
#include <debug.h>
#include "map_gen.h"
#include "read_ini.h"


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

static TILE ocean( TIDX tidx ) ;
static TILE grass( TIDX tidx ) ;
static TILE forest( TIDX tidx ) ;

static BIT add_one_tile( U8 add_here ) ;


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


/*-------------------->   scenario_great_plains   <-------------- 2016-Aug-08
Scenario description see "map_gen.ini"
-----------------------------------------------------------------------------
Used functions:
Globals:
Internals:
Parameters:	--
Return value:	void
Exitcode:	x
---------------------------------------------------------------------------*/
void scenario_great_plains( void )
{
/*THIS_FUNC(scenario_great_plains)*/
char* _this_func = "scenario_great_plains" ; /*for WORLD_FLAGS*/
  CLUSTER_PARAMS clus ;
  RIVER_PARAMS riv ;
  STARTPOS_PARAMS sp ;
  TIDX tile_index ; /*loop control*/
  TIDX mirror ; /*north-south shortcut*/
  TIDX anchor [4] ; /*area anchor points*/
  TIDX start_idx, end_idx ; /*for mountain stripes*/
  U32  delta_idx ;

  U16 effective_area_dist ; /*the value actually used*/
  U16 no_of_tiles_target ; /*scratch pad for cluster generation*/
  U16 no_of_tiles ;

  U16 dice, mountain_prob ;

  U8 i ; /*loop control*/
  U8 temp ;
  U8 ongoing ; /*array of four flags*/
  U8 i_mask ;



     /*check params from "map_gen.ini"*/
  if ( ! found_comp_opponents) {
    comp_opponents = 1 ;
  }

     /*default minimum requirement for startpos*/
  if ( ! found_startpos_min_rating) {
    startpos_min_rating = 8 ;
  }

  if ( ! found_mountain_stripe) {
    mountain_stripe = DEFAULT_MOUNTAIN_STRIPE ;
  }
  if ( ! found_mountain_stripe_uncond) {
    mountain_stripe_uncond = DEFAULT_MOUNTAIN_STRIPE_UNCOND ;
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


  /*PRT_VAR((unsigned)water_width,u)*/
  fprintf( log_fp, "scenario_great_plains: water_width = %u\n",
                   (unsigned)water_width ) ;

  if (water_width == 0) { /*fill map with PRAIRIE */
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      world [ tile_index ] = PRAIRIE ;
    }
  }
  else { /*fill map with OCEAN, then add four land areas */
    /*PRT_VAR((unsigned)(area_dist[map_size_idx]),u)*/
    fprintf( log_fp, "scenario_great_plains: area_dist[map_size_idx] = %u\n",
                     (unsigned)(area_dist [map_size_idx]) ) ;
    effective_area_dist = max( area_dist[map_size_idx], water_width + 100 ) ;
    /*PRT_VAR((unsigned)(effective_area_dist),u)*/
    fprintf( log_fp, "scenario_great_plains: effective_area_dist = %u\n",
                     (unsigned)(effective_area_dist) ) ;

    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      world [ tile_index ] = OCEAN ;
    }
       /*set anchor points*/
    WORLD_FLAGS_ALLOC( TAGGED )
    clear_flags(       TAGGED ) ;
    for ( i = 0 ; i < 4 ; i++ ) {
      anchor [i] = draw_on_flag_pattern_match( TAGGED, 0x0 ) ;
      fprintf( log_fp, "seed point %u: tile %lu\n",
                       (unsigned)i, (unsigned long)(anchor [i]) ) ;
      if (anchor [i] == INVALID_TIDX) {
        fprintf( log_fp, "can find only %u seed points\n", (unsigned)i ) ;
         printf(         "can find only %u seed points\n", (unsigned)i ) ;
        map_gen_exit() ;
      }
      tag_inside_radius( anchor [i], effective_area_dist, TAGGED ) ;
    }
    WORLD_FLAGS_FREE( TAGGED )
    /*
    Flags used to shape the map:
    .......x   add area0 here
    ...x....   don't add area0 here
    ......x.   add area1 here
    ..x.....   don't add area1 here
    .....x..   add area2 here
    .x......   don't add area2 here
    ....x...   add area3 here
    x.......   don't add area3 here
    */
    WORLD_FLAGS_ALLOC( 0xff )
    clear_flags(       0xff ) ;
    for ( i = 0 ; i < 4 ; i++ ) { /*set ADD_HERE flags for anchor points*/
      world_flags [anchor [i]] |= (0x01 << i) ;
    }

    /*
    'ongoing' flags:
    .......x   ongoing0
    ......x.   ongoing1
    .....x..   ongoing2
    ....x...   ongoing3
    */
    ongoing = 0x0f ;
    while (ongoing != 0x0) { /*at least one area is ongoing*/
      for ( i = 0 ; i < 4 ; i++ ) {
        i_mask = 1 << i ;
        if (ongoing & i_mask) { /*add one tile to area i*/
          if ( add_one_tile( i_mask )) {
            ongoing &= ~i_mask ;
          }
      }
    }
  } /*end while*/

  WORLD_FLAGS_FREE( 0xff )
  }

  /*TIME_STAMP("placing lakes")*/
  log_with_timestamp( "placing lakes" ) ;
  if (lake_percentage != 0) { /*add lakes*/
    no_of_tiles_target = (lake_percentage * total_no_of_tiles) / 100 ;
    no_of_tiles = 0 ;
    /*PRT_VAR(no_of_tiles_target,u)*/
    fprintf( log_fp, "scenario_great_plains: no_of_tiles_target = %u\n",
                                             no_of_tiles_target ) ;

    WORLD_FLAGS_ALLOC( TAGGED | NO_LAKE_HERE | IS_SHORE )
    clear_flags(       TAGGED | NO_LAKE_HERE | IS_SHORE ) ;

    if ( ! allow_lakes_near_poles) {
      delta_idx = mountain_stripe * LX ; /*inhibit lakes in mountain stripe*/
      for ( tile_index = 0 ; tile_index <= delta_idx ; tile_index++ ) {
        world_flags [                     tile_index ] |= NO_LAKE_HERE ;
        world_flags [ total_no_of_tiles - tile_index ] |= NO_LAKE_HERE ;
      }
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
  }
  /*TIME_STAMP("lakes placed")*/
  log_with_timestamp( "lakes placed" ) ;


     /*not in this scenario*/
  /*eliminate_one_tile_islands() ;*/


     /*add rivers*/
  fprintf( log_fp, "adding rivers\n" ) ;
  riv.rain_func = rain_on_land_uniform ;
  riv.visibility = 80 ;
  riv.change_mountains = FALSE ;
  if (water_width != 0) {
    /*riv.visibility /= 4 ;*/
    riv.visibility /= 3 ;
  }
  /*PRT_VAR((unsigned)(riv.visibility),u)*/
  fprintf( log_fp, "scenario_great_plains: riv.visibility = %u\n",
                                (unsigned)(riv.visibility) ) ;
  riv.change_tiles = FALSE ;
  add_rivers( & riv ) ;
  fprintf( log_fp, "rivers added\n" ) ;


     /*add north and south mountains, preserving WATER and RIVER*/
  delta_idx = mountain_stripe_uncond * LX ;
  for ( tile_index = 0 ; tile_index <= delta_idx ; tile_index++ ) {
    if ( ! is_in_TTL( tile_index, TTL_WATER | TTL_RIVER )) {
      world [ tile_index ] = MOUNTAINS ;
    }
  }

  delta_idx = (mountain_stripe - mountain_stripe_uncond) * LX ;
  start_idx = mountain_stripe_uncond * LX ;
  end_idx = mountain_stripe * LX ;
  for ( tile_index = start_idx ; tile_index <= end_idx ; tile_index++ ) {
    mountain_prob = (100 * (end_idx - tile_index)) / (delta_idx) ;
    if (is_in_TTL( tile_index, TTL_WATER | TTL_RIVER )) {
      mountain_prob = 0 ;
    }
    if (count_neighborhood_tiles( tile_index, TTL_MOUNTAINS ) == 0) {
      mountain_prob = 0 ;
    }
    dice = random_draw_range( 1, 100 ) ;
    if (dice <= mountain_prob) {
      world [ tile_index ] = MOUNTAINS ;
    }
  }

  delta_idx = mountain_stripe_uncond * LX ;
  for ( tile_index = total_no_of_tiles - delta_idx ;
        tile_index < total_no_of_tiles ; tile_index++ ) {
    if ( ! is_in_TTL( tile_index, TTL_WATER | TTL_RIVER )) {
      world [ tile_index ] = MOUNTAINS ;
    }
  }

  delta_idx = (mountain_stripe - mountain_stripe_uncond) * LX ;
  start_idx = total_no_of_tiles - mountain_stripe * LX ;
  end_idx =   total_no_of_tiles - mountain_stripe_uncond * LX ;
  for ( tile_index = end_idx ; tile_index >= start_idx ; tile_index-- ) {
    mountain_prob = (100 * (tile_index - start_idx)) / (delta_idx) ;
    if (is_in_TTL( tile_index, TTL_WATER | TTL_RIVER )) {
      mountain_prob = 0 ;
    }
    if (count_neighborhood_tiles( tile_index, TTL_MOUNTAINS ) == 0) {
      mountain_prob = 0 ;
    }
    dice = random_draw_range( 1, 100 ) ;
    if (dice <= mountain_prob) {
      world [ tile_index ] = MOUNTAINS ;
    }
  }


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
  /*TIME_STAMP("placing grass")*/
  log_with_timestamp( "placing grass" ) ;
  if (grassland_percentage != 0) { /*add grass*/
    no_of_tiles_target = (grassland_percentage * total_no_of_tiles) / 100 ;
    no_of_tiles = 0 ;
    /*PRT_VAR(no_of_tiles_target,u)*/
    fprintf( log_fp, "scenario_great_plains: no_of_tiles_target = %u\n",
                                             no_of_tiles_target ) ;

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
  /*TIME_STAMP("grass placed")*/
  log_with_timestamp( "grass placed" ) ;


     /*add some FOREST*/
  log_with_timestamp( "placing forest" ) ;
  if (forest_percentage != 0) { /*add forest*/
    no_of_tiles_target = (forest_percentage * total_no_of_tiles) / 100 ;
    no_of_tiles = 0 ;
    /*PRT_VAR(no_of_tiles_target,u)*/
    fprintf( log_fp, "scenario_great_plains: no_of_tiles_target = %u\n",
                                             no_of_tiles_target ) ;

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
  /*TIME_STAMP("forest placed")*/
  log_with_timestamp( "forest placed" ) ;



     /*convert ocean to coast*/
  set_ocean_and_coast() ;

     /*add bonus resources & plains*/
  set_bonus_resources() ;


     /*add special resources in the north and south mountain stripes*/
  fprintf( log_fp, "adding special resources\n" ) ;
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
        fprintf( log_fp, "could place %u only special resources\n",
                         (unsigned)i) ;
        break ; /*place less than 12 specials*/
      }
      else {
        fprintf( log_fp, "not enough places for special resources\n" ) ;
         printf(         "not enough places for special resources\n" ) ;
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
  fprintf( log_fp, "special resources added\n" ) ;


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
-> move to aux_clus.c
-----------------------------------------------------------------------------
Used functions:
Globals:
Internals:
Parameters:	- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
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

/*-------------------->   add_one_tile   <----------------------- 2012-Oct-01
This function adds one tile to an area.
All world_flags are updated.
-----------------------------------------------------------------------------
Used functions: draw_on_flag_pattern_match,
                tag_neighborhood_tiles, tag_inside_radius
Globals:        world, world_flags
Internals:
Parameters:	- add_here is a bit mask    0x01 .. 0x08 (i = 0,1,2,3)
Return value:	TRUE if no tile could be added, FALSE else
Exitcode:	--
---------------------------------------------------------------------------*/
static BIT add_one_tile( U8 add_here )
{
THIS_FUNC(add_one_tile)
  TIDX tile_index ; /*scratch pad*/
  U8 dont_add_mask = add_here << 4 ;


  tile_index = draw_on_flag_pattern_match(
                add_here | dont_add_mask,    /*mask ...*/
                add_here                     /*... matches this*/
                                         ) ;
  /*PRT_VAR((unsigned long)tile_index,lu)*/
  if (tile_index == INVALID_TIDX) {
    return TRUE ;
  }
     /*add at "tile_index"*/
  world [tile_index] = PRAIRIE ;
  world_flags [tile_index] |= 0xf0 ; /*add nothing more here*/
  tag_neighborhood_tiles( tile_index, add_here ) ;
  tag_inside_radius(      tile_index, water_width, 0xf0 & ~dont_add_mask ) ;
                                 /*inhibit three other areas inside radius*/

  return FALSE ;
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
