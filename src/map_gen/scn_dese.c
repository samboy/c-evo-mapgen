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
 * 
 *  Modified by Sam Trenholme
 */
/********************************************************** Ulrich Krueger **
scn_dese.c

This file generates a C-Evo-map file (Scenario: Desert).
*****************************************************************************
History: (latest change first)
2017-Apr-05: debugging (identical anchor points)
2017-Feb-22..25: debugging (assert failed, world_flags)
2016-Aug-17: clean log file implementation
2015-Jun-03..06: derived from "scn_plai.c"
*****************************************************************************
Global objects:
- void scenario_desert( void )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include "map_gen.h"
#include "read_ini.h"

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

#define DEFAULT_GRASSLAND_PERCENTAGE 1
#define DEFAULT_GRASSLAND_MIN_SIZE 1
#define DEFAULT_GRASSLAND_MAX_SIZE 6

#define DEFAULT_PRAIRIE_PERCENTAGE 1
#define DEFAULT_PRAIRIE_MIN_SIZE 1
#define DEFAULT_PRAIRIE_MAX_SIZE 6

#define DEFAULT_LAND_AMOUNT 1


/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

static BIT add_one_tile( U8 add_here ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/



/*--  internal variables  -------------------------------------------------*/



/*-------------------->   scenario_desert   <-------------------- 2017-Mar-27
Scenario description see "map_gen.ini"
-----------------------------------------------------------------------------
Used functions:
Globals:   world [], world_flags [], special_resource_tab []
Internals: --
Parameters: --
Return value: void
Exitcode:     EXITCODE_UNSPECIFIED_ERR
---------------------------------------------------------------------------*/
void scenario_desert( void )
{
/*THIS_FUNC(scenario_desert)*/
char* _this_func = "scenario_desert" ; /*for WORLD_FLAGS*/
  CLUSTER_PARAMS clus ;
  RIVER_PARAMS riv ;
  STARTPOS_PARAMS sp ;
  TIDX tile_index ; /*loop control*/
  TIDX anchor [4] ; /*area anchor points*/

  U16 no_of_tiles_target ; /*scratch pad for cluster generation*/
  U16 no_of_tiles ;

  U8 i ; /*loop control*/
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

     /*use these scenario-specific defaults if not specified in map_gen.ini*/
  if ( ! found_grassland_percentage) {
    grassland_percentage = DEFAULT_GRASSLAND_PERCENTAGE ;
  }
  if ( ! found_grassland_min_size) {
    grassland_min_size = DEFAULT_GRASSLAND_MIN_SIZE ;
  }
  if ( ! found_grassland_max_size) {
    grassland_max_size = DEFAULT_GRASSLAND_MAX_SIZE ;
  }

  if ( ! found_prairie_percentage) {
    prairie_percentage = DEFAULT_PRAIRIE_PERCENTAGE ;
  }
  if ( ! found_prairie_min_size) {
    prairie_min_size = DEFAULT_PRAIRIE_MIN_SIZE ;
  }
  if ( ! found_prairie_max_size) {
    prairie_max_size = DEFAULT_PRAIRIE_MAX_SIZE ;
  }
  if ( ! found_land_amount) {
    land_amount = DEFAULT_LAND_AMOUNT ;
  }

     /*set anchor points*/
  WORLD_FLAGS_ALLOC( TAGGED )
  clear_flags(       TAGGED ) ;
  anchor [0] = draw_on_flag_pattern_match( TAGGED, 0x0 ) ;
  fprintf( log_fp, "seed point 0: tile %lu\n",
                   (unsigned long)(anchor [0]) ) ;

  tag_inside_radius( anchor [0], 500, TAGGED ) ;
  world_flags [ anchor [0] ] &= ~TAGGED ; /*untag center tile*/
                       /*anchor points must not be identical*/

  anchor [1] = draw_on_flag_pattern_match( TAGGED, TAGGED ) ;
  fprintf( log_fp, "seed point 1: tile %lu\n",
                   (unsigned long)(anchor [1]) ) ;
  ASSERT(anchor [0] != anchor [1])
  if(land_amount >= 2) {
    anchor [2] = draw_on_flag_pattern_match( TAGGED, TAGGED ) ;
    fprintf( log_fp, "seed point 2: tile %lu\n",
                   (unsigned long)(anchor [2]) ) ;
  } 
  if(land_amount >= 3) {
    anchor [3] = draw_on_flag_pattern_match( TAGGED, TAGGED ) ;
    fprintf( log_fp, "seed point 3: tile %lu\n",
                   (unsigned long)(anchor [3]) ) ;
  } 
  if(land_amount >= 4) {
    anchor [4] = draw_on_flag_pattern_match( TAGGED, TAGGED ) ;
    fprintf( log_fp, "seed point 4: tile %lu\n",
                   (unsigned long)(anchor [4]) ) ;
  } 
  if(land_amount >= 5) {
    anchor [5] = draw_on_flag_pattern_match( TAGGED, TAGGED ) ;
    fprintf( log_fp, "seed point 5: tile %lu\n",
                   (unsigned long)(anchor [5]) ) ;
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
  /*set ADD_HERE flags for anchor points*/
  for ( i = 0 ; i < 1 + land_amount ; i++ ) { 
    world_flags [anchor [i]] |= (0x01 << i) ;
  }

  /*
  'ongoing' flags:
  .......x   ongoing0   water
  ......x.   ongoing1   land (=desert)
  .....x..   ongoing2
  ....x...   ongoing3
  */
  /*ongoing = 0x0f ;*/
  int lamount = land_amount;
  if(lamount > 3) {
    lamount = land_amount - 3;
    lamount += 1;
  }
  ongoing = 2 << lamount;
  ongoing -= 1 ;
  while (ongoing != 0x0) { /*at least one area is ongoing*/
    for ( i = 0 ; i < 1 + lamount ; i++ ) {
      i_mask = 1 << i ;
      if (ongoing & i_mask) { /*add one tile to area i*/
        if ( add_one_tile( i_mask )) {
          ongoing &= ~i_mask ;
        }
      }
    }
  } /*end while*/

  WORLD_FLAGS_FREE( 0xff )


     /*eliminate one-tile-islands, except for 'arctic'*/
  eliminate_one_tile_islands() ;


     /*add 2 mountain ridges inside desert*/
     /*TAGGED: All tiles inside the desert*/
  WORLD_FLAGS_ALLOC( TAGGED )
  clear_flags(       TAGGED ) ;
     /*world_flags allocation status:*/
     /*- TAGGED       used to tag all land tiles with 'all desert' city radius*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_LAND )) {
      if (count_city_tiles( tile_index, TTL_DESERT ) == 20) {
        world_flags [tile_index] |= TAGGED ;
      }
    }
  }

  WORLD_FLAGS_ALLOC( PREV_TAGGED | TEMP_TAG )
  clear_flags(       PREV_TAGGED | TEMP_TAG ) ;
     /*world_flags allocation status:*/
     /*- TAGGED       used to tag all land tiles with 'all desert' city radius*/
     /*- PREV_TAGGED  used to tag new cluster members*/
     /*- TEMP_TAG     used to tag mountain cluster edges*/
  clus.method = CLUSTER_METHOD_ADJACENT ;
  clus.no_of_clusters = 2 ; /*two mountain ridges*/
  clus.area_mask = TAGGED ;
  clus.area = TAGGED ;
  clus.area_only_for_start_points = FALSE ;
  clus.size = 20 ;
  clus.min_size = 5 ;
  clus.tile_func = clus_mountains ;
  clus.set_flags = PREV_TAGGED ;
  clus.clear_flags = 0x0 ;
  clus.edge_flag = TEMP_TAG ;
  clus.grow_into = TTL_DESERT ;
  clus.task_str = "add mountain ridges" ;
  clus.flag1 = 0x0 ;
  clus.dist1 = 0 ;
  clus.flag2 = 0x0 ;
  clus.dist2 = 0 ;
  add_clusters( & clus ) ;

     /*add some hills (4 clusters) nearby the ridges*/
  clus.no_of_clusters = 4 ;
  clus.area_mask = TEMP_TAG ; /*start at the edge of mountain clusters*/
  clus.area = TEMP_TAG ;
  clus.area_only_for_start_points = TRUE ;
  clus.size = 10 ;
  clus.min_size = 2 ;
  clus.tile_func = clus_hills ;
  clus.set_flags = PREV_TAGGED ;
  clear_flags(     PREV_TAGGED ) ;
  clus.clear_flags = 0x0 ;
  clus.edge_flag = 0x0 ;
  clus.grow_into = TTL_DESERT ;
  clus.task_str = "add hills" ;
  clus.flag1 = 0x0 ;
  clus.dist1 = 0 ;
  clus.flag2 = 0x0 ;
  clus.dist2 = 0 ;
  add_clusters( & clus ) ;

  WORLD_FLAGS_FREE( PREV_TAGGED | TEMP_TAG )
  WORLD_FLAGS_FREE( TAGGED )
     /*world_flags allocation status: none allocated*/


     /*add rivers*/
  fprintf( log_fp, "adding rivers\n" ) ;
  riv.rain_func = rain_on_land_terrain_dependant ;
  riv.visibility = 80 ;
  /*PRT_VAR((unsigned)(riv.visibility),u)*/
  fprintf( log_fp, "riv.visibility = %u\n", (unsigned)(riv.visibility) ) ;
  riv.change_tiles = FALSE ;
  riv.change_mountains = TRUE ;
  riv.new_tile = PRAIRIE ;
  add_rivers( & riv ) ;
  fprintf( log_fp, "rivers added\n" ) ;
     /*some mountains may be changed to prairie here*/



     /*set SHORE flags (Lakes *and* BigRivers)*/
  WORLD_FLAGS_ALLOC( IS_SHORE )
  clear_flags(       IS_SHORE ) ;
     /*world_flags allocation status:*/
     /*- IS_SHORE     used to tag all land tiles with a WATER tile in its neighborhood*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_LAND )) {
      if (count_neighborhood_tiles( tile_index, TTL_WATER ) > 0) {
        world_flags [tile_index] |= IS_SHORE ;
      }
    }
  }

     /*add some GRASSLAND (nearby WATER)*/
  if (grassland_percentage != 0) { /*add grass*/
    log_with_timestamp( "placing grass" ) ;
    no_of_tiles_target = (grassland_percentage * total_no_of_tiles) / 100 ;
    no_of_tiles = 0 ;
    /*PRT_VAR(no_of_tiles_target,u)*/
    fprintf( log_fp, "no_of_tiles_target = %u\n", no_of_tiles_target ) ;

    WORLD_FLAGS_ALLOC( TAGGED | TEMP_TAG )
    clear_flags(       TAGGED | TEMP_TAG ) ;
/*world_flags allocation status:*/
/*- IS_SHORE     used to tag all land tiles with a WATER tile in its neighborhood*/
/*- TAGGED       used to ???  Never set, i. e. all are zero!*/
                /*Experimental result: it affects map reproducability!!*/
                /*ToDo: Investigate it next time I update aux_clus.c*/
/*- TEMP_TAG     used to tag new grassland cluster members*/
    clus.method = CLUSTER_METHOD_CITY_RADIUS ;
    clus.no_of_clusters = 1 ;
    clus.area_mask = (IS_SHORE | TAGGED) ; /*<-- TAGGED used (but never set?)*/
    clus.area = IS_SHORE ;
    clus.area_only_for_start_points = TRUE ; /*may grow away from shore*/
    /*clus.size = xxx ;*/ /*takeover from hills, i. e. 10*/
    /*clus.min_size = grassland_min_size ;*/
    clus.min_size = 1 ;
    clus.tile_func = clus_grass ;
    clus.set_flags = TEMP_TAG ;
    clus.clear_flags = 0x0 ;
    clus.edge_flag = 0x0 ;
    clus.grow_into = TTL_DESERT ;
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
/*world_flags allocation status:*/
/*- IS_SHORE     used to tag all land tiles with a WATER tile in its neighborhood*/
    log_with_timestamp( "grass placed" ) ;
  }

 
    /*add some PRAIRIE (nearby WATER and rivers)*/
     /*Tiles nearby WATER are already tagged with IS_SHORE.*/
     /*Now tag all tiles with RIVER, too.*/
     /*Mountains with rivers (if any) have been turned into prairie.*/
     /*Furthermore, some 'shore deserts' are now grasslands.  So untag*/
     /*all tiles which are not desert any more.*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if ((world [tile_index] & RIVER) == RIVER) {
      world_flags [tile_index] |= IS_SHORE ;
    }
    if ((world [tile_index] & BASIC_TILE_TYPE_MASK) != DESERT) {
      world_flags [tile_index] &= ~IS_SHORE ;
    }
  }
/*world_flags allocation status:*/
/*- IS_SHORE     used to tag all shore or river tiles which are desert*/

  if (prairie_percentage != 0) { /*add prairie*/
    log_with_timestamp( "placing prairie" ) ;
    no_of_tiles_target = (prairie_percentage * total_no_of_tiles) / 100 ;
    no_of_tiles = 0 ;
    /*PRT_VAR(no_of_tiles_target,u)*/
    fprintf( log_fp, "no_of_tiles_target = %u\n", no_of_tiles_target ) ;

    WORLD_FLAGS_ALLOC( TAGGED | TEMP_TAG )
    clear_flags(       TAGGED | TEMP_TAG ) ;
/*world_flags allocation status:*/
/*- IS_SHORE     used to tag all shore or river tiles which are desert*/
/*- TAGGED       used to ???  Never set, i. e. all are zero!*/
/*- TEMP_TAG     used to tag new prairie cluster members*/

    clus.area_only_for_start_points = FALSE ; /*do not place PRAIRIE off rivers*/
    clus.tile_func = clus_prairie ;
    clus.grow_into = TTL_DESERT ;
    clus.task_str = "add prairie" ;

    while (no_of_tiles < no_of_tiles_target) {
      clus.size = random_draw_range( prairie_min_size, prairie_max_size ) ;
      add_clusters( & clus ) ;
      no_of_tiles += clus.actual_size ;
    } /*end of prairie generation*/
    WORLD_FLAGS_FREE( TAGGED | TEMP_TAG )
/*world_flags allocation status:*/
/*- IS_SHORE     used to tag all shore or river tiles which are desert*/
    log_with_timestamp( "prairie placed" ) ;
  }

  WORLD_FLAGS_FREE( IS_SHORE )
/*world_flags allocation status: none allocated*/


     /*convert ocean to coast*/
  set_ocean_and_coast() ;

     /*add bonus resources & plains*/
  set_bonus_resources() ;



     /*add starting positions*/
  fprintf( log_fp, "adding starting positions\n" ) ;
  WORLD_FLAGS_ALLOC( DONT_USE | DONT_SETTLE_DOWN )
  sp.min_rating = startpos_min_rating ;
  sp.number = comp_opponents +1 ;
  sp.city_tag = DONT_USE ;
  sp.city_dist = starting_pos_dist ;
  sp.dont_settle_down = DONT_SETTLE_DOWN ;
  sp.exit_if_failing = TRUE ;
  sp.try_irrigation = TRUE ;
  sp.make_worst_pos_human = TRUE ;
  add_starting_positions( & sp ) ;
  fprintf(log_fp, "starting positions added\n" ) ;

     /*add special resources*/
  fprintf( log_fp, "adding special resources\n" ) ;
  WORLD_FLAGS_ALLOC( TAGGED )
  clear_flags(       TAGGED ) ;

     /*tag all 'deep in the desert' tiles*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (((world_flags [tile_index]) & (DONT_USE | DONT_SETTLE_DOWN)) == 0) {
      if (count_city_tiles( tile_index, TTL_DESERT ) == 20) {
        world_flags [tile_index] |= TAGGED ;
      }
    }
  }

  for ( i = 0 ; i < 12 ; i++ ) {
    tile_index = draw_on_flag_pattern_match( TAGGED, TAGGED ) ;
    if (tile_index == INVALID_TIDX) {
      if (i > 2) { /*essential resources are already placed*/
        /*continue ;*/
        break ;
      }
      fprintf( log_fp, "Can place only %u special resources\n", (unsigned)i ) ;
      printf(          "Can place only %u special resources\n", (unsigned)i ) ;
      map_gen_exit() ;
    }
    world       [tile_index] |= special_resource_tab [i] ;
    world_flags [tile_index] &= ~TAGGED ;
  }
  WORLD_FLAGS_FREE( TAGGED )
  fprintf( log_fp, "special resources added\n" ) ;
  
  WORLD_FLAGS_FREE( DONT_USE | DONT_SETTLE_DOWN )
}

/*-------------------->   add_one_tile   <----------------------- 2015-Jun-06
This function adds one tile to an area.
All world_flags are updated.
-----------------------------------------------------------------------------
Used functions: draw_on_flag_pattern_match,
                tag_neighborhood_tiles, tag_inside_radius
Globals:        world, world_flags
Internals:
Parameters:	- mask    0x01 .. 0x08 (i = 0,1,2,3)
                                            0 - add WATER
                                              1 - add DESERT
Return value:	TRUE if no tile could be added, FALSE else
Exitcode:	--
---------------------------------------------------------------------------*/
static BIT add_one_tile( U8 add_here )
{
/*THIS_FUNC(add_one_tile)*/
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
  if(land_amount <= 3) {
    world [tile_index] = ((add_here != 2) ? DESERT : OCEAN) ;
  } else {
    world [tile_index] = ((add_here != 2 && add_here != 4) ? DESERT : OCEAN) ;
  }
  world_flags [tile_index] |= 0xf0 ; /*add nothing more here*/
  /*tag_neighborhood_tiles( tile_index, add_here ) ;*/
  tag_adjacent_tiles( tile_index, add_here ) ;
  /*tag_city_tiles( tile_index, add_here ) ;*/

  return FALSE ;
}

/*-------------------->   x   <---------------------------------- 2015-Jun-05
This function x
-----------------------------------------------------------------------------
Used functions:
Globals:   x
Internals: x
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
