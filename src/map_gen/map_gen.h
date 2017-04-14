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
map_gen.h

Include file for map_gen.c
*****************************************************************************
History: (latest change first)
2017-Feb-27: changed include pathes for source code release
2016-Aug-08: added "log_fp", "log_with_timestamp()"
2016-Jul-15: adoptions for gcc
2015-Aug-01: - added scenario "Fjords"
             - "erosion" made global
2015-Jun-06: added "clus_prairie"
2015-Jun-05: - extended RIVER_PARAMS
             - added "clus_hills" & "rain_on_land_terrain_dependant"
2015-Jun-04: made "clus_*" public
2015-Jun-03: added scenario "Desert"
2015-Feb-13: added scenario "Volcano Islands"
2014-Oct-08: added debug function "print_pv"
2013-Apr-12: added "draw_land_tile"
2012-Sep-25: introduction of STARTPOS_PARAMS
2012-Sep-24: introduction of RIVER_PARAMS and "map_size_idx"
2012-Sep-22: RUGGED->RAGGED
2012-Sep-21: added entries for "clusters.c"
2012-Jun-08: changed args of "add_rivers"
2012-Jun-06: - added scenario "Great Plains"
             - moved func prototypes for "map_gen" from "cevo_map.h" here
2012-Feb-13: added scenario "big_river"
2012-Feb-10..12: - added functions to avoid warnings
                 - renamed old scenario to english names
2010-May-09: nested include "random.h"
2010-Apr-18: added more external scenario functions
2009-Jan-11..17: added more externals
2008-Jun-01: initial version
****************************************************************************/
#ifndef MAP_GEN_H
#define MAP_GEN_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

#include <stdio.h>
#ifdef unix
# include "../cevo_lib/cevo_map.h"
#else
# include "..\cevo_lib\cevo_map.h"
#endif
#include <random.h>


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  typedefs & enums  ---------------------------------------------------*/

   /*a structure for environment-dependent tile selection*/
typedef struct {
  U32 water ;
  U32 grassland ;
  U32 desert ;
  U32 prairie ;
  U32 tundra ;
  U32 arctic ;
  U32 swamp ;
  U32 forest ;
  U32 hills ;
  U32 mountains ;
} PROBABILITY_VECTOR ;


typedef TILE (*DETERMINE_TILE)( TIDX tidx ) ;

   /*The cluster method determines the way a cluster grows,*/
   /*i. e. the way new tiles are chosen to be added to the cluster*/
typedef enum {
  CLUSTER_METHOD_ADJACENT,
  CLUSTER_METHOD_NEIGHBORHOOD,
  CLUSTER_METHOD_CITY_RADIUS, /*'scattered' cluster*/
  CLUSTER_METHOD_LAKE_SMOOTH,
  CLUSTER_METHOD_LAKE_RAGGED,
  CLUSTER_METHOD_ISLAND
} CLUSTER_METHOD ;

   /*CLUSTER_PARAMS control the generation of one ore more clusters*/
typedef struct {
  CLUSTER_METHOD method ; /*defines the style of the cluster*/
  U16 no_of_clusters ; /* = number of start points*/

     /*these two params are currently not used*/
  /*U16 start_point_distance ;*/
  /*U8 start_point_flag ;*/

     /*the following three params limit the parts of the map*/
     /*where clusters may be generated (using "world_flags")*/
  U8 area_mask ; /*mask for the following param*/
  U8 area ; /*defines an allowed area for adding tiles
              (setting both to 0x00 allows the whole map)*/
  BIT area_only_for_start_points ; /*If TRUE, only the start point of
     a cluster must be inside the allowed area, but added tiles
     may be outside*/
     /*If FALSE, the starting point as well as all added tiles must
     be inside the area*/

  U16 size ; /*for a single cluster: size
               for clusters: cumulative size*/
  U16 min_size ; /*if no more tiles can be added but min_size is reached,
                   return without error*/

     /*for function "add_tile_to_cluster": a caller-supplied function
       which determines the tile type being generated (desert, water, ...
        or even an environment dependent tile type selection)*/
  DETERMINE_TILE tile_func ;

     /*Some world_flags set/cleared for every new tile added to the cluster.*/
     /*They can be used for further map processing because all cluster tiles*/
     /*are tagged with these flags.*/
     /*Set to 0x00 if you don't want to use them.  CAUTION: You must set
       at least one bit in "set_flags" so that new member tiles can be marked*/
  U8 set_flags ; /*new tiles are tagged with this/these flag(s)*/
  U8 clear_flags ;
  U8 edge_flag ; /*tag for every tile outside the clusters
                   but in its neighborhood*/

  TTL grow_into ; /*Tile Type List of tiles the cluster may grow into*/

     /*for error messages, fill with description what cluster is being generated*/
  char* task_str ; /*e. g. "add lakes"*/

     /*after placing all clusters, tag surrounding tiles and clusters*/
  U8 flag1 ;
  U8 flag2 ;
  U16 dist1 ;
  U16 dist2 ;

     /*return value(s)*/
  U16 actual_size ; /*min_size <= actual_size <= size*/

     /*private members*/
  U8 tag_mask ; /*cluster members can be identified by these flags*/
  U8 clear_all ; /*shortcut: clear_flags ORed with edge_flag*/
} CLUSTER_PARAMS ;


typedef U16 (*RAIN_FUNC)( TIDX tidx ) ;

   /*RIVER_PARAMS control how rivers are added to the map*/
typedef struct {
  RAIN_FUNC rain_func ; /*a function which determines how much rain falls
                          on every land tile.  Examples:
                          - uniform distribution
                          - more rain around mountains, less in deserts*/
  U16 visibility ; /* visibility: if erosion >= visibility,
                      a river is (might be) drawn on that tile*/
  BIT change_tiles ; /*for the 'Mountains' scenario: change tiles with rivers
                      to Grasslands, Hills, Forest*/

     /*the following two params have been added for the 'Desert' scenario*/
  BIT change_mountains ; /*MOUNTAINS can't have rivers.  If a river is to
            be added to a MOUNTAINS tile, change the basic tile type to
            "new_tile"*/
  TILE new_tile ;
} RIVER_PARAMS ;


   /*STARTPOS_PARAMS control the placement of the players' first cities*/
typedef struct {
  U8 min_rating ; /*minimum rating for a tile to qualify as starting position*/
  U8 number ; /*... of start positions to be generated*/
  U8 city_tag ; /*city tiles are tagged with this mask*/
  U16 city_dist ; /*tagged with "dont_settle_down" around each city
                    to prevent other cities placed too near*/
  U8 dont_settle_down ; /*tiles in "city_dist" are tagged with this mask*/
  BIT exit_if_failing ; /*... to place the required number of cities, else return*/
  BIT try_irrigation ;
  BIT make_worst_pos_human ; /*... to make it a little bit harder
                               for the human player*/

     /*return value(s)*/
  U8 actual_number ; /*number of cities the algorithm could place*/
} STARTPOS_PARAMS ;


/*--  function prototypes  ------------------------------------------------*/

void scenario_nav_required( void ) ;
void scenario_hard_fight( void ) ;
void scenario_arctic( void ) ;
void scenario_micronesia( void ) ;
void scenario_mountains( void ) ;
void scenario_desert( void ) ;
void scenario_big_river( void ) ;
void scenario_great_plains( void ) ;
void scenario_volcano_islands( void ) ;
void scenario_fjords( void ) ;
/*void scenario_norddeutsche_tiefebene( void ) ;*/

void eliminate_one_tile_islands( void ) ;
void set_ocean_and_coast( void ) ;
void set_bonus_resources( void ) ;

TIDX add_clusters( CLUSTER_PARAMS* params ) ;
TILE draw_land_tile( TIDX idx ) ; /*for add_clusters()*/
TILE clus_ocean( TIDX idx ) ; /*for add_clusters()*/
TILE clus_mountains( TIDX idx ) ; /*for add_clusters()*/
TILE clus_hills( TIDX idx ) ; /*for add_clusters()*/
TILE clus_grass( TIDX idx ) ; /*for add_clusters()*/
TILE clus_prairie( TIDX idx ) ; /*for add_clusters()*/
TILE clus_forest( TIDX idx ) ; /*for add_clusters()*/

TIDX add_island( U16 size, U16 min_size, U8 mask,
                 U16 big_island_dist, U16 small_island_dist,
                 U32 bp_desert, U32 bp_prairie, U32 bp_tundra ) ;

TIDX add_lake( U16 size, U16 dist, U8 shore_mask, U8 mode ) ;
#define RAGGED 0
#define SMOOTH 1

void add_rivers( RIVER_PARAMS* params ) ;
U16 rain_on_land_uniform( TIDX tile_index ) ;
U16 rain_on_land_terrain_dependant( TIDX tile_index ) ;

TILE draw_island_tile( TIDX idx,
               U32 bp_desert, U32 bp_prairie, U32 bp_tundra ) ;
TILE draw_tile( PROBABILITY_VECTOR* v ) ;


TIDX draw_on_flag_pattern_match( U8 mask, U8 pattern ) ;
void add_starting_positions( STARTPOS_PARAMS* params ) ;

void write_map_to_file( char* filename ) ;
void log_with_timestamp( char* msg ) ;
void map_gen_exit( void ) ;

#ifdef DEBUG
void print_pv( PROBABILITY_VECTOR* pv ) ;
#endif /*DEBUG*/


/*--  macros  -------------------------------------------------------------*/

/*--  global variables  ---------------------------------------------------*/

extern int map_size_idx ; /*0 for 35%, 1 for 50%, ... , 5 for 230%*/
      /*map_size_idx can be used to select parameters dependent on map size*/

extern TILE start_pos_mask [] ;
extern TIDX start_pos_idx  [] ;
extern TILE special_resource_tab [] ;

extern U16* erosion ;

extern FILE* log_fp ;


/***************************************************************************/
#endif	/* MAP_GEN_H */
/***************************************************************************/
