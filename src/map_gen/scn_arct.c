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
scn_arct.c

This file generates a C-Evo-map file (Scenario: Arctica).
*****************************************************************************
History: (latest change first)
2-16-Aug-17: clean log file implementation
2013-Apr-11..12: Southern land generation with clusters
2013-Apr-08..09: Implemented parameter "MHT_percentage"
2013-Apr-07: Improved placing of harbor cities
2013-Feb-23: default values for "big_island_m??_size"
2012-Sep-25: "add_starting_positions" with STARTPOS_PARAMS
2010-Jun-21: removed call to cevo_lib_init
2010-May-14: Changed code due to new function "rate_1st_pos"
2009-Mar-15..16: Option "no_southern_lands"
2009-Jan-11..17: First version
*****************************************************************************
Global objects:
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include "map_gen.h"
#include "read_ini.h"

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

/*#define MAX_NORTHERN_HARBOR_POSITIONS 30*/


/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

static TILE return_MHT( TIDX index ) ;
static void set_northern_harbors( U8 ocean_tag ) ;
static TIDX set_harbor_city( TIDX idx, TILE new ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/



/*--  internal variables  -------------------------------------------------*/

static TIDX first_mid_index, first_southern_index ; /*stripe boundaries*/


/*-------------------->   scenario_arctic   <-------------------- 2016-Aug-17
Description see "map_gen.ini"
-----------------------------------------------------------------------------
Used functions:
Globals:   x
Internals: first_mid_index, first_southern_index
Parameters:	--
Return value:	--
Exitcode:	x
---------------------------------------------------------------------------*/
void scenario_arctic( void )
{
/*THIS_FUNC(scenario_arctic)*/
char* _this_func = "scenario_arctic" ; /*for WORLD_FLAGS*/
  STARTPOS_PARAMS sp ;
  CLUSTER_PARAMS clus ;
  TIDX tile_index ;
  U32 temp_U32 ; /*scratch pad*/
  U16 no_of_southern_tiles ;
  U16 no_of_land_tiles, no_of_land_tiles_limit ;
  U16 dice ;
  U16 no_of_MHT_tiles, no_of_MHT_tiles_target ;
  U8 water_prob ;
  U8 i, j ; /*loop control*/



     /*default parameters from ini file*/
  if ( ! found_big_island_min_size) {
    big_island_min_size = 2 ;
  }
  if ( ! found_big_island_max_size) {
    big_island_max_size = 20 ;
  }

     /*boundary check for ini file values*/
  PRT_VAR((unsigned)northern_harbors,u)
  PRT_VAR((unsigned)northern_percentage,u)
  PRT_VAR((unsigned)southern_percentage,u)
  if ( northern_percentage + southern_percentage > 99 ) {
    fprintf( log_fp, "northern/southern percentage >=1, sum <99, ok?\n" ) ;
    exit( EXITCODE_WRONG_PARAM ) ;
  }
  if (comp_opponents < 2) {
    fprintf( log_fp,
       "Minimum 2 comp_opponents, to get two special resources\n" ) ; /*ToDo*/
    exit( EXITCODE_WRONG_PARAM ) ;
  }
  if (no_southern_land) {
    if (comp_opponents >= northern_harbors) {
      fprintf( log_fp,
           "northern_harbors must be greater than comp_opponents\n" ) ;
      exit( EXITCODE_WRONG_PARAM ) ;
    }
  }

  first_mid_index = (northern_percentage * total_no_of_tiles) / 100 ;
  first_southern_index =
          (total_no_of_tiles * (100 - southern_percentage)) / 100 ;
  if (first_southern_index > total_no_of_tiles - LX) {
       /*minimum: one ocean line at the bottom*/
    first_southern_index = total_no_of_tiles - LX ;
  }
  no_of_southern_tiles = total_no_of_tiles - first_southern_index ;
  /*PRT_VAR((unsigned)no_of_southern_tiles,u)*/
  fprintf( log_fp, "no_of_southern_tiles = %u\n",
          (unsigned)no_of_southern_tiles ) ;




     /*fill northern stripe with ARCTIC*/
  for ( tile_index = 0 ; tile_index < first_mid_index ; tile_index++ ) {
    world [ tile_index ] = ARCTIC ;
  }
     /*fill southern and mid stripe with OCEAN*/
  for ( tile_index = first_mid_index ;
        tile_index < total_no_of_tiles ; tile_index++ ) {
    world [ tile_index ] = OCEAN ;
  }
  /*TIME_STAMP( "Northern stripe generated" )*/

     /*shelve ice region (mid stripe):
       ratio WATER:ARCTIC = from 0:100 to 100:0*/
  for ( tile_index = first_mid_index ;
        tile_index < first_southern_index ; tile_index++ ) {
    water_prob = (tile_index - first_mid_index )
               * (10000 / (100 - northern_percentage - southern_percentage))
               / total_no_of_tiles ;
    dice = random_draw_range( 1, 100 ) ;
    if (dice > water_prob) {
      world [ tile_index ] = ARCTIC ;
    }
  }
  /*TIME_STAMP( "MID stripe generated" )*/



     /*add some MOUNTAINS, HILLS and TUNDRA in northern and mid stripes*/
  WORLD_FLAGS_ALLOC( TAGGED | PREV_TAGGED )
  clear_flags(       TAGGED | PREV_TAGGED ) ;
  temp_U32 = (first_mid_index + first_southern_index) /2 ;
                              /*use only the north and half the mid stripe*/
  no_of_MHT_tiles_target = (U16)((MHT_percentage * temp_U32) / 100) ;
  /*PRT_VAR((unsigned)no_of_MHT_tiles_target,u)*/
  fprintf( log_fp, "no_of_MHT_tiles_target = %u\n",
          (unsigned)no_of_MHT_tiles_target ) ;
  no_of_MHT_tiles = 0 ;
  for ( tile_index = 0 ; tile_index < temp_U32 ; tile_index++ ) {
    world_flags [tile_index] |= TAGGED ;
  }

  clus.method = CLUSTER_METHOD_ADJACENT ;
  clus.no_of_clusters = 1 ;
  clus.area_mask = TAGGED ;
  clus.area = TAGGED ;
  clus.area_only_for_start_points = FALSE ;
  /*clus.size = ... (random value) ;*/
  clus.min_size = 1 ;
  clus.tile_func = return_MHT ;
  clus.set_flags = PREV_TAGGED ; /*dummy, needed for add_clusters()*/
  clus.clear_flags = 0x0 ;
  clus.edge_flag = 0x0 ;
  clus.grow_into = TTL_WATER | TTL_ARCTIC ;
  clus.task_str = "add MHT tiles" ;
  clus.flag1 = 0x0 ;
  clus.dist1 = 0 ;
  clus.flag2 = 0x0 ;
  clus.dist2 = 0 ;

  while (no_of_MHT_tiles < no_of_MHT_tiles_target) {
    /*clus.size = random_draw_range( 3, 9 ) ;*/
    clus.size = random_draw_range( 3, 7 ) ;
    /*PRT_VAR((unsigned)(clus.size),u)*/
    TIME_STAMP( "adding MHT cluster" )
    add_clusters( & clus ) ;
    no_of_MHT_tiles += clus.actual_size ;
    /*PRT_VAR((unsigned)(clus.actual_size),u)*/
  }
  /*PRT_VAR((unsigned)no_of_MHT_tiles,u)*/
  fprintf( log_fp, "no_of_MHT_tiles = %u\n", (unsigned)no_of_MHT_tiles ) ;
  WORLD_FLAGS_FREE( TAGGED | PREV_TAGGED )
  log_with_timestamp( "MHT tiles added" ) ;


     /*ocean has to be tagged before southern land is generated*/
     /*because someone might set southern_percentage to 100%*/
  WORLD_FLAGS_ALLOC( PREV_TAGGED )
  clear_flags(       PREV_TAGGED ) ;
  ASSERT(world [total_no_of_tiles -1] == OCEAN)
  world_flags [total_no_of_tiles -1] = PREV_TAGGED ;
                                            /*anchor point for tagging*/
  tag_whole_ocean( PREV_TAGGED ) ;


     /*southern land for computer opponents*/
  log_with_timestamp( "begin southern land generation" ) ;
  no_of_land_tiles_limit = (U16)(
      /*((U32)land_percentage * (U32)no_of_southern_tiles) / 100*/
      ((U32)land_percentage * (U32)no_of_southern_tiles) / 134
         /*only 3/4 of the southern stripe is used for land!*/
                                ) ;
  if (no_southern_land) {
    no_of_land_tiles_limit = 0 ;
  }
  /*PRT_VAR((unsigned)no_of_land_tiles_limit,u)*/
  fprintf( log_fp, "no_of_land_tiles_limit = %u\n",
          (unsigned)no_of_land_tiles_limit ) ;
  no_of_land_tiles = 0 ;

     /*no land in northern and mid stripe*/
     /*use only three quarters of southern stripe for land*/
  WORLD_FLAGS_ALLOC( NO_BIG_ISLAND_HERE )
  clear_flags( NO_BIG_ISLAND_HERE ) ;
  for ( tile_index = 0 ;
        tile_index < first_southern_index + (no_of_southern_tiles / 4) ;
        tile_index++ ) {
    world_flags [ tile_index ] |= NO_BIG_ISLAND_HERE ;
  }

  WORLD_FLAGS_ALLOC( TAGGED )
  clear_flags(       TAGGED ) ;
  clus.method = CLUSTER_METHOD_ISLAND ;
  clus.no_of_clusters = 1 ;
  clus.area_mask = NO_BIG_ISLAND_HERE ;
  clus.area = 0x0 ;
  clus.area_only_for_start_points = TRUE ;
  clus.min_size = 1 ;
  clus.tile_func = draw_land_tile ;
  clus.set_flags = TAGGED ;
  clus.clear_flags = 0x0 ;
  clus.edge_flag = 0x0 ;
  clus.grow_into = TTL_WATER ;
  clus.task_str = "add southern land" ;
  while (no_of_land_tiles < no_of_land_tiles_limit) {
    clus.size = random_draw_range( big_island_min_size,
                                   big_island_max_size ) ;
    log_with_timestamp( "adding southern cluster" ) ;
    add_clusters( & clus ) ;
    no_of_land_tiles += clus.actual_size ;
  }
  WORLD_FLAGS_FREE( TAGGED )
  WORLD_FLAGS_FREE( NO_BIG_ISLAND_HERE )
  /*PRT_VAR((unsigned)no_of_land_tiles,u)*/
  fprintf( log_fp, "no_of_land_tiles = %u\n",
          (unsigned)no_of_land_tiles ) ;
  log_with_timestamp( "Southern land generated" ) ;


     /*eliminate one-tile-islands, except for 'arctic'*/
  eliminate_one_tile_islands() ;

     /*add rivers*/
  /*add_rivers() ;*/

     /*convert ocean to coast*/
  set_ocean_and_coast() ;

     /*add bonus resources & plains*/
  set_bonus_resources() ;
  /*TIME_STAMP( "Bonus resources set" )*/



     /*add starting positions resp. TUNDRA*/
  WORLD_FLAGS_ALLOC( DONT_SETTLE_DOWN )
  clear_flags(       DONT_SETTLE_DOWN ) ;
  set_northern_harbors( PREV_TAGGED ) ;
  WORLD_FLAGS_FREE( PREV_TAGGED )
     /*Northern harbors must be set before southern land starting positions*/
     /*because southern land starting position placement tags*/
     /*whole north & mid stripe with DONT_SETTLE_DOWN*/


     /*add one resp. three special resource in northern stripe*/
  WORLD_FLAGS_ALLOC( TAGGED )
  j = no_southern_land ? 3 : 1 ;
  for ( i = 0 ; i < j ; i++) {
    clear_flags( TAGGED ) ;
    for ( tile_index = 0 ; tile_index < first_mid_index ; tile_index++ ) {
      if (count_city_tiles( tile_index, TTL_ARCTIC ) == 20){
        world_flags [ tile_index ] |= TAGGED ;
      }
    }
    tile_index = draw_on_flag_pattern_match(
                         TAGGED | DONT_SETTLE_DOWN, TAGGED ) ;
    fprintf( log_fp, "City center tile for special resource: %lu\n",
                 (unsigned long)tile_index ) ;
    if (tile_index == INVALID_TIDX) {
      fprintf( log_fp, "cannot place northern special resource(s)\n" ) ;
       printf(         "cannot place northern special resource(s)\n" ) ;
      map_gen_exit() ;
    }
    world [tile_index] = HILLS ;
    set_special_resource( tile_index, special_resource_tab [i] ) ;
    tag_city_tiles( tile_index, DONT_SETTLE_DOWN ) ;
  }
  WORLD_FLAGS_FREE( TAGGED )
  log_with_timestamp( "Northern special resources set" ) ;
  set_bonus_resources() ;

     /*add starting positions for computer players in southern stripe*/
  if ( ! no_southern_land) {
    for ( tile_index = 0 ;
          tile_index < first_southern_index + (no_of_southern_tiles / 4) ;
          tile_index++ ) {
         /*force southern stripe positions even in the unlike case that*/
         /*there are super-duper positions way up north*/
      world_flags [ tile_index ] |= DONT_SETTLE_DOWN ;
    }
    sp.min_rating = startpos_min_rating ;
    sp.number = comp_opponents ;
    sp.city_tag = 0x0 ;
    sp.city_dist = starting_pos_dist ;
    sp.dont_settle_down = DONT_SETTLE_DOWN ;
    sp.exit_if_failing = TRUE ;
    sp.try_irrigation = FALSE ;
    sp.make_worst_pos_human = FALSE ;
    add_starting_positions( & sp ) ;
  }
  /*TIME_STAMP( "Southern start positions set" )*/


     /*add two special resources at the two best computer start positions*/
  /*PRT_VAR(start_pos_idx [1],lu)*/
  /*PRT_VAR(start_pos_idx [2],lu)*/
  if ( ! no_southern_land) {
    set_special_resource( start_pos_idx [1], special_resource_tab [1] ) ;
    set_special_resource( start_pos_idx [2], special_resource_tab [2] ) ;
    TIME_STAMP( "Southern special resources set" )
  }
  set_bonus_resources() ;
}

/*-------------------->   return_MHT   <------------------------- 2013-Apr-08
This function returns a Mountain, Hill or Tundra tile.
It is used for add_clusters().
-----------------------------------------------------------------------------
Used functions: random_draw_range
Globals:   x
Internals: x
Parameters:     - index    (not used here)
Return value:   MOUNTAIN (40%), HILL (20%) or TUNDRA (40%)
Exitcode:       --
---------------------------------------------------------------------------*/
static TILE return_MHT( TIDX index )
{
THIS_FUNC(return_MHT)
  U16 dice ;


  /*PRT_VAR((unsigned long)index,lu)*/
  /*PRT_VAR((unsigned long)(world [index]),08lx)*/
  ASSERT(index < total_no_of_tiles)
  ASSERT_ALT(is_in_TTL( index, TTL_ARCTIC | TTL_WATER ),
             PRT_VAR((unsigned long)index,lu)
             PRT_VAR((unsigned long)(world [index]),lx)
            )

     /*40% MOUNTAINS, 20% HILLS, 40% TUNDRA*/
  dice = random_draw_range( 1, 100 ) ;
  if (dice < 41) {
    /*DEB((stderr, "returning MOUNTAIN\n"))*/
    return MOUNTAINS ;
  }
  else if (dice < 61) {
    /*DEB((stderr, "returning HILLS\n"))*/
    return HILLS ;
  }
  else {
    /*DEB((stderr, "returning TUNDRA\n"))*/
    return TUNDRA ;
  }
}

/*-------------------->   set_northern_harbors   <--------------- 2016-Aug-17
This function identifies "northern_harbors" positions (WATER tiles)
and scans the neigborhood for suitable city positions.
The first harbor is made the human players starting position,
the other positions are either converted to TUNDRA or made computer player
starting positions.

The function respects existing DONT_SETTLE_DOWN flags.
It sets the DONT_SETTLE_DOWN flags in a radius of "starting_pos_dist"
around each city it drops.

All OCEAN tiles must be tagged with "ocean_tag" when this function is called.
-----------------------------------------------------------------------------
Used functions:
Globals:   TAGGED, no_southern_land, northern_harbors  r/o
           DONT_SETTLE_DOWN  r/w
Internals: harbor_pos []
           first_southern_index
Parameters:     - ocean_tag  the whole ocean must be tagged with this mask
Return value:   void
Exitcode:       x
---------------------------------------------------------------------------*/
static void set_northern_harbors( U8 ocean_tag )
{
THIS_FUNC(set_northern_harbors)
  TIDX tile_index ;
  TILE new_tile ;
  U8 i ; /*loop control*/
  TIDX found ;


  ASSERT((ocean_tag & DONT_SETTLE_DOWN) == 0)

  for( i = 0 ; i < northern_harbors ; i++ ) {
    found = INVALID_TIDX ; /*default*/
    for ( tile_index = 0 ; tile_index < first_southern_index ; tile_index++ ) {
      /*DEB((stderr, "scanning tile %lu, world flags 0x%02x\n",*/
                   /*(unsigned long)tile_index,*/
                   /*(unsigned)(world_flags [tile_index])))*/
      if (((world_flags [tile_index]) & (ocean_tag | DONT_SETTLE_DOWN))
           == ocean_tag) {
        /*DEB((stderr, "testing tile %lu\n", (unsigned long)tile_index))*/
        new_tile = no_southern_land ?  GRASSLAND | NORMAL_STARTPOS : TUNDRA ;
        if (i > comp_opponents) {
          new_tile = TUNDRA ;
        }
        if (i == 0) {
          new_tile = GRASSLAND | HUMAN_STARTPOS ;
        }
        found = set_harbor_city( tile_index, new_tile ) ;
        if (found != INVALID_TIDX) {
          fprintf( log_fp, "Northern Harbor city at loc. code %lu\n",
                        (unsigned long)found ) ;
          tag_inside_radius( found, starting_pos_dist, DONT_SETTLE_DOWN ) ;
          break ;
        }
      }
    }
    /*PRT_VAR((unsigned long)found,lx)*/
    if (found == INVALID_TIDX) {
      fprintf( log_fp, "could place only %u northern harbors\n", (unsigned)i ) ;
       printf(         "could place only %u northern harbors\n", (unsigned)i ) ;
      map_gen_exit() ;
    }
  }
  log_with_timestamp( "Harbor positions set" ) ;
}

/*-------------------->   set_harbor_city   <-------------------- 2013-Apr-07
This function tests the NEIGHBORHOOD of one WATER tile position for
suitable city positions.
It chooses the best one and replaces the tile with "new".
-----------------------------------------------------------------------------
Used functions: set_ocean_and_coast, set_bonus_resources,
                set_neighborhood_tiles, is_land_tile, rate_1st_pos
Globals:   world []
Internals: --
Parameters:     - idx    Tile index of WATER tile
                - new    TILE to be placed (TUNDRA or GRASSLAND | *_STARTPOS)
Return value:   tile index of changed tile or INVALID_TIDX if not successful
Exitcode:       --
---------------------------------------------------------------------------*/
static TIDX set_harbor_city( TIDX idx, TILE new )
{
THIS_FUNC(set_harbor_city)
  NEIGHBORHOOD N ;
  NEIGHBORHOOD* Np ;
  TIDX* idxp ;
  TIDX best_index ;
  TILE org_tile ;
  U16 rating, best_rating ;
  U8 j ; /*loop control*/


    /*PRT_VAR((unsigned long)idx,lu)*/
    Np = set_neighborhood_tiles( idx, & N ) ;
    best_rating = 0 ;
    best_index = -1 ;
    for( j = 0, idxp = & (Np->n_idx) ; j < 8 ; j++, idxp++ ) {
      /*PRT_VAR((unsigned)j,u)*/
      if (is_land_tile( *idxp )) {
        org_tile = world [*idxp] ;
        world [*idxp] = GRASSLAND ;
        set_ocean_and_coast() ; /*ARCTIC->GRASSLAND => OCEAN->COAST*/
           /*all COASTs set new => all WATER bonus resources erased!*/
        set_bonus_resources() ; /*new COAST->new fish/manganese*/
        rating = rate_1st_pos( *idxp ) ;
        /*DEB((stderr, "Rating for loc. %lu: %u\n",*/
                    /*(unsigned long)(*idxp), (unsigned)rating))*/
        if (rating > best_rating) {
          best_rating = rating ;
          best_index = *idxp ;
        }
        world [*idxp] = org_tile ; /*restore original tile*/
      }
    } /*end loop 'around' harbor place*/

    if (best_rating == 0) {
      set_ocean_and_coast() ; /*set according to final situation*/
      set_bonus_resources() ;
      return INVALID_TIDX ;
    }

    world [best_index] = new ;
    set_ocean_and_coast() ; /*set according to final situation*/
    set_bonus_resources() ;
    return best_index ;
}

/*-------------------->   x   <---------------------------------- 2013-Apr-08
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
