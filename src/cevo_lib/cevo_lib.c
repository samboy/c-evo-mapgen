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
cevo_lib.c

This file contains lib functions for map_gen (and maybe for an AI)
*****************************************************************************
History: (latest change first)
2016-Jul-15: adoptions for gcc
2015-Jun-06: added "tag_adjacent_tiles"
2013-Apr-12..Jun-14: added "tag_whole_area" and "tag_on_flag_pattern_match"
2012-Oct-01: changed "count_*_tiles" to count poles, too
2012-Sep-25: new function "untag_inside_radius"
2012-Sep-21: dist_tile_to_group with world flag matching
2012-Feb-10: replaced "map_gen_exit" by "exit" (lib doesn't know about map_gen)
2010-Jun-22: added "tag_whole_TTL"
2010-Jun-21: lock against multiple lib initialization, "flags_in_use"
2010-May-14: tag_whole_land reprogrammed more object oriented
2009-Jan-16: tag_whole_ocean()
2008-Jul-13: tag_whole_island -> tag_whole_land, now 'pole-proof'
2008-Apr-06: added functions "count_city_tiles", "tag_city_tiles"
2008-Mar-21: "is_in_TTL" can handle pole indices
2008-Mar-18: added function "direction_tile_to_tile"
2008-Mar-17: added functions "count_neighborhood_tiles",
             "mark_neighborhood_tiles"
2007-Mar-17..18: added function "tile_resources"
2007-Mar-16: added function "dist_tile_to_tile"
2007-Mar-15: functionality added to "cevo_lib_init"
2006-Mar-26: init function added
2006-Feb-24..Mar-25: initial version
*****************************************************************************
Global objects:
- cevo_lib_init()
- set_neighborhood_tiles()
- set_city_tiles()
- is_land_tile()
- dist_tile_to_group()
- dist_tile_to_tile()
- direction_tile_to_tile()
- tag_whole_TTL( list, mask )
- tag_whole_land()
- tag_whole_ocean()
- tag_inside_radius()
- untag_inside_radius()
- tag_whole_coast_water()
- count_adjacent_tiles()
- count_neighborhood_tiles()
- count_city_tiles()
- is_in_TTL()
- clear_flags()
- tag_city_tiles()
- tag_neighborhood_tiles()
- void tag_adjacent_tiles( U32 index, U8 mask )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/


/*--  include files  ------------------------------------------------------*/

#include <misc.h>
#include <math.h>
/*#include <stdio.h>*/
/*#include <stdlib.h>*/
#include <falloc.h>

#define DEBUG
#include <debug.h>
#include "cevo_map.h"


/*--  constants  ----------------------------------------------------------*/



/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

static TTL tile2TTL( TILE tile ) ;


/*--  macros  -------------------------------------------------------------*/

#define IS_IN_TTL_AND_FLAGS_MATCH(tidx, list, mask, flags)  \
        (   (is_in_TTL( world [ tidx ], list ))  \
         && (((world_flags [ tidx ]) & (mask)) == (flags))   )


/*--  global variables  ---------------------------------------------------*/

U32 LX ;
U32 LY ;
U32 total_no_of_tiles ; /* = LX * LY */
U32 LX2 ; /* = 2 * LX */
U32 LX3 ; /* = 3 * LX */
U32 LX4 ; /* = 4 * LX */
U32 LY2 ; /* = 2 * LY */
U32 LY3 ; /* = 3 * LY */
U32 LY4 ; /* = 4 * LY */

TILE* world ; /*pointer to start of world map on heap*/

U8* world_flags ;
U8 flags_in_use = 0x00 ; /*1-bit means "in use"*/


/*--  internal variables  -------------------------------------------------*/


/*--  library functions  --------------------------------------------------*/

/*-------------------->   cevo_lib_init   <---------------------- 2010-Jun-21
This function x
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
void cevo_lib_init( void )
{
THIS_FUNC(cevo_lib_init)
static BIT initialized = FALSE ;
  U32 tile_index ;

  if (initialized) {
    return ;
  }
  initialized = TRUE ;
  total_no_of_tiles = LX * LY ;
  world = falloc( 4 * total_no_of_tiles ) ;
  world_flags = falloc( total_no_of_tiles ) ;
  LX2 = LX + LX ;
  LX3 = LX2 + LX ;
  LX4 = LX3 + LX ;
  LY2 = LY + LY ;
  LY3 = LY2 + LY ;
  LY4 = LY3 + LY ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    world [ tile_index ] = 0xffffffff ;
    world_flags [ tile_index ] = 0 ;
  }
}
/*-------------------->   set_neighborhood_tiles   <---------- 2012-Feb-10 --
This function fills in a NEIGHBORHOOD structure for a certain tile.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- index = tile index
		- dest = pointer to NEIGHBORHOOD to fill or NULL
Return value:	pointer to the filled NEIGHBORHOOD structure
Exitcode:	--
---------------------------------------------------------------------------*/
NEIGHBORHOOD* set_neighborhood_tiles( U32 index, NEIGHBORHOOD* dest )
{
THIS_FUNC(set_neighborhood_tiles)
  static NEIGHBORHOOD ret_val ;
  NEIGHBORHOOD* p ;
  /*BIT odd_line = (((index / LX) & 0x1) == 0x1) ;*/
  BIT even_line = (((index / LX) & 0x1) == 0) ;
  
  if (dest == NULL) {
    DEB((stderr,"using NULL\n"))
    /*map_gen_exit() ;*/ /*the general cevo lib doesn't know about map_gen!*/
    exit( 1 ) ;
    p = & ret_val ;
  }
  else {
    p = dest ;
  }

     /*default values*/
  p->n_idx = index - 2 * LX ;
  p->s_idx = index + 2 * LX ;
  p->e_idx = index + 1 ;
  p->w_idx = index - 1 ;
  if (even_line) {
    p->ne_idx = index - LX ;
    p->se_idx = index + LX ;
    p->sw_idx = index + LX - 1 ;
    p->nw_idx = index - LX - 1 ;
  }
  else { /*odd line*/
    p->ne_idx = index - LX + 1 ;
    p->se_idx = index + LX + 1 ;
    p->sw_idx = index + LX ;
    p->nw_idx = index - LX ;
  }

     /*east-west exceptions*/
  if (index % LX == 0) {
    p->w_idx = index + LX - 1 ;
    if (even_line) {
      p->nw_idx = index - 1 ;
      p->sw_idx = index + 2 * LX - 1 ;
    }
  }
  if ((index + 1) % LX == 0) {
    p->e_idx = index - LX + 1 ;
    if ( ! even_line) {
      p->ne_idx = index - 2 * LX + 1 ;
      p->se_idx = index + 1 ;
    }
  }

     /*north-south exceptions*/
  if (index < 2 * LX) {
    p->n_idx = NORTH_POLE ;
  }
  if (index < LX) {
    p->ne_idx = NORTH_POLE ;
    p->nw_idx = NORTH_POLE ;
  }
  if (index >= (LY - 2) * LX) {
    p->s_idx = SOUTH_POLE ;
  }
  if (index >= (LY - 1) * LX) {
    p->se_idx = SOUTH_POLE ;
    p->sw_idx = SOUTH_POLE ;
  }

  p->n_tile = (p->n_idx == NORTH_POLE) ? NORTH_POLE : world [p->n_idx] ;
  p->ne_tile = (p->ne_idx == NORTH_POLE) ? NORTH_POLE : world [p->ne_idx] ;
  p->e_tile = world [p->e_idx] ;
  p->se_tile = (p->se_idx == SOUTH_POLE) ? SOUTH_POLE : world [p->se_idx] ;
  p->s_tile = (p->s_idx == SOUTH_POLE) ? SOUTH_POLE : world [p->s_idx] ;
  p->sw_tile = (p->sw_idx == SOUTH_POLE) ? SOUTH_POLE : world [p->sw_idx] ;
  p->w_tile = world [p->w_idx] ;
  p->nw_tile = (p->nw_idx == NORTH_POLE) ? NORTH_POLE : world [p->nw_idx] ;

  return p ;
}

/*-------------------->   set_city_tiles   <------------------ 2012-Feb-10 --
This function x
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
CITY_TILES* set_city_tiles( U32 index, CITY_TILES* dest )
{
THIS_FUNC(set_city_tiles)
  static CITY_TILES ret_val ;
  CITY_TILES* p ;
  /*BIT odd_line = (((index / LX) & 0x1) == 0x1) ;*/
  BIT even_line = (((index / LX) & 0x1) == 0) ;
  
  if (dest == NULL) {
    DEB((stderr,"using NULL\n"))
    /*map_gen_exit() ;*/ /*the general cevo lib doesn't know about map_gen!*/
    exit( 1 ) ;
    p = & ret_val ;
  }
  else {
    p = dest ;
  }

     /*default values*/
  p->i3  = index - LX2 - 1 ;
  p->i4  = index - LX2 ;
  p->i5  = index - LX2 + 1 ;
  p->i10 = index - 1 ;
  p->i11 = index + 1 ;
  p->i16 = index + LX2 - 1 ;
  p->i17 = index + LX2 ;
  p->i18 = index + LX2 + 1 ;
  if (even_line) {
    p->i1  = index - LX3 - 1 ;
    p->i2  = index - LX3 ;
    p->i6  = index - LX - 2 ;
    p->i7  = index - LX - 1 ;
    p->i8  = index - LX ;
    p->i9  = index - LX + 1 ;
    p->i12 = index + LX - 2 ;
    p->i13 = index + LX - 1 ;
    p->i14 = index + LX ;
    p->i15 = index + LX + 1 ;
    p->i19 = index + LX3 - 1 ;
    p->i20 = index + LX3 ;
  }
  else { /*odd line*/
    p->i1  = index - LX3 ;
    p->i2  = index - LX3 + 1 ;
    p->i6  = index - LX - 1 ;
    p->i7  = index - LX ;
    p->i8  = index - LX + 1 ;
    p->i9  = index - LX + 2 ;
    p->i12 = index + LX - 1 ;
    p->i13 = index + LX ;
    p->i14 = index + LX + 1 ;
    p->i15 = index + LX + 2 ;
    p->i19 = index + LX3 ;
    p->i20 = index + LX3 + 1 ;
  }

     /*east-west exceptions*/
  if (index % LX == 0) {
    p->i3  = index - LX - 1 ;
    p->i10 = index + LX - 1 ;
    p->i16 = index + LX3 - 1 ;
    if (even_line) {
      p->i1  = index - LX2 - 1 ;
      p->i6  = index - 2 ;
      p->i7  = index - 1 ;
      p->i12 = index + LX2 - 2 ;
      p->i13 = index + LX2 - 1 ;
      p->i19 = index + LX4 - 1 ;
    }
    if ( ! even_line) {
      p->i6  = index - 1 ;
      p->i12 = index + LX2 - 1 ;
    }
  }
  else if ((index -1) % LX == 0) {
    if (even_line) {
      p->i6  = index - 2 ;
      p->i12 = index + LX2 - 2 ;
    }
  }

  else if ((index + 1) % LX == 0) {
    p->i5  = index - LX3 + 1 ;
    p->i11 = index - LX + 1 ;
    p->i18 = index + LX + 1 ;
    if (even_line) {
      p->i9  = index - LX2 + 1 ;
      p->i15 = index + 1 ;
    }
    if ( ! even_line) {
      p->i2  = index - LX4 + 1 ;
      p->i8  = index - LX2 + 1 ;
      p->i9  = index - LX2 + 2 ;
      p->i14 = index + 1 ;
      p->i15 = index + 2 ;
      p->i20 = index + LX2 + 1 ;
    }
  }
  else if ((index + 2) % LX == 0) {
    if ( ! even_line) {
      p->i9  = index - LX2 + 2 ;
      p->i15 = index + 2 ;
    }
  }

     /*north-south exceptions*/
  if (index < LX3) {
    p->i1 = NORTH_POLE ;
    p->i2 = NORTH_POLE ;
  }
  if (index < LX2) {
    p->i3 = NORTH_POLE ;
    p->i4 = NORTH_POLE ;
    p->i5 = NORTH_POLE ;
  }
  if (index < LX) {
    p->i6 = NORTH_POLE ;
    p->i7 = NORTH_POLE ;
    p->i8 = NORTH_POLE ;
    p->i9 = NORTH_POLE ;
  }

  if (index >= total_no_of_tiles - LX3) {
    p->i19 = SOUTH_POLE ;
    p->i20 = SOUTH_POLE ;
  }
  if (index >= total_no_of_tiles - LX2) {
    p->i16 = SOUTH_POLE ;
    p->i17 = SOUTH_POLE ;
    p->i18 = SOUTH_POLE ;
  }
  if (index >= total_no_of_tiles - LX) {
    p->i12 = SOUTH_POLE ;
    p->i13 = SOUTH_POLE ;
    p->i14 = SOUTH_POLE ;
    p->i15 = SOUTH_POLE ;
  }

  p->t1 = (p->i1 == NORTH_POLE) ? NORTH_POLE : world [p->i1] ;
  p->t2 = (p->i2 == NORTH_POLE) ? NORTH_POLE : world [p->i2] ;
  p->t3 = (p->i3 == NORTH_POLE) ? NORTH_POLE : world [p->i3] ;
  p->t4 = (p->i4 == NORTH_POLE) ? NORTH_POLE : world [p->i4] ;
  p->t5 = (p->i5 == NORTH_POLE) ? NORTH_POLE : world [p->i5] ;
  p->t6 = (p->i6 == NORTH_POLE) ? NORTH_POLE : world [p->i6] ;
  p->t7 = (p->i7 == NORTH_POLE) ? NORTH_POLE : world [p->i7] ;
  p->t8 = (p->i8 == NORTH_POLE) ? NORTH_POLE : world [p->i8] ;
  p->t9 = (p->i9 == NORTH_POLE) ? NORTH_POLE : world [p->i9] ;
  p->t10 = world [p->i10] ;
  p->t11 = world [p->i11] ;
  p->t12 = (p->i12 == SOUTH_POLE) ? SOUTH_POLE : world [p->i12] ;
  p->t13 = (p->i13 == SOUTH_POLE) ? SOUTH_POLE : world [p->i13] ;
  p->t14 = (p->i14 == SOUTH_POLE) ? SOUTH_POLE : world [p->i14] ;
  p->t15 = (p->i15 == SOUTH_POLE) ? SOUTH_POLE : world [p->i15] ;
  p->t16 = (p->i16 == SOUTH_POLE) ? SOUTH_POLE : world [p->i16] ;
  p->t17 = (p->i17 == SOUTH_POLE) ? SOUTH_POLE : world [p->i17] ;
  p->t18 = (p->i18 == SOUTH_POLE) ? SOUTH_POLE : world [p->i18] ;
  p->t19 = (p->i19 == SOUTH_POLE) ? SOUTH_POLE : world [p->i19] ;
  p->t20 = (p->i20 == SOUTH_POLE) ? SOUTH_POLE : world [p->i20] ;

  return p ;
}

/*-------------------->   is_land_tile   <-------------------- 2006-Mar-25 --
This function tells if a certain tile is a land tile.
ARCTIC is regarded as land tile.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
BIT is_land_tile( U32 tile_index)
{
THIS_FUNC(is_land_tile)
  switch ((world [ tile_index ]) & BASIC_TILE_TYPE_MASK) {
  case OCEAN:
  case COAST:
  /*case NORTHPOLE:*/
  /*case SOUTHPOLE:*/
    return FALSE ;
  default:
    return TRUE ;
  }
}

/*-------------------->   dist_tile_to_group   <----------------- 2012-Sep-21
This function returns the shortest distance between a single tile and
a group of tiles.  The group is defined by matching "world_flags".
-----------------------------------------------------------------------------
Used functions: dist_tile_to_tile
Globals:        total_no_of_tiles, world_flags
Internals:
Parameters:	- index      of tile
		- mask       for flags
		- flags      define the group
Return value:	distance in old movement points
Exitcode:	--
---------------------------------------------------------------------------*/
U16 dist_tile_to_group( TIDX index, U8 mask, U8 flags )
{
THIS_FUNC(dist_tile_to_group)
  U16 ret_val = 32767 ; /*default*/
  U16 dist ;
  TIDX i ;

  for ( i = 0 ; i < total_no_of_tiles -1 ; i++ ) {
    if ((world_flags [i] & mask) == flags) {
      dist = dist_tile_to_tile( i, index ) ;
      if (dist < ret_val) {
        ret_val = dist ;
      }
    }
  }
  return ret_val ;
}

/*-------------------->   dist_tile_to_tile   <--------------- 2007-Mar-16 --
This function returns the distance between two tiles, measured in
movement points for planes, ships and ground units in easy terrain.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- index1   Tile index of tile 1
		- index2   Tile index of tile 2 (may be equal to index 1)
Return value:	Distance in movement points
Exitcode:	x
---------------------------------------------------------------------------*/
U16 dist_tile_to_tile( U32 index1, U32 index2 )
{
THIS_FUNC(dist_tile_to_tile)
  U16 line1, line2, col1, col2 ;
  U16 delta_line, delta_col ;

  line1 = (U16)(index1 / LX) ;
  line2 = (U16)(index2 / LX) ;
  col1 = (((U16)(index1 % LX)) << 1) + (line1 % 2) ;
  col2 = (((U16)(index2 % LX)) << 1) + (line2 % 2) ;

  if (line1 > line2) {
    delta_line = line1 - line2 ;
  }
  else {
    delta_line = line2 - line1 ;
  }

  if (col1 > col2) {
    delta_col = col1 - col2 ;
  }
  else {
    delta_col = col2 - col1 ;
  }
  if (delta_col > LX) { /*to east or to west, whatever is shorter*/
    delta_col = LX2 - delta_col ;
  }

  if (delta_line > delta_col) {
    return 75 * delta_line + 25 * delta_col ;
  }
  else {
    return 75 * delta_col + 25 * delta_line ;
  }
}

/*-------------------->   direction_tile_to_tile   <------------- 2008-Mar-18
This function calculates the direction from one tile to another in a style
known from aviation (Heading 0..359).
0 = to north, 90 = to east ...
-----------------------------------------------------------------------------
Used functions:
Parameters:	- index_from
		- index_to
Return value:	Heading 0..359
Exitcode:	--
---------------------------------------------------------------------------*/
U16 direction_tile_to_tile( U32 from_index, U32 to_index )
{
THIS_FUNC(direction_tile_to_tile)
  U16 line1, line2, col1, col2 ;
  S16 delta_line, delta_col ;
  U16 heading ;
  double direction_radiant ;

  line1 = (U16)(from_index / LX) ;
  line2 = (U16)(to_index / LX) ;
  col1 = (((U16)(from_index % LX)) << 1) + (line1 % 2) ;
  col2 = (((U16)(to_index % LX)) << 1) + (line2 % 2) ;

  delta_line = line1 - line2 ; /*line numbers increasing downwards!!*/
  delta_col = col2 - col1 ;
  /*PRT_VAR(delta_col,d)*/
  if (delta_col > LX) { /*to east or to west, whatever is shorter*/
    delta_col -= LX2 ;
  }
  /*PRT_VAR(delta_col,d)*/
  if (-delta_col > (S16)LX) { /*to east or to west, whatever is shorter*/
    delta_col += LX2 ;
  }
  /*PRT_VAR(delta_col,d)*/
  /*PRT_VAR(delta_line,d)*/
  direction_radiant = atan2( (double)delta_col, (double)delta_line ) ;
  /*PRT_VAR(direction_radiant,lf)*/
  if (direction_radiant < 0.0) {
    direction_radiant += 6.2831852 ;
  }
  heading = (U16)(57.29578 * direction_radiant + 0.5 ) ;
  if (heading < 0 ) heading = 0 ; /*if rounding is not perfect ...*/
  if (heading > 359 ) heading = 359 ;
  return heading ;
}

/*-------------------->   tag_whole_area   <--------------------- 2013-Jun-14
This function tags all tiles which are in neighborhood/adjacent
to already tagged tiles and which are in TTL.
More than one tile may be tagged when calling this function, so more than one
area can be scanned with one single function call.
-----------------------------------------------------------------------------
Used functions:
Globals:   x
Internals: x
Parameters:
- area_mask, area_flags, list    define the area
- method                         ~NEIGHBORHOOD or ~ADJACENT
- tag_mask, untag_mask           what to do with the tiles
Return value:   cumulative number of tiles tagged (not including those
                tiles already tagged upon entry)
Exitcode:       x
---------------------------------------------------------------------------*/
U32 tag_whole_area( U8 area_mask, U8 area_flags, TTL list,
                    /*CLUSTER_METHOD method,*/ U8 tag_mask, U8 untag_mask )
{
THIS_FUNC(tag_whole_area)
  NEIGHBORHOOD NT ;
  U32 ret_val = 0 ;
  TIDX tile_index ;
  TIDX* p ; /*pointer to tile indices in NEIGHBORHOOD*/
  U8 i ; /*loop control*/
  U8 temp ;
  U8 set_mask = tag_mask | untag_mask ; /*shortcut*/
  BIT new_tiles_found ;


  PRT_VAR((unsigned)area_mask,02x)
  PRT_VAR((unsigned)area_flags,02x)
  PRT_VAR((unsigned long)list,lx)
  PRT_VAR((unsigned)tag_mask,02x)
  PRT_VAR((unsigned)untag_mask,02x)
  /*ASSERT(method == CLUSTER_METHOD_NEIGHBORHOOD)*/
                                         /*adjacent not yet impl.*/
  /*PRT_VAR((unsigned)(world_flags [300]),02x)*/
  do {
    new_tiles_found = FALSE ; /*default*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      /*PRT_VAR((unsigned long)tile_index,lu)*/
      if (IS_IN_TTL_AND_FLAGS_MATCH(tile_index, list, area_mask, area_flags)) {
        /*DEB((stderr, "setting neighborhood for %lu\n", (unsigned long)tile_index))*/
        set_neighborhood_tiles( tile_index, & NT ) ;
        for ( i = 0, p = & (NT.n_idx) ; i < 8 ; i++, p++ ) {
          if(*p < total_no_of_tiles) { /*inside map*/
            if (IS_IN_TTL_AND_FLAGS_MATCH( *p, list, area_mask, area_flags)) {
                 /*tag this tile, if not yet tagged*/
              temp = world_flags [*p] ;
              if ((temp & set_mask) != tag_mask) {
                /*DEB((stderr, "tagging %lu\n", (unsigned long)(*p)))*/
                new_tiles_found = TRUE ;
                ret_val++ ;
                temp |= tag_mask ;
                temp &= ~untag_mask ;
                world_flags [*p] = temp ;
              }
            }
          }
        } /*end neighborhood loop*/
           /*don't forget the central tile*/
        temp = world_flags [tile_index] ;
        if ((temp & set_mask) != tag_mask) {
          /*DEB((stderr, "tagging (central) %lu\n", (unsigned long)(tile_index)))*/
          new_tiles_found = TRUE ;
          ret_val++ ;
          temp |= tag_mask ;
          temp &= ~untag_mask ;
          world_flags [tile_index] = temp ;
        }
      }
    }
  } while (new_tiles_found) ;
  /*speedup with tile index lists !!*/
  /*PRT_VAR((unsigned long)ret_val,lu)*/
  return ret_val ;
}

/*-------------------->   tag_whole_TTL   <---------------------- 2010-Jun-22
This function tags all tiles which are in neighborhood to already tagged
tiles and which are in TTL.
More than one tile may be tagged when calling this function, so more than one
area can be scanned with one single function call.
-----------------------------------------------------------------------------
Used functions:
Globals/Internals:
Parameters:	- TTL of tiles to tag
		- mask: world flag to use for tagging
Return value:	--
Exitcode:	x
---------------------------------------------------------------------------*/
void tag_whole_TTL( TTL list, U8 mask )
{
THIS_FUNC(tag_whole_TTL)
  NEIGHBORHOOD NT ;
  TIDX tile_index ;
  TIDX* p ; /*pointer to tile indices in NEIGHBORHOOD*/
  U8 i ; /*loop control*/
  BIT new_tiles_found ;

  do {
    new_tiles_found = FALSE ; /*default*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      if (((world_flags [ tile_index ]) & mask) != 0) {
        set_neighborhood_tiles( tile_index, & NT ) ;
        for ( i = 0, p = & (NT.n_idx) ; i < 8 ; i++, p++ ) {
          if(*p < total_no_of_tiles) { /*inside map*/
            if (is_in_TTL( *p, list )) { /*tag this tile type*/
              if (((world_flags [*p]) & mask) == 0) { /*not yet tagged*/
                new_tiles_found = TRUE ;
                world_flags [*p] |= mask ;
              }
            }
          }
        }
      }
    }
    /*speedup should be possible if for loop is
      done again here in reverse order*/
  } while (new_tiles_found) ;
}

/*-------------------->   tag_whole_island   <---------------- 2007-Mar-18 --*/
/*-------------------->   tag_whole_land   <--------------------- 2010-May-14
This function tags all tiles which have direct land connection to a tile
which is already tagged.  ARCTIC is regarded as land.

More than one tile may be tagged when calling this function, so more than one
island can be scanned with one single function call.

When this function returns, there are only 2 types of land masses: Those
which have each of their TAGGED flags set and those which have not a single
TAGGED flag set.
-----------------------------------------------------------------------------
Used functions:
Globals/Internals: TAGGED world_flags
Parameters:	--
Return value:	--
Exitcode:	x
---------------------------------------------------------------------------*/
void tag_whole_land( void )
{
THIS_FUNC(tag_whole_land)
  /*NEIGHBORHOOD* NTp ;*/ /*not object oriented --- took 3 hours to find*/
  NEIGHBORHOOD NT ;
  TIDX tile_index ;
  TIDX* p ; /*pointer to tile indices in NEIGHBORHOOD*/
  U8 i ; /*loop control*/
  BIT new_tiles_found ;

  do {
    new_tiles_found = FALSE ; /*default*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      if ((world_flags [ tile_index ]) & TAGGED) {
        set_neighborhood_tiles( tile_index, & NT ) ;
        for ( i = 0, p = & (NT.n_idx) ; i < 8 ; i++, p++ ) {
          if(*p < total_no_of_tiles) { /*inside map*/
            if (is_land_tile( *p )) {
              if (((world_flags [*p]) & TAGGED) == 0) {
                new_tiles_found = TRUE ;
                world_flags [*p] |= TAGGED ;
              }
            }
          }
        }
      }
    }
    /*speedup should be possible if for loop is
      done again here in reverse order*/
  } while (new_tiles_found) ;
}

/*-------------------->   tag_whole_ocean   <-------------------- 2010-May-14
This function tags all tiles which have direct WATER connection to a tile
which is already tagged.

More than one tile may be tagged when calling this function, so more than one
ocean/lake can be scanned with one single function call.

When this function returns, there are only 2 types of water areas: Those
which have each of their 'mask' flags set and those which have not a single
'mask' flag set.
-----------------------------------------------------------------------------
Used functions:
Parameters:	mask   bit(s) of world_flags to be used for
                       this tagging operation
Return value:	--
Exitcode:	x
---------------------------------------------------------------------------*/
void tag_whole_ocean( U8 mask )
{
THIS_FUNC(tag_whole_ocean)
  NEIGHBORHOOD NT ;
  TIDX tile_index ;
  NEIGHBORHOOD* NTp ;
  TIDX* p ; /*pointer to tile indices in NEIGHBORHOOD*/
  U8 i ; /*loop control*/
  BIT new_tiles_found ;

  do {
    new_tiles_found = FALSE ; /*default*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      if ((world_flags [ tile_index ]) & mask) {
        NTp = set_neighborhood_tiles( tile_index, & NT ) ;
        for ( i = 0, p = & (NTp->n_idx) ; i < 8 ; i++, p++ ) {
          if(*p < total_no_of_tiles) {
            if (is_in_TTL( *p, TTL_WATER )) {
              if (((world_flags [*p]) & mask) == 0) {
                new_tiles_found = TRUE ;
                world_flags [*p] |= mask ;
              }
            }
          }
        }
      }
    }
    /*speedup should be possible if for loop is
      done again here in reverse order*/
  } while (new_tiles_found) ;
}

/*-------------------->   tag_inside_radius   <---------------- 2009-Jan-16*/
/*------------------>   untag_inside_radius   <---------------- 2012-Sep-25*/
/*This function (un)tags each tile which has "radius" or less distance to
"index" with "mask".
Example: If radius=500, tiles in dist 500, 450, ... are tagged, while
tiles in dist 550 are not.
-----------------------------------------------------------------------------
Used functions: dist_tile_to_tile
Parameters:	- center       center tile of circle
		- radius      in old MP (i. e. 100 = 1 tile)
		- mask        flag(s) to (un)tag
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
void tag_inside_radius( TIDX center, U16 radius, U8 mask )
{
THIS_FUNC(tag_inside_radius)
  TIDX tile_index ;

  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (dist_tile_to_tile( tile_index, center ) <= radius) {
      world_flags [tile_index] |= mask ;
    }
  }
}

void untag_inside_radius( TIDX center, U16 radius, U8 mask )
{
THIS_FUNC(untag_inside_radius)
  TIDX tile_index ;

  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (dist_tile_to_tile( tile_index, center ) <= radius) {
      world_flags [tile_index] &= ~mask ;
    }
  }
}

/*-------------------->   tag_whole_coast_water   <-------------- 2010-May-14
This function tags all COAST tiles which belong to one ore more islands.
The islands must be marked with the "island_mask" world_flags.
One tagged waterside tile is sufficient, since coast tagging propagates trough
the whole coast region of the island.
               OBS! as long as there is no big ARCTIC area on the island,
                    since ARCTIC might interrupt the COAST line!!
Tagging propagates even to 'neighborhood islands' which can be reached by
ships without navigation capability.

More than one tile may be tagged when calling this function, so more than one
island can be scanned with one single function call.

When this function returns, there are only 2 types of coast areas:
Those which have each of their "coast_mask" flags set
and those which have not a single "coast_mask" flag set.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- island_mask   island tiles are marked with this flag
                - coast_mask    use this flag to mark coast tiles
Return value:	--
Exitcode:	x
---------------------------------------------------------------------------*/
void tag_whole_coast_water( U8 island_mask, U8 coast_mask )
{
THIS_FUNC(tag_whole_coast_water)
  CITY_TILES CT ;
  NEIGHBORHOOD NT ;
  U32 tile_index ;
  CITY_TILES* CTp ;
  NEIGHBORHOOD* NTp ;
  TILE* Tp ;
  TIDX* idxp ; /*pointer to tile indices in NEIGHBORHOOD & CITY_TILES*/
  U8 i ; /*loop control*/
  BIT new_tiles_found ;

     /*step 1: tag all direct COAST tiles*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if ((world_flags [ tile_index ]) & island_mask) {
      CTp = set_city_tiles( tile_index, & CT ) ;
      for ( i = 0, idxp = & (CTp->i1), Tp = & (CTp->t1) ;
            i < 20 ; i++,
            idxp++, Tp++ ) {
        if ((*Tp & BASIC_TILE_TYPE_MASK) == COAST) {
          world_flags [ *idxp ] |= coast_mask ;
        }
      }
    }
  }
     /*step 2: propagate to neighborhood islands*/
  do {
    new_tiles_found = FALSE ; /*default*/
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      if ((world_flags [ tile_index ]) & coast_mask) {
        NTp = set_neighborhood_tiles( tile_index, & NT ) ;
        for ( i = 0, idxp = & (NTp->n_idx), Tp = & (NTp->n_tile) ;
              i < 8 ;
              i++, idxp++, Tp++ ) {
          if ((*Tp & BASIC_TILE_TYPE_MASK) == COAST) {
            if (((world_flags [*idxp]) & coast_mask) == 0) {
              new_tiles_found = TRUE ;
              world_flags [*idxp] |= coast_mask ;
            }
          }
        }
      }
    }
  } while (new_tiles_found) ;
}

/*-------------------->   tag_on_flag_pattern_match   <---------- 2013-Apr-14
This function modifies the world_flags for each tile which matches a certain
flag pattern.
It returns how many tiles it has modified.
-----------------------------------------------------------------------------
Used functions:
Globals:   x
Internals: x
Parameters:     - mask, match    define which tiles to modify
                - x
Return value:   x
Exitcode:       x
---------------------------------------------------------------------------*/
U32 tag_on_flag_pattern_match( U8 mask, U8 match, U8 set, U8 clr )
{
THIS_FUNC(tag_on_flag_pattern_match)
  U32 ret_val = 0 ;
  TIDX tidx ; /*loop control*/
  U8 temp ; /*scratch pad for world_flags*/

  for ( tidx = 0 ; tidx < total_no_of_tiles ; tidx++ ) {
    if ((world_flags [tidx] & mask) == match) {
      temp = world_flags [tidx] ;
      temp |= set ;
      temp &= ~clr ;
      world_flags [tidx] = temp ;
      ret_val++ ;
    }
  }
  return ret_val ;
}

/*-------------------->   count_adjacent_tiles   <--------------- 2012-Oct-01
This function counts how many tiles from 'list' are in the four adjacent
tiles.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- index  tile to inspect
		- list   list of tile types to count
Return value:	number of tiles from 'list' in neighborhood
Exitcode:	--
---------------------------------------------------------------------------*/
U8 count_adjacent_tiles( U32 index, TTL list )
{
THIS_FUNC(count_adjacent_tiles)
  NEIGHBORHOOD nbh ;
  U32 idx2 ;
  U8 count = 0 ;
  U8 i ; /*loop control*/

  set_neighborhood_tiles( index, & nbh ) ;
  for ( i = 1 ; i < 8 ; i += 2 ) {
    idx2 = ( & nbh.n_idx) [i] ;
    /*PRT_VAR(idx2,lu)*/
    if (idx2 < total_no_of_tiles) {
      if (list & tile2TTL( world [ idx2 ] )) {
        count++ ;
      }
    }
    else { /*!!count poles, too!!*/
      if (list & TTL_POLE) {
        count++ ;
      }
    }
  }
  return count ;
}

/*-------------------->   count_neighborhood_tiles   <----------- 2012-Oct-01
This function counts how many tiles from 'list' are in the eight neighborhood
tiles.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- index  tile to inspect
		- list   list of tile types to count
Return value:	number of tiles from 'list' in neighborhood
Exitcode:	--
---------------------------------------------------------------------------*/
U8 count_neighborhood_tiles( U32 index, TTL list )
{
THIS_FUNC(count_neighborhood_tiles)
  U32 idx2 ;
  NEIGHBORHOOD nbh ;
  U8 count = 0 ;
  U8 i ; /*loop control*/

  set_neighborhood_tiles( index, & nbh ) ;
  for ( i = 0 ; i < 8 ; i++ ) {
    idx2 = ( & nbh.n_idx) [i] ;
    /*PRT_VAR(idx2,lu)*/
    if (idx2 < total_no_of_tiles) {
      if (list & tile2TTL( world [ idx2 ] )) {
        count++ ;
      }
    }
    else { /*!!count poles, too!!*/
      if (list & TTL_POLE) {
        count++ ;
      }
    }
  }
  return count ;
}

/*-------------------->   count_city_tiles   <------------------- 2012-Oct-01
This function counts how many tiles from 'list'
are in the twenty city radius tiles.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- index  tile to inspect
		- list   list of tile types to count
Return value:	number of tiles from 'list' in city radius
Exitcode:	--
---------------------------------------------------------------------------*/
U8 count_city_tiles( TIDX index, TTL list )
{
THIS_FUNC(count_city_tiles)
  TIDX idx2 ;
  CITY_TILES CT ;
  U8 count = 0 ;
  U8 i ; /*loop control*/

  set_city_tiles( index, & CT ) ;
  for ( i = 0 ; i < 20 ; i++ ) {
    idx2 = ( & CT.i1) [i] ;
    /*PRT_VAR(idx2,lu)*/
    if (idx2 < total_no_of_tiles) {
      if (list & tile2TTL( world [ idx2 ] )) {
        count++ ;
      }
    }
    else { /*!!count poles, too!!*/
      if (list & TTL_POLE) {
        count++ ;
      }
    }
  }
  return count ;
}

/*-------------------->   is_in_TTL   <-------------------------- 2008-Mar-21
This function tells if a certain tile is of a type named in "list".
Out-of-range (pole) indices are allowed and correctly handled.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- index  ... of tile to test
		- list  ... of tile types
Return value:	TRUE if in list
Exitcode:	--
---------------------------------------------------------------------------*/
BIT is_in_TTL( TIDX index, TTL list )
{
THIS_FUNC(is_in_TTL)
  TILE tile ;

  /*PRT_VAR(list,08lx)*/
  if (index > total_no_of_tiles) {
    tile = NORTH_POLE ; /*resp. SOUTH_POLE*/
  }
  else {
    tile = world [ index ] ;
  }
  /*PRT_VAR(tile,08lx)*/
  if (tile2TTL( tile ) & list) {
    /*DEB((stderr,"TRUE\n"))*/
    return TRUE ;
  }
  /*DEB((stderr,"FALSE\n"))*/
  return FALSE ;
}

/*-------------------->   tile2TTL   <--------------------------- 2016-Jul-15
This function converts a TILE to its TTL pattern.
RIVER is handled.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- tile   tile to convert
Return value:	TTL pattern of input tile
Exitcode:	EXITCODE_UNEXPECTED_ERR
---------------------------------------------------------------------------*/
static TTL tile2TTL( TILE tile )
{
THIS_FUNC(tile2TTL)
  TTL ret_val ;

  /*PRT_VAR(tile & BASIC_TILE_TYPE_MASK,lu)*/
  switch (tile & BASIC_TILE_TYPE_MASK) {
  case OCEAN:
    ret_val = TTL_OCEAN ;
    break ;

  case COAST:
    ret_val = TTL_COAST ;
    break ;

  case GRASSLAND:
    ret_val = TTL_GRASSLAND ;
    break ;

  case DESERT:
    ret_val = TTL_DESERT ;
    break ;

  case PRAIRIE:
    ret_val = TTL_PRAIRIE ;
    break ;

  case TUNDRA:
    ret_val = TTL_TUNDRA ;
    break ;

  case ARCTIC:
    ret_val = TTL_ARCTIC ;
    break ;

  case SWAMP:
    ret_val = TTL_SWAMP ;
    break ;

  case FOREST:
    ret_val = TTL_FOREST ;
    break ;

  case HILLS:
    ret_val = TTL_HILLS ;
    break ;

  case MOUNTAINS:
    ret_val = TTL_MOUNTAINS ;
    break ;

  case (NORTH_POLE & BASIC_TILE_TYPE_MASK):
  case (SOUTH_POLE & BASIC_TILE_TYPE_MASK):
    /*DEB((stderr,"case pole\n"))*/
    ret_val = TTL_POLE ;
    break ;

  default:
    fprintf( stderr, "tile2TTL: unexpected tile: %08lx\n",
                     (unsigned long)tile) ;
    exit( EXITCODE_UNEXPECTED_ERR ) ;
  }

  if (tile & RIVER) {
    ret_val |= TTL_RIVER ;
  }
  /*PRT_VAR(ret_val,08lx)*/
  return ret_val ;
}

/*-------------------->   clear_flags   <--------------------- 2007-Mar-20 --
This function clears all world_flags given in "mask".
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
void clear_flags( U8 mask )
{
THIS_FUNC(clear_flags)
  U32 tile_index ;

  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    world_flags [ tile_index ] &= ~mask ;
  }
}

/*-------------------->   tag_city_tiles   <--------------------- 2008-Apr-06
This function tags all "world_flags" in city radius of "index"
-----------------------------------------------------------------------------
Used functions:
Globals:   x
Internals: x
Parameters: - x
            - x
Return value: x
Exitcode:     x
---------------------------------------------------------------------------*/
void tag_city_tiles( U32 index, U8 mask )
{
THIS_FUNC(tag_city_tiles)
  CITY_TILES CT ;
  U32 idx2 ;
  U8 i ; /*loop control*/

  set_city_tiles( index, & CT ) ;
  for ( i = 0 ; i < 20 ; i++ ) {
    idx2 = ( & (CT.i1)) [i] ;
    if (idx2 < total_no_of_tiles) { /*no pole*/
      world_flags [ idx2 ] |= mask ;
    }
  }
  world_flags [ index ] |= mask ; /*don't forget the center tile*/
}

/*-------------------->   tag_neighborhood_tiles   <------------- 2015-Jun-06
This function tags all "world_flags" in neighborhood of "index"
-----------------------------------------------------------------------------
Used functions: set_neighborhood_tiles
Globals:   world []
Internals: --
Parameters: - index    ... of the central tile
            - mask     these are the world_flags to set
Return value: void
Exitcode:     --
---------------------------------------------------------------------------*/
void tag_neighborhood_tiles( U32 index, U8 mask )
{
THIS_FUNC(tag_neighborhood_tiles)
  NEIGHBORHOOD NT ;
  TIDX idx2 ;
  U8 i ; /*loop control*/

  set_neighborhood_tiles( index, & NT ) ;
  for ( i = 0 ; i < 8 ; i++ ) {
    idx2 = ( & (NT.n_idx)) [i] ;
    if (idx2 < total_no_of_tiles) { /*no pole*/
      world_flags [ idx2 ] |= mask ;
    }
  }
  world_flags [ index ] |= mask ; /*don't forget the center tile*/
}

/*-------------------->   tag_adjacent_tiles   <----------------- 2015-Jun-06
This function tags all "world_flags" adjacent to "index"
-----------------------------------------------------------------------------
Used functions: set_neighborhood_tiles
Globals:   world []
Internals: --
Parameters: - index    ... of the central tile
            - mask     these are the world_flags to set
Return value: void
Exitcode:     --
---------------------------------------------------------------------------*/
void tag_adjacent_tiles( U32 index, U8 mask )
{
THIS_FUNC(tag_adjacent_tiles)
  NEIGHBORHOOD NT ;
  TIDX idx2 ;
  U8 i ; /*loop control*/

  set_neighborhood_tiles( index, & NT ) ;
  for ( i = 1 ; i < 8 ; i += 2 ) {
    idx2 = ( & (NT.n_idx)) [i] ;
    if (idx2 < total_no_of_tiles) { /*no pole*/
      world_flags [ idx2 ] |= mask ;
    }
  }
  world_flags [ index ] |= mask ; /*don't forget the center tile*/
}

/*-------------------->   x   <---------------------------------- 2015-Jun-06
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
