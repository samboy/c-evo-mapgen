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
aux_rive.c

This lib handles rivers for "map_gen".
*****************************************************************************
History: (latest change first)
2016-Aug-08: clean log file implementation
2016-Jul-16: adoptions for gcc
2015-Aug-01: "erosion" internal->global
2015-Jun-05: new RIVER_PARAMS (change_mountains and new_tile)
2012-Sep-24: introduction of RIVER_PARAMS
2012-Sep-23: improved program structure, variables naming and comments
2012-Sep-21: - dist_tile_to_group with world_flags
             - renamed   rivers.c -> aux_rive.c
2012-Jun-08: add_rivers() now with parameter "visibility"
2012-Feb-15: changed several functions to be more object oriented
             (no NULL calls to set_neighborhood/set_city_tiles)
2012-Feb-10: Included "map_gen.h"
2010-May-01: Debugging
2008-Apr-26: River tile type probabilities dependent of param RIVER_VISIBLE
2008-Mar-20..Apr-05: physical model "add_rivers"
2008-Mar-17..20: Initial version
*****************************************************************************
Global objects:
- void add_rivers( RIVER_PARAMS* params )
- U16 rain_on_land_uniform( TIDX tile_index )
- U16 rain_on_land_terrain_dependant( TIDX tile_index )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include <stdio.h>

#include <misc.h>
#include <falloc.h>

/*#define DEBUG*/
#include <debug.h>
#include "map_gen.h"

#ifdef DEBUG
#  include <openfile.h>
#endif /*DEBUG*/


/*--  constants  ----------------------------------------------------------*/



/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} PROBABILITY_VECTOR ;*/


/*--  local function prototypes  ------------------------------------------*/

static void generate_elevation( void ) ;
static U16 force_flow( void ) ;
static void tag_all_flowable_tiles( U8 mask ) ;
static BIT let_it_flow( U16 visibility ) ;

static TIDX flows_to( TIDX index, U8 mode ) ;
#define                              NO_MOD            0
#define                              CONVERT_TO_OCEAN  1
#define                              FORCE_FLOW        2

static void draw_rivers( RIVER_PARAMS* params ) ;
static void change_tile( TIDX tile_index, U16 visibility ) ;
static void tab_put( TIDX index ) ;
static TIDX tab_get( void ) ;

#ifdef DEBUG
  static void write_elevation( char* filename ) ;
  static void prt_flow_path( U32 index ) ;
#endif /*DEBUG*/


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/

U16* erosion ; /*total water flowing thru this tile*/


/*--  internal variables  -------------------------------------------------*/

static U16* elev ; /*elevation = Hoehenprofil*/
static U16* quantity ; /*water quantity*/
/*static U16* erosion ;*/ /*total water flowing thru this tile*/

static U16 total_rain, total_sink ;

#define MAX_NO_OF_START_POINTS 200
static U32 start_point [MAX_NO_OF_START_POINTS] ;
static U16 no_of_start_points ;
DEB_STATEMENT(static U16 max_start_points = 0 ;)



/*-------------------->   add_rivers   <------------------------- 2016-Aug-08
This function adds rivers to the map.
A height map (elevation) is created, based on the distance to the next water
tile, superponed with a random elevation ('noise') so that rivers don't
run too straight.
Rain is falling down on each tile, flows down until it hits a water tile.
The accumulated water flowing thru a tile gives the tiles erosion.
The more erosion, the deeper a tile gets.  The whole process is repeated a
few times (currently twice).
-----------------------------------------------------------------------------
Used functions:
Parameters:	- params     ... for river generation (see "map_gen.h")
Return value:	void
Exitcode:	x
---------------------------------------------------------------------------*/
void add_rivers( RIVER_PARAMS* params )
{
/*THIS_FUNC(add_rivers)*/
char* _this_func = "add_rivers" ; /*for WORLD_FLAGS*/
  /*U32 tile_index, idx2 ;*/
  U32 tile_index ;
  U8 i ; /*loop control*/
  S16 temp ;

  DEB_STATEMENT(U16 sum_remaining;)
  DEB_STATEMENT(char filename [13];)


  log_with_timestamp( "Entry add_rivers" ) ;

     /*is WATER on the map?*/
  temp = 0 ; /*temp == 1 means WATER_found*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_WATER )) {
      temp = 1 ;
    }
  }
  if (temp == 0) { /*no WATER on the map*/
    fprintf( log_fp, "add_rivers: no water tiles on the map => no rivers\n" ) ;
    /*map_gen_exit() ;*/ /*currently, I allow this, so it is a return*/
    return ;
  }

     /*check parameters*/
  if (params->rain_func == NULL) {
    fprintf( log_fp, "add_rivers: rain_func == NULL\n" ) ;
    map_gen_exit() ;
  }
  if (params->visibility == 0) {
    fprintf( log_fp, "add_rivers: visibility == 0\n" ) ;
    map_gen_exit() ;
  }

     /*there is WATER on the map => do generate rivers*/
  elev =     falloc( 2 * total_no_of_tiles ) ;
  quantity = falloc( 2 * total_no_of_tiles ) ; /*water quantity*/
  erosion =  falloc( 2 * total_no_of_tiles ) ;

  WORLD_FLAGS_ALLOC( TAGGED | ADD_HERE0 | NO_RIVER_HERE | NO_LAKE_HERE )
  clear_flags(       TAGGED | ADD_HERE0 | NO_RIVER_HERE | NO_LAKE_HERE ) ;

  generate_elevation() ; /*Hoehenprofil erstellen*/

  for ( i = 0 ; i < 2 ; i++ ) { /*erosion cycles*/
    /*PRT_VAR((unsigned)i,u)*/
    fprintf( log_fp, "add_rivers: erosion cycle = %u\n", (unsigned)i ) ;

       /*let it rain*/
    total_rain = 0 ;
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      erosion [ tile_index ] = 0 ;
      quantity [ tile_index ] = (*(params->rain_func))( tile_index ) ;
      total_rain += quantity [ tile_index ] ;
    }
    /*DEB((stderr, "The rain came down (%u)\n", total_rain))*/
    /*PRT_VAR(max_gradient,u)*/


    force_flow() ;
    /*sprintf( filename, "erosi_%02u.txt", (unsigned)i ) ;*/
    /*write_elevation( filename ) ;*/
    clear_flags( TAGGED | ADD_HERE0 ) ;
    total_sink = 0 ;
    temp = 200 ;
    while (let_it_flow( params->visibility ) && temp) {
      ;
      temp-- ;
      ASSERT(temp > 0)
#ifdef DEBUG
      sum_remaining = 0 ;
      for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
        sum_remaining += quantity [ tile_index ] ;
      }
#endif /*DEBUG*/
      /*PRT_VAR(sum_remaining,u)*/
      /*PRT_VAR(total_sink,u)*/
      ASSERT(total_rain == total_sink + sum_remaining)
    }
    /*PRT_VAR(total_rain,u)*/
    /*PRT_VAR(total_sink,u)*/
    ASSERT(total_rain == total_sink)

       /*do the erosion*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      if (erosion [ tile_index ] > 0) {

        elev [ tile_index ] -= erosion [ tile_index ] ;
#ifdef NEVER
           /*lower nearby tiles*/
        if (erosion [ tile_index ] > RIVER_VISIBLE / 2) {
          for ( idx2 = 0 ; idx2 < total_no_of_tiles ; idx2++ ) {
            if (is_in_TTL( idx2, TTL_LAND )) {
              if (dist_tile_to_tile( tile_index, idx2 )
                                 < 10 * erosion [tile_index]) {
                elev [ idx2 ] -= 1 ;
              }
            }
          }
        }
#endif /*NEVER*/
        ASSERT(elev [ tile_index ] > 10000)
      }
    }
  } /*end erosion cycles*/
  /*write_elevation( "erosion.txt" ) ;*/

  draw_rivers( params ) ;

  WORLD_FLAGS_FREE( TAGGED | ADD_HERE0 | NO_RIVER_HERE | NO_LAKE_HERE )
  log_with_timestamp( "Exit add_rivers" ) ;
}

/*-------------------->   generate_elevation   <----------------- 2016-Aug-08
This function sets the elevation profile (elev[]) for the whole map.
WATER is lowest.  Land tiles are higher.  The larger the distance to the next
WATER tile is, the higher the land tile.
-----------------------------------------------------------------------------
Used functions:
Globals: world [], total_no_of_tiles (r/o)
Internals: elev [] (r/w)
Parameters: --
Return value: void
Exitcode:	x
---------------------------------------------------------------------------*/
static void generate_elevation( void )
{
THIS_FUNC(generate_elevation)
  TIDX tile_index, idx2 ;
  NEIGHBORHOOD NT ;
  NEIGHBORHOOD* Np ;
  CITY_TILES CT ;
  CITY_TILES* Cp ;
  U32* idxp ;
  U16 dist ;
  U16 inc_depth ;
  U16 dice ; /*scratchpad for random number*/
  /*U16 min_elev, max_elev ;*/
  U8 i ; /*loop control*/


  log_with_timestamp( "Entry generate_elevation" ) ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_WATER )) {
      world_flags [ tile_index ] |= TAGGED ; /*tag water tiles*/
    }
    else {
      world_flags [ tile_index ] &= ~TAGGED ; /*untag land tiles*/
    }
  }

  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_WATER )) {
      elev [ tile_index ] = 0 ;
    }
    else {
      elev [ tile_index ] = 50000 /*an offset big enough so that land tiles
                            never are lowered below water level, no matter
                            what modifications follow (erosion, ...)*/
            + (dist_tile_to_group( tile_index, TAGGED, TAGGED ) / 10) ;
    }
  }

  /*write_elevation( "elev.txt" ) ;*/

     /*add slopes with 1/r characteristic*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_WATER )) {
      for ( idx2 = 0 ; idx2 < total_no_of_tiles ; idx2++ ) {
        if (is_in_TTL( tile_index, TTL_LAND )) {
          dist = dist_tile_to_tile( tile_index, idx2 ) / 100 ;
          if (dist < 1) {
            dist = 1 ;
          }
          /*inc_depth = 900 / (dist * dist) ;*/
          inc_depth = 30 / dist ; /*1/r characteristic*/
          if (inc_depth > 100) { /*should never be >30; test for occurance,
                                   then delete*/
            inc_depth = 100 ; /*limit to a reasonable value*/
          }
          elev [idx2] -= inc_depth ;
          ASSERT(elev [idx2] < 30000)
          ASSERT(elev [idx2] > 1000)
        }
      }
    }
  }
  /*write_elevation( "grav.txt" ) ;*/


     /*set WATER to zero elevation and add random hills ("noise")*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_WATER )) {
      elev [ tile_index ] = 0 ;
    }
    else {
      dice = random_draw_range( 0, 10 ) ;
      elev [ tile_index ] += 3 * dice ;
      Np = set_neighborhood_tiles( tile_index, & NT ) ;
      for ( i = 0, idxp = & (Np->n_idx) ; i < 8 ; i++, idxp++ ) {
        if (is_in_TTL( *idxp, TTL_LAND )) { /*no poles, no water*/
          elev [ *idxp ] += dice ;
        }
      }
      Cp = set_city_tiles( tile_index, & CT ) ;
      for ( i = 0, idxp = & (Cp->i1) ; i < 20 ; i++, idxp++ ) {
        if (is_in_TTL( *idxp, TTL_LAND )) { /*no poles, no water*/
          elev [ *idxp ] += dice ;
        }
      }
    }
  }
  log_with_timestamp( "Hight profile done" ) ;
  /*PRT_VAR(max_elev,u)*/
  /*write_elevation( "noise.txt" ) ;*/

     /*Wasserscheiden markieren*/
#ifdef NEVER
  max_elev = 0 ;
  min_elev = 65535 ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (elev [ tile_index ] > max_elev) {
      max_elev = elev [ tile_index ] ;
    }
    if (elev [ tile_index ] < min_elev) {
      min_elev = elev [ tile_index ] ;
    }
    Np = set_neighborhood_tiles( tile_index, & NT ) ;
    if (count_adjacent_tiles( tile_index, TTL_POLE ) == 0) {
      if (  ((elev [tile_index] > elev [Np->ne_idx])
          && (elev [tile_index] > elev [Np->sw_idx])
            )
        ||  ((elev [tile_index] > elev [Np->nw_idx])
          && (elev [tile_index] > elev [Np->se_idx])
            )
         ) {
        if (is_in_TTL( tile_index, TTL_LAND )) {
          world [tile_index] |= RAILROAD ;
        }
      }
    }
  }
  DEB((stderr, "Wasserscheiden markiert\n"))
  PRT_VAR(min_elev,u)
  PRT_VAR(max_elev,u)
#endif /*NEVER*/
  log_with_timestamp( "Exit generate_elevation" ) ;
}

/*-------------------->   force_flow   <------------------------- 2012-Feb-15
Diese Funktion erzwingt einen Abfluss zu WATER fuer jedes Land-Tile.
Algorithmus:
Jedes Land-Tile, welches keinen ungehinderten Abfluss zu WATER hat, jedoch
adjacent zu einem solchen liegt, wird auf eine relative Hoehe von +1 zu dem
Tile mit Abfluss gesetzt.  => Tiles werden nur  *hoeher*  gesetzt
Zweck: Eliminieren von Mulden, aus denen Wasser nicht abfliessen kann.
-----------------------------------------------------------------------------
Used world_flags: TAGGED: Tiles, die ungehinderten Abfluss haben
Used functions:
Parameters:	--
Return value:	no. of tiles modified
Exitcode:	--
---------------------------------------------------------------------------*/
static U16 force_flow( void )
{
THIS_FUNC(force_flow)
  NEIGHBORHOOD NT ;
  NEIGHBORHOOD* Np ;
  U32 tile_index, idx2, remember_idx ;
  U16  min_local_elev ;
  U16 tiles_modified = 0 ;
  U8 i ; /*loop control*/
  BIT ongoing = TRUE ; /*default*/


  tag_all_flowable_tiles( TAGGED ) ;
  /*DEB_STATEMENT(prt_flow_path(77);)*/

  while (ongoing) {
    ongoing = FALSE ; /*default*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      if ((world_flags [ tile_index ] & TAGGED) == 0) {
        Np = set_neighborhood_tiles( tile_index, & NT ) ;
        min_local_elev = 65535 ;
        remember_idx = 0xffffffff ;
        for ( i = 1 ; i < 8 ; i += 2 ) { /*scan adjacent tiles*/
          idx2 = ( & (Np->n_idx))[ i ] ;
             /*Suche alle Tiles, die ...*/
          if (idx2 < total_no_of_tiles) { /*... noch auf der Karte sind*/
            if (      (world_flags [idx2] & TAGGED) /*... Abfluss haben*/
                   && (min_local_elev > elev [idx2]) /*... davon den tiefsten*/
                   && is_in_TTL( idx2, TTL_LAND ) /*... und LAND sind*/
               ) {
              min_local_elev = elev [ idx2 ] ;
              remember_idx = idx2 ;
            }
          }
        } /*end neighborhood for loop*/
        if ( remember_idx != 0xffffffff) {
          ASSERT_ALT(elev [remember_idx] < 60000,
                     write_elevation( "forceflw.txt" );)
          if (elev [ tile_index ] <= elev [ remember_idx ]) {
            /*DEB((stderr, "index %lu: setting elev from %u to %u\n",*/
                /*tile_index, elev [tile_index], elev [remember_idx] +1 ))*/
            elev [ tile_index ] = elev [ remember_idx ] +1 ;
            tiles_modified++ ;
          }
          world_flags [ tile_index ] |= TAGGED ;
          ongoing = TRUE ;
        }
      }
    } /*end world for loop*/
  } /*end while loop*/
#ifdef DEBUG
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_LAND )) {
      ASSERT_ALT(flows_to( tile_index, NO_MOD ) != 0xffffffff,
                 PRT_VAR(world_flags [tile_index] & TAGGED,02x)
                 prt_flow_path(tile_index);)
    }
  }
#endif /*DEBUG*/
  return tiles_modified ;
}

/*-------------------->   tag_all_flowable_tiles   <------------- 2008-Apr-05
This function tags all tiles which have a non-inhibited flow path to WATER.
All WATER tiles are tagged, too.
-----------------------------------------------------------------------------
Used functions:
Parameters:	mask    for world_flags: Use this to tag
Return value:	--
Exitcode:	--
---------------------------------------------------------------------------*/
static void tag_all_flowable_tiles( U8 mask )
{
THIS_FUNC(tag_all_flowable_tiles)
  U32 tile_index ;
  U32 flowing_to ;
  BIT ongoing = TRUE ; /*default*/

  clear_flags( mask ) ;
  while (ongoing) {
    ongoing = FALSE ; /*default*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      if ((world_flags [ tile_index ] & mask) == 0) {
        if (is_in_TTL( tile_index, TTL_LAND )) {
          flowing_to = flows_to( tile_index, NO_MOD ) ;
          if (flowing_to != 0xffffffff) {
            if (world_flags [flowing_to] & mask) {
              world_flags [ tile_index ] |= mask ;
              ongoing = TRUE ;
            }
          }
        }
        else { /*is WATER tile*/
          world_flags [ tile_index ] |= mask ;
          ongoing = TRUE ;
        }
      }
    } /*end for loop*/
  } /*end while loop*/
}

/*-------------------->   let_it_flow   <------------------------ 2016-Jul-16
Diese Funktion laesst das abgeregnete Wasser jeweils ein Tile weiterfliessen.
Dabei werden nicht alle Tiles beruecksichtigt, sondern nur solche, die selbst
*keinen* Zufluss haben.
Das "erosion"-Array wird aufakkumuliert, proportional zur durch ein Tile
fliessenden Wassermenge.
-----------------------------------------------------------------------------
Used functions:
Used internal/global variables: TAGGED bit of "world_flags"
Parameters:	--
Return value:	TRUE, wenn mindestens ein Tile bearbeitet wurde, sonst FALSE
Exitcode:	--
---------------------------------------------------------------------------*/
static BIT let_it_flow( U16 visibility )
{
THIS_FUNC(let_it_flow)
  U32 tile_index, idx2 ;
  BIT yet_flowing = FALSE ; /*default*/
  U16 total_q ;


  /*ENTRY*/

     /*1. Suche alle nicht-Wasser-leeren Land-Tiles ohne Zufluss*/
  /*DEB((stderr, "Step 1\n"))*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_LAND )) {
      if (quantity [ tile_index ] > 0) {
        world_flags [ tile_index ] |= TAGGED ;/*default:hat keinen Zufluss */
      }
      else {
        world_flags [ tile_index ] &= ~TAGGED ;
           /*Wo nix is, kann nix verschoben werden*/
      }
    }
  }
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_LAND )) {
      if (quantity [ tile_index ] > 0) {
        idx2 = flows_to( tile_index, NO_MOD);/*where does this water flow to?*/
        if (idx2 != 0xffffffff) {
          world_flags [ idx2 ] &= ~TAGGED ; /*target tile hat Zufluss*/
        }
      }
    }
  }

     /*2. Lasse die unter 1. gefundenen Tiles eine Stufe abfliessen*/
  /*DEB((stderr, "Step 2\n"))*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (((  world_flags [tile_index]) & TAGGED)
         && is_in_TTL( tile_index, TTL_LAND ) /*beware of converted OCEANs*/
       ) {
      yet_flowing = TRUE ;
      if (quantity [tile_index] > visibility) {
        world_flags [ tile_index ] |= ADD_HERE0 ; /*als RIVER vormerken*/
      }
      idx2 = flows_to( tile_index, NO_MOD ) ;
      ASSERT_ALT(idx2 != 0xffffffff, PRT_VAR((unsigned long)tile_index,lu))
      if (is_in_TTL( idx2, TTL_WATER )) {
        total_sink += quantity [tile_index] ;
      }
      else { /*land tile*/
        quantity [idx2] += quantity [tile_index] ;
      }
      /*erosion [ tile_index ] = quantity [ tile_index ] ;*/
      erosion [ tile_index ] += quantity [ tile_index ] ;
      quantity [ tile_index ] = 0 ;
    }
  }
  total_q = 0 ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    total_q += quantity [ tile_index ] ;
  }
  if (total_q == 0) {
    /*EXIT*/
    return FALSE ;
    ASSERT(yet_flowing == FALSE)
  }
  else {
    /*EXIT*/
    return TRUE ;
    ASSERT(yet_flowing == TRUE)
  }
  /*EXIT*/
  return yet_flowing ;
}

/*-------------------->   flows_to   <--------------------------- 2016-Aug-08
Diese Funktion berechnet, in welche Richtung Wasser von einem Tile wegfliesst.
1. to WATER
2. to adjacent  *lower*  river
3. to lowest elevation
;Ist kein Abfluss moeglich, so wird das Tile zu OCEAN.
;Ist kein Abfluss moeglich, so wird das niedrigste Nachbar-Tile tiefergelegt.
Was passiert, falls kein Abfluss moeglich ist, bestimmt der 'mode'-Parameter.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- Source tile
                - mode: NO_MOD, CONVERT_TO_OCEAN, FORCE_FLOW
Return value:	Target tile ;bzw. 0xffffffff, falls kein Abfluss moeglich
Exitcode:	--
---------------------------------------------------------------------------*/
static TIDX flows_to( TIDX index, U8 mode )
{
THIS_FUNC(flows_to)
  NEIGHBORHOOD NT ;
  NEIGHBORHOOD* Np ;
  U32 idx2, remember_idx ;
  U16  min_local_elev ;
  U8 i ; /*loop control*/


  ASSERT(is_in_TTL( index, TTL_LAND ))
  Np = set_neighborhood_tiles( index, & NT ) ;
  /*min_local_elev = elev [ index ] ;*/
  min_local_elev = 65535 ;
  remember_idx = 0xffffffff ;
  for ( i = 1 ; i < 8 ; i += 2 ) { /*scan adjacent tiles*/
    idx2 = ( & (Np->n_idx))[ i ] ;
    if (idx2 < total_no_of_tiles) {
      if (is_in_TTL( idx2, TTL_WATER )) { /*flow always to WATER*/
        return idx2 ;
      }
      else if ((world_flags [idx2] & ADD_HERE0)
            && (elev [index] > elev [idx2])
              ) {
        return idx2 ;
      }
      else if (elev [ idx2 ] < min_local_elev) {
        min_local_elev = elev [ idx2 ] ;
        remember_idx = idx2 ;
      }
    }
  } /*end for loop*/
     /*Remember_idx zeigt jetzt auf das niedrigste adjacente Tile.*/
     /*Dieses kann allerdings immer noch hoeher sein als das zentrale Tile.*/
  ASSERT(remember_idx != 0xffffffff)

  switch (mode) {
  case NO_MOD:
    if (min_local_elev >= elev [ index ]) { /*kein Abfluss*/
      remember_idx = 0xffffffff ;
    }
    break ;

  case CONVERT_TO_OCEAN:
    if (remember_idx == 0xffffffff) {
      DEB((stderr, "new OCEAN at index %lu\n", (unsigned long)index))
      world [ index ] = OCEAN ;
      elev [ index ] = 0 ;
      total_rain -= quantity [ index ] ;
      quantity [ index ] = 0 ;
    }
    break ;

  case FORCE_FLOW:
    if (min_local_elev >= elev [ index ]) { /*eigentlich kein Abfluss*/
      ASSERT(elev [ index ] > 1)
      DEB((stderr, "index %lu: setting elev from %u to %u\n",
            (unsigned long)remember_idx, elev [remember_idx], elev [index] -1 ))
      elev [ remember_idx ] = elev [ index ] -1 ; /*Abfluss erzwingen*/
    }
    break ;

  default:
    fprintf( log_fp, "flows_to: invalid 'mode' parameter\n" ) ;
    map_gen_exit() ;
  } /*end switch*/

  return remember_idx ;
}

/*-------------------->   draw_rivers   <------------------------ 2016-Aug-08
This function does the actual drawing, i. e. the map manipulation.

"Primary start points" are put into a table.
Primary start points are all tiles which:
- have at least one WATER tile adjacent
- have an erosion >= visibility
- have their NO_RIVER_HERE flag unset
-----------------------------------------------------------------------------
Used functions:
Globals:        world_flags: NO_RIVER_HERE
Internals:      erosion,
                no_of_start_points
Parameters: - params     ... for river generation
Return value: void
Exitcode:     x
---------------------------------------------------------------------------*/
static void draw_rivers( RIVER_PARAMS* params )
{
THIS_FUNC(draw_rivers)
  /*TIDX tile_index, idx2 ;*/
  TIDX tile_index ;
  NEIGHBORHOOD NT ;
  NEIGHBORHOOD* Np ;

  TIDX index_links, index_geradeaus, index_rechts ;
  TIDX index_halb_links, index_halb_rechts ;
  U8 heading_links, heading_geradeaus, heading_rechts ;
  U8 heading_halb_links, heading_halb_rechts ;

  U8 flowing_to ; /*indices in NEIGHBORHOOD*/
  U8 branch_flags ; /*indicates from which direction(s) a tile is supplied*/
#define LINKS     0x01
#define GERADEAUS 0x02
#define RECHTS    0x04

  //U8 i ; /*loop control*/
  DEB_STATEMENT(unsigned forks = 0 ;)


     /*primaere Startpunkte in Tabelle eintragen*/
  no_of_start_points = 0 ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if ((world_flags [ tile_index ] & NO_RIVER_HERE) == 0) {
      if (erosion [ tile_index ] >= params->visibility) {
        if (count_adjacent_tiles( tile_index, TTL_WATER ) > 0) {
          tab_put( tile_index ) ; /*quantity comes from erosion*/
        }
      }
    }
  }
  /*PRT_VAR(no_of_start_points,u)*/
  fprintf( log_fp, "draw_rivers: no_of_start_points = %u\n",
                                 no_of_start_points ) ;

     /*process all tiles in table, put new tiles into table recursivly*/
  while (no_of_start_points > 0) {
    tile_index = tab_get() ; /*read and erase in table*/
                             /*mouth point with max erosion*/
    ASSERT(tile_index < total_no_of_tiles)
    ASSERT(erosion [ tile_index ] >= params->visibility)
       /*There should be only one (1) river tile in the neighborhood.*/
       /*That is the direction where this tile will flow to.*/
    /*ASSERT_ALT(count_adjacent_tiles( tile_index, TTL_RIVER ) < 2,*/
               /*world [ tile_index ] = DESERT ;*/
               /*write_map_to_file( "debug.cevo map" ) ;)*/
       /*There should be at least one tile in the neighborhood where*/
       /*the water can flow to.*/
    ASSERT(count_adjacent_tiles( tile_index, TTL_RIVER | TTL_WATER ) > 0)
    /*DEB((stderr, "\nnew RIVER tile at tile index %lu (flow: %u)\n",*/
                      /*tile_index, erosion [tile_index]))*/


    if (params->change_tiles) {
                                             /*GRASSLAND, FOREST, HILLS*/
      change_tile( tile_index, params->visibility ) ;
    }
    else {
      if (is_in_TTL( tile_index, TTL_MOUNTAINS | TTL_ARCTIC | TTL_WATER )) {
        if (   (params->change_mountains)
            && ((world [tile_index] & BASIC_TILE_TYPE_MASK) == MOUNTAINS)) {
          world [tile_index] &= ~BASIC_TILE_TYPE_MASK ;
          ASSERT(((params->new_tile) & ~BASIC_TILE_TYPE_MASK) == 0)
          world [tile_index] |= params->new_tile ;
        }
        else {
          fprintf( log_fp, "cannot add river to mountain/arctic/water tile\n" ) ;
          map_gen_exit() ;
        }
      }
    }
    world [ tile_index ] |= RIVER ;


    Np = set_neighborhood_tiles( tile_index, & NT ) ;
    flowing_to = 22 ; /*'impossible' value for ASSERT below*/
    if (is_in_TTL( Np->ne_idx, TTL_WATER | TTL_RIVER )) flowing_to = 1 ;
    if (is_in_TTL( Np->se_idx, TTL_WATER | TTL_RIVER )) flowing_to = 3 ;
    if (is_in_TTL( Np->sw_idx, TTL_WATER | TTL_RIVER )) flowing_to = 5 ;
    if (is_in_TTL( Np->nw_idx, TTL_WATER | TTL_RIVER )) flowing_to = 7 ;
    ASSERT(flowing_to != 22)
    /*PRT_VAR((unsigned)flowing_to,u)*/
       /*german directions are UPstream directions*/
    heading_links       = (flowing_to +2) % 8 ;
    heading_halb_links  = (flowing_to +3) % 8 ;
    heading_geradeaus   = (flowing_to +4) % 8 ;
    heading_halb_rechts = (flowing_to +5) % 8 ;
    heading_rechts      = (flowing_to +6) % 8 ;
    index_links       = ( & (Np->n_idx)) [heading_links] ;
    index_halb_links  = ( & (Np->n_idx)) [heading_halb_links] ;
    index_geradeaus   = ( & (Np->n_idx)) [heading_geradeaus] ;
    index_halb_rechts = ( & (Np->n_idx)) [heading_halb_rechts] ;
    index_rechts      = ( & (Np->n_idx)) [heading_rechts] ;

    branch_flags = 0 ;
    /*PRT_VAR(index_links,lu)*/
    if (index_links < total_no_of_tiles) {
      /*PRT_VAR(erosion [ index_links ],u)*/
      if (((world_flags [ index_links ] & NO_RIVER_HERE) == 0)
       && (erosion [ index_links ] >= params->visibility)
         ) {
        /*PRT_VAR(flows_to( index_links, NO_MOD ),lu)*/
        if (flows_to( index_links, NO_MOD ) == tile_index) {
          branch_flags |= LINKS ;
          tab_put( index_links ) ; /*draw river there later*/
        }
      }
    }
    /*PRT_VAR(index_geradeaus,lu)*/
    if (index_geradeaus < total_no_of_tiles) {
      /*PRT_VAR(erosion [ index_geradeaus ],u)*/
      if (((world_flags [ index_geradeaus ] & NO_RIVER_HERE) == 0)
       && (erosion [ index_geradeaus ] >= params->visibility)
         ) {
        /*PRT_VAR(flows_to( index_geradeaus, NO_MOD ),lu)*/
        if (flows_to( index_geradeaus, NO_MOD ) == tile_index) {
          branch_flags |= GERADEAUS ;
          tab_put( index_geradeaus ) ; /*draw river there later*/
        }
      }
    }
    /*PRT_VAR(index_rechts,lu)*/
    if (index_rechts < total_no_of_tiles) {
      /*PRT_VAR(erosion [ index_rechts ],u)*/
      if (((world_flags [ index_rechts ] & NO_RIVER_HERE) == 0)
       && (erosion [ index_rechts ] >= params->visibility)
         ) {
        /*PRT_VAR(flows_to( index_rechts, NO_MOD ),lu)*/
        if (flows_to( index_rechts, NO_MOD ) == tile_index) {
          branch_flags |= RECHTS ;
          tab_put( index_rechts ) ; /*draw river there later*/
        }
      }
    }

    switch (branch_flags) {
    case 0:
      /*DEB((stderr, "end of river\n"))*/
      SET_WORLD_FLAGS( index_links, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_halb_links, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_geradeaus, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_halb_rechts, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_rechts, NO_RIVER_HERE | NO_LAKE_HERE )
      break ;

    case LINKS:
      SET_WORLD_FLAGS( index_geradeaus, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_halb_rechts, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_rechts, NO_RIVER_HERE | NO_LAKE_HERE )
      break ;

    case GERADEAUS:
      SET_WORLD_FLAGS( index_links, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_rechts, NO_RIVER_HERE | NO_LAKE_HERE )
      break ;

    case LINKS | GERADEAUS:
      DEB_STATEMENT(forks++ ;)
      SET_WORLD_FLAGS( index_halb_links, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_rechts, NO_RIVER_HERE | NO_LAKE_HERE )
      break ;

    case RECHTS:
      SET_WORLD_FLAGS( index_links, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_halb_links, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_geradeaus, NO_RIVER_HERE | NO_LAKE_HERE )
      break ;

    case LINKS | RECHTS:
      DEB_STATEMENT(forks++ ;)
      SET_WORLD_FLAGS( index_geradeaus, NO_RIVER_HERE | NO_LAKE_HERE )
      break ;

    case GERADEAUS | RECHTS:
      DEB_STATEMENT(forks++ ;)
      SET_WORLD_FLAGS( index_links, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_halb_rechts, NO_RIVER_HERE | NO_LAKE_HERE )
      break ;

    case LINKS | GERADEAUS | RECHTS:
      DEB_STATEMENT(forks += 2 ;)
      SET_WORLD_FLAGS( index_halb_links, NO_RIVER_HERE | NO_LAKE_HERE )
      SET_WORLD_FLAGS( index_halb_rechts, NO_RIVER_HERE | NO_LAKE_HERE )
      break ;

    default:
      fprintf( log_fp, "draw_rivers: branch_flags=%02x\n", branch_flags ) ;
      map_gen_exit() ;
    }
  } /*end while*/
  /*PRT_VAR(forks,u)*/
  /*PRT_VAR((unsigned)max_start_points,u)*/
  DEB_STATEMENT(fprintf( log_fp, "draw_rivers: forks = %u\n", forks ) ;)
  DEB_STATEMENT(fprintf( log_fp, "draw_rivers: max_start_points = %u\n",
                                    (unsigned)(max_start_points) ) ;)
}

/*-------------------->   change_tile   <------------------------ 2012-Sep-23
This function changes the basic tile type of a tile with river,
according to the ratio of erosion to visibility.
Effect: Tiles near the river's mouth (high erosion) tend to be GRASSLAND,
        while tiles near the river's source (low erosion) tend to be
        FOREST or HILLS
-----------------------------------------------------------------------------
Used functions: random_draw_range
Globals:        world []
Internals:      erosion []
Parameters:	- tile_index     index of tile to change
		- visibility
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
static void change_tile( TIDX tile_index, U16 visibility )
{
THIS_FUNC(change_tile)
  TILE new_tile_type ;
  U16 dice ; /*scratchpad for random number*/


  if (erosion [tile_index] > 7 * visibility) { /*always GRASSLAND*/
    new_tile_type = GRASSLAND ;
  }
  else if (erosion [tile_index] > 5 * visibility) { /*70% GRASS, 30% FOREST*/
    if (random_draw_range( 1,100 ) < 71) {
      new_tile_type = GRASSLAND ;
    }
    else {
      new_tile_type = FOREST ;
    }
  }
  else if (erosion [tile_index] > 2 * visibility) { /*50% GRASS, 50% FOREST*/
    if (random_draw_range( 1,100 ) < 51) {
      new_tile_type = GRASSLAND ;
    }
    else {
      new_tile_type = FOREST ;
    }
  }
  else { /*40% GRASS, 50% FOREST, 10% HILLS*/
    dice = random_draw_range( 1,100 ) ;
    if (dice < 41) {
      new_tile_type = GRASSLAND ;
    }
    else if (dice < 51) {
      new_tile_type = HILLS ;
    }
    else {
      new_tile_type = FOREST ;
    }
  }
  world [ tile_index ] = /*preserve RAILROAD (for debugging only)*/
       (world [tile_index] & ~BASIC_TILE_TYPE_MASK) | new_tile_type ;
}

/*-------------------->   tab_put   <---------------------------- 2016-Aug-08
A river is flowing from its source to its mouth.
This function keeps all (potential) mouth points in a sorted list.
Sorting criteria is the amount of water flowing thru the tile (= erosion).
Sorting order: least amount is in 1st entry

Secondary mouth points are ALL upstream tiles which are sorted dynamically
into this table.
-----------------------------------------------------------------------------
Used functions:
Globals:        no_of_start_points, start_point [],
                erosion []
Internals:
Parameters:	- index    ... of tile to be sorted into the table
Return value:	void
Exitcode:	;EXITCODE_TABLE_FULL
---------------------------------------------------------------------------*/
static void tab_put( TIDX index )
{
THIS_FUNC(tab_put)
  U16 quantity ;
  U16 i, j ; /*loop control*/


  no_of_start_points++ ;
#ifdef DEBUG
  if (no_of_start_points > max_start_points) {
    max_start_points = no_of_start_points ;
  }
#endif /*DEBUG*/
  /*PRT_VAR(no_of_start_points,u)*/
  if (no_of_start_points == MAX_NO_OF_START_POINTS) {
    fprintf( log_fp, "tab_put: table full\n" ) ;
    /*exit( EXITCODE_TABLE_FULL ) ;*/
    map_gen_exit() ;
  }

  if (no_of_start_points == 1) {
    start_point [ 0 ] = index ; /*the very first entry*/
    return ;
  }

  quantity = erosion [ index ] ;
  for ( i = 0 ; i < no_of_start_points -1 ; i++ ) {
    if (quantity < erosion [ start_point [i] ]) { /*hier einsortieren*/
      for ( j = no_of_start_points -1 ; j > i ; j-- ) {
        start_point [j] = start_point [ j -1 ] ;
      }
      start_point [ i ] = index ;
      return ;
    }
  }
  start_point [ no_of_start_points -1 ] = index ; /*am Ende einsortieren*/
}

/*-------------------->   tab_get   <---------------------------- 2016-Aug-08
Of all tiles in the table, this function returns the one with the maximum
erosion.  This tile is removed from the table.
-----------------------------------------------------------------------------
Used functions: fprintf, exit
Parameters:	--
Return value:	Index of tile with maximum erosion
Exitcode:	;EXITCODE_ENTRY_NOT_FOUND
---------------------------------------------------------------------------*/
static TIDX tab_get( void )
{
THIS_FUNC(tab_get)
  if (no_of_start_points == 0) {
    fprintf( log_fp, "tab_get: table empty\n" ) ;
    /*exit( EXITCODE_ENTRY_NOT_FOUND ) ;*/
    map_gen_exit() ;
  }
  no_of_start_points-- ;
  /*PRT_VAR(no_of_start_points,u)*/
  return start_point [ no_of_start_points ] ;
}

#ifdef DEBUG
/*-------------------->   write_elevation   <-------------------- 2008-Apr-05
Diese Funktion schreibt die Hoehenkarte in eine CSV-Datei.
Water tiles are set from zero to min_elev-1 in the output file (min_elev
is the lowest land tile elevation)
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
static void write_elevation( char* filename )
{
THIS_FUNC(write_elevation)
  FILE* fp ;
  U32 x, y ; /*loop control*/
  U32 index ;
  U16 e ;
  U16 min_elev = 0xffff ;

  PRT_VAR(filename,s)
  for ( index = 0 ; index < total_no_of_tiles ; index++ ) {
    if (elev [ index ] > 0) {
      if (elev [ index ] < min_elev) {
        min_elev = elev [ index ] ;
      }
    }
  }
  PRT_VAR(min_elev,u)

  fp = forced_fopen_wt( filename ) ;
  for ( y = 0 ; y < LY ; y++ ) {
    for ( x = 0 ; x < LX ; x++ ) {
      index = y * LX + x ;
      e = elev [ index ] ;
      if (e == 0) {
        e = min_elev -1 ;
      }
      if (x +1 == LX) {
        fprintf( fp, "%u", e ) ;
      }
      else {
        fprintf( fp, "%u,", e ) ;
      }
    }
    fprintf( fp, "\n" ) ;
  }
  forced_fclose( fp ) ;
}

/*-------------------->   prt_flow_path   <---------------------- 2016-Jul-16
This function printf the flow path (the chain of tiles where water flows)
for a certain tile to stderr.  For debugging purposes only.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- index   the start tile of the chain
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
static void prt_flow_path( U32 index )
{
THIS_FUNC(prt_flow_path)
  U32 next_idx ;

  fprintf( stderr, "flow path: %lu (%u)", (unsigned long)index, elev [index] ) ;
  next_idx = flows_to( index, NO_MOD ) ;
  while (next_idx != 0xffffffff) {
    fprintf( stderr, "->%lu (%u)", (unsigned long)next_idx, elev [next_idx] ) ;
    if (elev [next_idx] == 0) {
      break ;
    }
    next_idx = flows_to( next_idx, NO_MOD ) ;
  }
  fprintf( stderr, "\n" ) ;
}
#endif /*DEBUG*/

/*-------------------->   rain_on_land_uniform   <--------------- 2012-Sep-24
This function returns how much rain falls down on a certain tile.
Here: 1 unit of water upon every land tile
-----------------------------------------------------------------------------
Used functions: is_in_TTL
Globals:   world []
Internals: --
Parameters: - tile_index
Return value: rain quantity
Exitcode:     --
---------------------------------------------------------------------------*/
U16 rain_on_land_uniform( TIDX tile_index )
{
THIS_FUNC(rain_on_land_uniform)
  if (is_in_TTL( tile_index, TTL_LAND )) {
    return 1 ; /*one rain unit on each land tile*/
  }
  else {
    return 0 ; /*no rain on WATER tiles*/
  }
}

/*-------------------->   rain_on_land_terrain_dependant   <----- 2016-Aug-08
This functions return how much rain falls down on a certain tile.
Here: depending on terrain (tile type)
Used in the desert scenario.
-----------------------------------------------------------------------------
Used functions: is_in_TTL
Globals:   world []
Internals: --
Parameters: - tile_index
Return value: rain quantity
Exitcode:     --
---------------------------------------------------------------------------*/
U16 rain_on_land_terrain_dependant( TIDX tile_index )
{
THIS_FUNC(rain_on_land_terrain_dependant)
  TILE tile ;


  tile = (world [tile_index]) & BASIC_TILE_TYPE_MASK ;
  switch (tile) {
  case OCEAN:
  case COAST:
    return 0 ;

  case DESERT:
    return 1 ;

  case ARCTIC:
  case TUNDRA:
    return 2 ;

  case GRASSLAND:
  case PRAIRIE:
  case SWAMP:
    return 3 ;

  case HILLS:
  case FOREST:
    return 5 ;

  case MOUNTAINS:
    return 20 ;

  default:
    fprintf( log_fp,
"rain_on_land_terrain_dependant: illegal tile type (%u) at location code %lu\n",
             (unsigned)tile, (unsigned long)tile_index ) ;
    map_gen_exit() ;
    return 0 ; /*for gcc*/
  }
}

/*-------------------->   x   <---------------------------------- 2015-Jun-04
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
