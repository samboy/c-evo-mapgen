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
aux_clus.c

This file generates clusters for a C-Evo map.
Several different methods of cluster shaping are available.
*****************************************************************************
History: (latest change first)
2017-Feb-19: - clean log file implementation
             - better markers in "debug.cevo map" (dead lands)
2015-Jun-06: added "clus_prairie"
2015-Jun-05: added "clus_hills"
2015-Jun-04: More detailed error message
2013-Apr-08: Debugging
2012-Sep-19..24: - still initial version
                 - renamed   clusters.c -> aux_clus.c
2012-Aug-23..26: still initial version
2012-Feb-13..14: Derived from "islands.c"
*****************************************************************************
Global objects:
- TIDX add_clusters( CLUSTER_PARAMS* params )
- TILE draw_land_tile( TIDX tidx )
- TILE clus_ocean( TIDX tidx )
- TILE clus_mountains( TIDX tidx )
- TILE clus_hills( TIDX tidx )
- TILE clus_grass( TIDX tidx )
- TILE clus_prairie( TIDX tidx )
- TILE clus_forest( TIDX tidx )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include "map_gen.h"
#include "read_ini.h" /*for global definitions*/

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/


/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

static void add_tile_to_cluster( TIDX tile_idx, CLUSTER_PARAMS* params ) ;
static BIT may_grow_into( TIDX tidx, CLUSTER_PARAMS* params ) ;


/*--  macros  -------------------------------------------------------------*/

#define IN_CLUSTER(index)  \
     (((world_flags [ index ]) & (params->tag_mask)) == (params->set_flags))

#define NOT_IN_CLUSTER(index)  \
     (((world_flags [ index ]) & (params->tag_mask)) != (params->set_flags))


/*--  global variables  ---------------------------------------------------*/



/*--  internal variables  -------------------------------------------------*/


/*-------------------->   add_clusters   <----------------------- 2017-Feb-19
This function adds one or more clusters, according to the CLUSTER_PARAMS.
-----------------------------------------------------------------------------
Used functions:
Globals:   world_flags [ADD_HERE_ALL]
Internals:
Parameters:	- params    The parameter set
Return value:	Tile index of  *first*  cluster
Exitcode:	EXITCODE_WRONG_PARAM
---------------------------------------------------------------------------*/
TIDX add_clusters( CLUSTER_PARAMS* params )
{
THIS_FUNC(add_clusters)
char* _this_func = "add_clusters" ; /*for WORLD_FLAGS_ALLOC*/
  TIDX tidx ;
  TIDX ret_val ;
  U16 i ; /*loop control*/
  U16 tiles_in_clusters = 0 ;
  U16 dist ; /*scratch pad for distance*/


  /*ENTRY*/
  /*TRACE_ON*/
  /*PRT_VAR(params->task_str,s)*/
  /*TRACE_OFF*/
     /*do some checks on CLUSTER_PARAMS*/
  ASSERT(params->set_flags != 0x0)
       /*You must set at least one flag so that add_clusters() knows
         which tiles it has placed*/
  if (params->size == 0) {
    fprintf( log_fp, "add_clusters: size = 0\n" ) ;
    map_gen_exit() ;
  }
  if (params->min_size == 0) {
    fprintf( log_fp, "add_clusters: min_size = 0\n" ) ;
    map_gen_exit() ;
  }



  WORLD_FLAGS_ALLOC( ADD_HERE_ALL )
  clear_flags(       ADD_HERE_ALL ) ;

     /*establish some shortcuts*/
  params->clear_all = (params->clear_flags) | (params->edge_flag)
                                            | ADD_HERE_ALL ;
  params->tag_mask =  (params->set_flags) | (params->clear_flags) ;
  ASSERT((params->area_mask & params->tag_mask) == 0x0)
  /*PRT_VAR(params->task_str,s)*/
  /*PRT_VAR(params->clear_all,02x)*/
  /*PRT_VAR(params->tag_mask,02x)*/
  /*PRT_VAR(params->set_flags,02x)*/
  /*PRT_VAR(params->grow_into,08x)*/

     /*start the new cluster(s) where world_flags match*/
  for ( i = 0 ; i < params->no_of_clusters ; i++ ) {
    /*tidx = draw_on_flag_pattern_match( params->area_mask, params->area ) ;*/
          /*AND  NOT  YET  IN  CLUSTER !!!*/
    tidx = draw_on_flag_pattern_match(
               params->area_mask | params->set_flags, params->area ) ;
    /*fprintf( stderr, "add_clusters: start point=%u\n", (unsigned)tidx ) ;*/
    if (i == 0) {
      ret_val = tidx ;
      /*PRT_VAR((unsigned)ret_val,u)*/
    }
    if (tidx == INVALID_TIDX) {
      fprintf( log_fp, "WARNING %s: can add only %u start points\n",
                       params->task_str, (unsigned)i ) ;
      fprintf( stderr, "WARNING %s: can add only %u start points\n",
                       params->task_str, (unsigned)i ) ;
      break;
    }
    add_tile_to_cluster( tidx, params ) ;
    tiles_in_clusters++ ;
  }

     /*let the clusters grow around the start points*/
  while (tiles_in_clusters < params->size) {

    tidx = INVALID_TIDX ; /*default*/
    switch (params->method) {

    case CLUSTER_METHOD_ADJACENT:
    case CLUSTER_METHOD_NEIGHBORHOOD:
    case CLUSTER_METHOD_CITY_RADIUS:
      tidx = draw_on_flag_pattern_match( ADD_HERE0, ADD_HERE0 ) ;
      break ;

    case CLUSTER_METHOD_LAKE_SMOOTH: /*try high prio tiles first*/
      tidx = draw_on_flag_pattern_match( ADD_HERE2, ADD_HERE2 ) ;
         /*fall thru to case CLUSTER_METHOD_LAKE_RAGGED*/

    case CLUSTER_METHOD_LAKE_RAGGED:
         /*falling thru from CLUSTER_METHOD_LAKE_SMOOTH*/
      if (tidx == INVALID_TIDX) { /*no tile with highest prio found*/
        tidx = draw_on_flag_pattern_match( ADD_HERE1, ADD_HERE1 ) ;/*prio*/
        if ((tidx == INVALID_TIDX) || (random_draw_range( 1, 100 ) > 50)) {
          tidx = draw_on_flag_pattern_match( ADD_HERE0, ADD_HERE0 ) ;
        }
      }
      break ;

    case CLUSTER_METHOD_ISLAND:
      tidx = draw_on_flag_pattern_match( /* 7 = 1 1 1*/
          ADD_LAND_HERE_ALL, ADD_LAND_HERE_ALL ) ;
      if (tidx == INVALID_TIDX) { /*no tile found!*/
        tidx = draw_on_flag_pattern_match( /* 6 = 1 1 0*/
          ADD_LAND_HERE_ALL, ADD_LAND_HERE2 | ADD_LAND_HERE1 ) ;
      }
      if (tidx == INVALID_TIDX) { /*no tile found!*/
        tidx = draw_on_flag_pattern_match( /* 5 = 1 0 1*/
          ADD_LAND_HERE_ALL, ADD_LAND_HERE2 | ADD_LAND_HERE0 ) ;
      }
      if (tidx == INVALID_TIDX) { /*no tile found!*/
        tidx = draw_on_flag_pattern_match( /* 4 = 1 0 0*/
          ADD_LAND_HERE_ALL, ADD_LAND_HERE2 ) ;
      }
      if (tidx == INVALID_TIDX) { /*no tile found!*/
        tidx = draw_on_flag_pattern_match( /* 3 = 0 1 1*/
          ADD_LAND_HERE_ALL, ADD_LAND_HERE1 | ADD_LAND_HERE0 ) ;
      }
      if (tidx == INVALID_TIDX) { /*no tile found!*/
        tidx = draw_on_flag_pattern_match( /* 2 = 0 1 0*/
          ADD_LAND_HERE_ALL, ADD_LAND_HERE1 ) ;
      }
      if (tidx == INVALID_TIDX) { /*no tile found!*/
        tidx = draw_on_flag_pattern_match( /* 1 = 0 0 1*/
          ADD_LAND_HERE_ALL, ADD_LAND_HERE0 ) ;
      }
      break ;

    default:
      fprintf( log_fp, "add_clusters: invalid method\n" ) ;
      map_gen_exit() ;
    } /*end switch params->method*/


    if (tidx == INVALID_TIDX) {
      if (tiles_in_clusters >= params->min_size) {
        break ; /*min. size reached, go on*/
      }
      else {
        ASSERT(params->task_str != NULL)
        fprintf( log_fp, "%s: cannot find a tile to add\n",
                         params->task_str ) ;
        map_gen_exit() ;
      }
    }
    add_tile_to_cluster( tidx, params ) ;
    tiles_in_clusters++ ;

  } /*end while*/
  params->actual_size = tiles_in_clusters ;

  WORLD_FLAGS_FREE( ADD_HERE_ALL )


     /*mark surroundings with flag1/flag2*/
  for ( tidx = 0 ; tidx < total_no_of_tiles -1 ; tidx++ ) {
    dist = dist_tile_to_group( tidx, params->tag_mask, params->set_flags ) ;
    if (dist < params->dist1 ) {
      world_flags [ tidx ] |= params->flag1 ;
    }
    if (dist < params->dist2 ) {
      world_flags [ tidx ] |= params->flag2 ;
    }
  }
  return ret_val ;
}

/*-------------------->   add_tile_to_cluster   <---------------- 2012-Sep-21
This function adds one tile at a given index to a cluster.
All flags are updated, of the tile as well as in the neighborhood.
-----------------------------------------------------------------------------
Used functions: set_neighborhood_tiles, *tile_func, may_grow_into
Globals:        world, world_flags
Internals:
Parameters:	- x
Return value:	void
Exitcode:	EXITCODE_WRONG_PARAM
---------------------------------------------------------------------------*/
static void add_tile_to_cluster( TIDX tile_idx, CLUSTER_PARAMS* params )
{
/*THIS_FUNC(add_tile_to_cluster)*/
  NEIGHBORHOOD  N ;
  NEIGHBORHOOD* Np ;
  CITY_TILES  CT ;
  CITY_TILES* CTp ;
  TIDX* tile_idxp ;
  TIDX  N_index ;
  U8 i ; /*loop control*/
  U8 prio_counter ;


  /*if (tile_idx == 2090)  TRACE_ON*/
  /*PRT_VAR((unsigned)tile_idx,u)*/
  /*PRT_VAR((unsigned long)(world [tile_idx]),08lx)*/
  Np = set_neighborhood_tiles( tile_idx, & N ) ;

     /*set tile and tile flags*/
  world_flags [ tile_idx ] |=    params->set_flags ;
  world_flags [ tile_idx ] &=  ~(params->clear_all) ;
  world       [ tile_idx ]  = (*(params->tile_func))( tile_idx ) ;
  /*PRT_VAR((unsigned long)(world [tile_idx]),08lx)*/
  /*PRT_VAR((unsigned)(world_flags [tile_idx]),02x)*/

     /*set edge flags*/
  if (params->edge_flag != 0x0) {
    for ( i = 0, tile_idxp = & (Np->n_idx) ; i < 8 ; i++, tile_idxp++ ) {
      N_index = *tile_idxp ;
      if (N_index < total_no_of_tiles) { /*inside map*/
        if ((world_flags [ N_index ] & (params->tag_mask))
                          != (params->set_flags)) { /*not (yet) in cluster*/
          world_flags [ N_index ] |= params->edge_flag ;
        }
      }
    }
  }

  switch (params->method) {

  case CLUSTER_METHOD_ADJACENT:                    /*scan adjacent tiles*/
    for ( i = 1, tile_idxp = & (Np->ne_idx) ; i < 8 ; i += 2, tile_idxp += 2 ){
      N_index = *tile_idxp ;
      if (may_grow_into( N_index, params )) {
        world_flags [N_index] |= ADD_HERE0 ;
      }
    }
    break ;

  case CLUSTER_METHOD_NEIGHBORHOOD:               /*scan neighborhood tiles*/
    for ( i = 0, tile_idxp = & (Np->n_idx) ; i < 8 ; i++, tile_idxp++ ) {
      N_index = *tile_idxp ;
      if (may_grow_into( N_index, params )) {
        world_flags [N_index] |= ADD_HERE0 ;
      }
    }
    break ;

  case CLUSTER_METHOD_CITY_RADIUS:                /*scan city tiles*/
    CTp = set_city_tiles( tile_idx, & CT ) ;
    for ( i = 0, tile_idxp = & (CTp->i1) ; i < 20 ; i++, tile_idxp++ ) {
      N_index = *tile_idxp ;
      if (may_grow_into( N_index, params )) {
        world_flags [N_index] |= ADD_HERE0 ;
      }
    }
    break ;

  case CLUSTER_METHOD_LAKE_SMOOTH:                /*scan neighborhood tiles*/
  case CLUSTER_METHOD_LAKE_RAGGED:
    for ( i = 0, tile_idxp = & (Np->n_idx) ; i < 8 ; i++, tile_idxp++ ) {
      N_index = *tile_idxp ;
      if (may_grow_into( N_index, params )) {

        world_flags [ N_index ] |= ADD_HERE0 ;
        if (count_adjacent_tiles( N_index, TTL_WATER ) == 4) { /*prio*/
          world_flags [ N_index ] |= ADD_HERE1 ;
        }
        if (count_neighborhood_tiles( N_index, TTL_WATER ) > 4) {
             /*even higher prio*/
          world_flags [ N_index ] |= ADD_HERE2 ;
        }

      }
    } /*end for loop (neighborhood tiles)*/
    break ;

  case CLUSTER_METHOD_ISLAND:                     /*scan neighborhood tiles*/
    for ( i = 0, tile_idxp = & (Np->n_idx) ; i < 8 ; i++, tile_idxp++ ) {
      N_index = *tile_idxp ;
      if (may_grow_into( N_index, params )) {

        prio_counter = world_flags [ N_index ] & ADD_LAND_HERE_ALL ;
        if (prio_counter != ADD_LAND_HERE_ALL) { /*not yet 7 reached*/
          prio_counter++ ;
          world_flags [ N_index ] |= prio_counter ; /*<-Bug!!*/
        }

      }
    } /*end for loop (neighborhood tiles)*/
    break ;

  default: /*this should never happen*/
    fprintf( stderr, "add_tile_to_cluster: invalid method\n" ) ;
    exit( EXITCODE_WRONG_PARAM ) ;
  }
  /*TRACE_OFF*/
}

/*-------------------->   may_grow_into   <---------------------- 2012-Sep-24
This function tells if a cluster may grow into a certain tile, according to
the rules specified by the CLUSTER_PARAMS.
Checked rules:
- tile index inside map?
- match flags (= does not yet belong to the cluster under construction)
- tile is in 'Grow into' TTL list
- area flags
-----------------------------------------------------------------------------
Used functions:
Globals:   total_no_of_tiles, world, world_flags (r/o)
Internals: --
Parameters:	- tidx     the index of the tile in question
		- params   parameter set
Return value:	TRUE if the cluster may grow into the tile
Exitcode:	--
---------------------------------------------------------------------------*/
static BIT may_grow_into( TIDX tidx, CLUSTER_PARAMS* params )
{
/*THIS_FUNC(may_grow_into)*/
  /*if (tidx == 2090)  TRACE_ON*/
  /*PRT_VAR((unsigned)tidx,u)*/
  /*PRT_VAR((unsigned)(world_flags [tidx]),02x)*/
  /*PRT_VAR((unsigned)(world_flags [tidx] & (params->tag_mask)),02x)*/
  if (tidx < total_no_of_tiles) { /*inside map*/
    if (NOT_IN_CLUSTER( tidx )) {
      if (is_in_TTL( tidx, params->grow_into )) {
        if (params->area_only_for_start_points) {
            /*DEB((stderr,"TRUE\n"))*/
            /*TRACE_OFF*/
            return TRUE ;
        }
        else {
          if ((world_flags [tidx] & (params->area_mask)) == (params->area)) {
            /*DEB((stderr,"TRUE\n"))*/
            /*TRACE_OFF*/
            return TRUE ;
          }
        }
      }
    }
  }
  /*DEB((stderr,"FALSE\n"))*/
  /*TRACE_OFF*/
  return FALSE ;
}

/*-------------------->   draw_land_tile   <--------------------- 2013-Apr-12
This function returns a land tile to add_clusters().
-----------------------------------------------------------------------------
Used functions: draw_island_tile
Globals:   basic_probability_*
Internals: --
Parameters:	- tidx
Return value:	land tile
Exitcode:	--
---------------------------------------------------------------------------*/
TILE draw_land_tile( TIDX tidx )
{
/*THIS_FUNC(draw_land_tile)*/
  return draw_island_tile( tidx, basic_probability_desert,
                                 basic_probability_prairie,
                                 basic_probability_tundra
                         ) ;
}

/*-------------------->   clus_ocean   <----------------------- 2015-Jun-04*/
/*-------------------->   clus_mountains   <------------------- 2015-Jun-04*/
/*-------------------->   clus_hills   <----------------------- 2017-Feb-19*/
/*-------------------->   clus_grass   <----------------------- 2017-Feb-19*/
/*-------------------->   clus_prairie   <--------------------- 2017-Feb-19*/
/*-------------------->   clus_forest   <---------------------- 2017-Feb-19*/
/*These functions are for cluster generation.
-----------------------------------------------------------------------------
Used functions:
Globals:   world []
Internals: --
Parameters: - TIDX  index of tile to be added to cluster
Return value: a TILE (to be added to cluster)
Exitcode:     EXITCODE_UNSPECIFIED_ERR
---------------------------------------------------------------------------*/
TILE clus_ocean( TIDX tidx )
{
/*THIS_FUNC(clus_ocean)*/
  return OCEAN ;
}

TILE clus_mountains( TIDX tidx )
{
/*THIS_FUNC(clus_mountains)*/
  return MOUNTAINS ;
}

TILE clus_hills( TIDX tidx )
{
THIS_FUNC(clus_hills)
  ASSERT_ALT((world [tidx] & BASIC_TILE_TYPE_MASK) != HILLS,
                world [tidx] = DESERT | DEAD_LANDS ;
                map_gen_exit() ;)
  return (world [tidx] & ~BASIC_TILE_TYPE_MASK) | HILLS ;
                                      /*preserve river*/
}

TILE clus_grass( TIDX tidx )
{
THIS_FUNC(clus_grass)
  ASSERT_ALT((world [tidx] & BASIC_TILE_TYPE_MASK) != GRASSLAND,
                world [tidx] = DESERT | DEAD_LANDS ;
                map_gen_exit() ;)
  return (world [tidx] & ~BASIC_TILE_TYPE_MASK) | GRASSLAND ;
                                      /*preserve river*/
}

TILE clus_prairie( TIDX tidx )
{
THIS_FUNC(clus_prairie)
  ASSERT_ALT((world [tidx] & BASIC_TILE_TYPE_MASK) != PRAIRIE,
                world [tidx] = DESERT | DEAD_LANDS ;
                PRT_VAR((unsigned)tidx,u)
                map_gen_exit() ;)
  return (world [tidx] & ~BASIC_TILE_TYPE_MASK) | PRAIRIE ;
                                      /*preserve river*/
}

TILE clus_forest( TIDX tidx )
{
THIS_FUNC(clus_forest)
  /*PRT_VAR((unsigned)tidx,u)*/
  ASSERT_ALT((world [tidx] & BASIC_TILE_TYPE_MASK) != FOREST,
                world [tidx] = DESERT | DEAD_LANDS ;
                map_gen_exit() ;)
  return (world [tidx] & ~BASIC_TILE_TYPE_MASK) | FOREST ;
                                      /*preserve river*/
}

/*-------------------->   x   <---------------------------------- 2015-Jun-04
This function x
-----------------------------------------------------------------------------
Used functions:
Globals:   --
Internals: --
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
