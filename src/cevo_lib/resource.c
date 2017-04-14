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
resource.c

This file 
*****************************************************************************
History: (latest change first)
2017-Feb-27: changed include pathes for source code release
2016-Aug-17..2017-Feb-18: clean log file implementation
2016-Jul-15: adoptions for gcc
2015-May-14: Debugging: Special resource always on DESERT tile
2012-Sep-25: changed "rate_1st_pos" to correspond to docu
2012-Mar-13: Debugging (2 special resources on same tile)
2012-Feb-10: - conditionally includes "map_gen.h" if MAP_GEN is defined
             - included "string.h" for "memcpy"
2010-Jul-15..27: rework of "rate_1st_pos"
2010-Jul-14: Bug in "rate_1st_pos" fixed (central tile food)
2010-Jun-22: rate_1st_pos now uses TEMP_TAG
2010-May-14; less NULL pointer usage
2010-May-02: Debugging (and make set_ixxx_cnt obsolete)
2010-Apr-21..22: added func "rate_1st_pos"
2010-Apr-14: adapted to TinyC (const void* for sort functions)
2009-Jan-14: modified material rating in "rateXXX_start_pos"
2008-Aug-30: Debugging
2008-Jul-13: "set_ixxx_cnt" and "potential_tile_resources" are now
             'special-resource-proof'
2008-Apr-06: "set_ixxx_cnt" and "rateXXX_start_pos" are now 'pole-proof'
             "potential_tile_resources" now handles DEAD_LANDS and RIVER
2007-Jun-17: Better rating (avoid places with only few materials)
2007-May-21: Bug in "set_special_resource" fixed
2007-May-13: "set_ixxx_cnt" and "potential_tile_resources"
             improved for reuseability
2007-Mar-19..20: Rating for start pos improved
2007-Mar-18: derived from "cevo_lib.c"
*****************************************************************************
Global objects:
- rate_1st_pos()  --> nur fuer map_gen notwendig
- rateXXX_start_pos()
- set_special_resource()  --> nur fuer map_gen notwendig
- central_tile_resources()
;- potential_tile_resources()
;- early_potential_tile_resources()
;- initial_tile_resources()
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

   /*switches on additional functions which are not necessary for an AI*/
#define MAP_GEN


/*--  include files  ------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h> /*for qsort*/
#include <string.h> /*for memcpy*/

#ifdef MAP_GEN
# ifdef unix
#  include "../map_gen/map_gen.h" /*for map_gen_exit()*/
# else
#  include "..\map_gen\map_gen.h" /*for map_gen_exit()*/
# endif
#endif

#include "cevo_map.h"
/*#include <misc.h>*/
#include <falloc.h>

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/



/*--  type declarations & enums  ------------------------------------------*/

typedef struct {
  TIDX idx ;
  RESOURCES res ;
} IDX_TABLE_ELEM ;

   /*modifiers for resource values*/
typedef struct {
  EPOCH epoch ; /*bonus2*/
  GOV gov ; /*limits*/

  BIT center_tile ;
  BIT irrigation ;
  BIT farmland ;
  BIT road ;
  BIT railroad ;
  BIT mining ;

  BIT forest2prairie ; /*treat each plain forest as prairie*/
  BIT swamp2grassland ; /*treat each plain swamp as grassland/plains*/

  BIT townhall ;
  BIT courthouse ; /*or palace*/
  BIT harbor ;
  BIT offshore_platform ;
  BIT highways ;

  BIT wheel ;
  BIT map_making ;

  BIT lighthouse ;
  BIT space_port ;
} MODIFIERS ;


/*--  local function prototypes  ------------------------------------------*/

static void tile_resources( TILE tile, MODIFIERS* mod, RESOURCES* dest ) ;
static void fill_idx_table( CITY_TILES* p, MODIFIERS* mod,
                            U8 dont_use_mask, U8 improve_mask ) ;
static int idx_sort_max_food( const void* elem1, const void* elem2 ) ;
static int idx_sort_max_mat( const void* elem1, const void* elem2 ) ;
static int idx_sort_max_sum( const void* elem1, const void* elem2 ) ;


/*--  macros  -------------------------------------------------------------*/

   /*for debugging only*/
#define PRT_TAB \
   if (debug_trace_flag) {  \
     fprintf( log_fp, "-food-mat-trade---tidx-------\n") ;  \
     for (debug_i=0 ; debug_i<20 ; debug_i++) {  \
       fprintf( log_fp, "%u %u %u  %lu\n",  \
           (unsigned)((idx_table [debug_i]).res.food),  \
           (unsigned)((idx_table [debug_i]).res.mat),  \
           (unsigned)((idx_table [debug_i]).res.trade),  \
                      (idx_table [debug_i]).idx ) ;  \
     }  \
     fprintf( log_fp, "-----------------------------\n") ;  \
   }


/*--  global variables  ---------------------------------------------------*/



/*--  internal variables  -------------------------------------------------*/

static RESOURCES total ;
static RESOURCES central_tile ;
static IDX_TABLE_ELEM idx_table [20] ;
DEB_STATEMENT(static U32 debug_i ;)

static BIT is_coast_city ;
static BIT respect_dont_use ;

static MODIFIERS common_mod ;


/*--  library functions  --------------------------------------------------*/

#ifdef MAP_GEN
/*-------------------->   rate_1st_pos   <----------------------- 2016-Aug-17
This function does a rating for the first city of a nation.
Algorithm:
1. 4 Tiles must produce >= 8 food, or the city will starve. => if not, 0 points
   (under DESPOTISM & not irrigated)
2. With irrigation, 4 tiles should produce 9/10/11/12 food
   (only tiles with land connection to center tile, still despotism)
    8 food =>  0 points  -- limit 8
    9 food =>  1 points  -- limit 10
   10 food => 12 points  -- limit 41
   11 food => 18 points  -- no limit
   12 food => 24 points  -- no limit
3. With mine, top 4 material tiles should have 3 materials
   (only tiles with land connection to center tile, still despotism)
   0  materials =>  0 points  -- limit 8
   1  materials =>  1 points  -- limit 10
   2  materials =>  2 points  -- limit 20
   3  materials => 15 points  -- limit 41
   4+ materials => 15 points  -- no limit
4. Top 4 sum producing tiles (1 point each resource -> approx. 24 points)
   (irrigation, roads/railroads, mines and HARBOR under MONARCHY with WHEEL
   and MAP_MAKING)
An acceptable starting position should have 10 food and 3 materials
=> 41 points
To be implemented:
- It is important that the 1st city can grow to size 8 -- to be tested!
- It is less important (but appreciated) that the 1st city can grow further
-----------------------------------------------------------------------------
Used functions:
Globals:        total_no_of_tiles
Internals:      idx_table
Parameters:	- tile_idx: Index of tile to test
Return value:
  0 - game engine prohibits to settle down here
  0 - city will starve, but 2 tiles produce 4 food
  0 - city will starve, but 3 tiles produce 6 food
 >0 - see above and below
Exitcode:	EXITCODE_UNEXPECTED_ERR
---------------------------------------------------------------------------*/
U16 rate_1st_pos( TIDX tile_idx )
{
/*THIS_FUNC(rate_1st_pos)*/
char* _this_func = "rate_1st_pos" ; /*for WORLD_FLAGS*/
  CITY_TILES CT ;
  TILE tile ;
  TIDX idx2 ;
  RESOURCES* central ;
  RESOURCES* Rp ;
  U16 rating, limit, penalty ;
  U8 i ; /*loop control*/


  /*PRT_VAR((unsigned)tile_idx,lu)*/
  tile = world [ tile_idx ] ;
  switch (tile & BASIC_TILE_TYPE_MASK) {
  case GRASSLAND:
  case PRAIRIE:
    limit = 999 ;
    break ; /*ok*/

  case TUNDRA:
  case HILLS:
    limit = 10 ; /*we dont like starting here*/
    break ; /*ok*/

  default:
    return 0 ; /*game engine prohibits to settle down here*/
  }


  common_mod.epoch = EARLY_AGE ;
  common_mod.gov = DESPOTISM ;

  common_mod.center_tile = FALSE ;
  common_mod.irrigation = FALSE ;
  common_mod.farmland = FALSE ;
  common_mod.road = FALSE ;
  common_mod.railroad = FALSE ;
  common_mod.mining = FALSE ;

  common_mod.forest2prairie = FALSE ;
  common_mod.swamp2grassland = FALSE ;

  common_mod.townhall = FALSE ;
  common_mod.courthouse = TRUE ; /*palace*/
  common_mod.harbor = FALSE ;
  common_mod.offshore_platform = FALSE ;
  common_mod.highways = FALSE ;

  common_mod.wheel = FALSE ;
  common_mod.map_making = FALSE ;

  common_mod.lighthouse = FALSE ;
  common_mod.space_port = FALSE ;


  set_city_tiles( tile_idx, & CT ) ;

  central = central_tile_resources( tile, EARLY_AGE /*no bonus 2*/ ) ;
  /*PRT_VAR((unsigned)(central->food),u)*/
     /*does count irrigation of central tile, but does not limit food to 3*/
  if (central->food > 3) {
    central->food = 3 ; /*1st city starts with DESPOTISM*/
  }
  /*central->trade = 0 ;*/

  /*respect_dont_use = FALSE ;*/
  fill_idx_table( & CT, & common_mod,
                  0, /*no dont_use tiles, you may use all tiles*/
                  0  /*no tile improvements yet*/
                ) ;
  qsort( & idx_table, 20, sizeof(IDX_TABLE_ELEM), idx_sort_max_food ) ;
  /*DEB_STATEMENT((PRT_TAB))*/
  total.food = central->food + (idx_table [0]).res.food
                             + (idx_table [1]).res.food
                             + (idx_table [2]).res.food ;
  /*PRT_VAR((unsigned)(total.food),u)*/
  if (total.food < 8) {
    return 0 ;
  }

     /*decide which tiles can be improved early,
       i. e. which tiles have land connection*/
  WORLD_FLAGS_ALLOC( TEMP_TAG )
  clear_flags( TEMP_TAG ) ;
  world_flags [ tile_idx ] |= TEMP_TAG ; /*tag city center tile*/
  tag_whole_TTL( TTL_LAND, TEMP_TAG ) ;
     /*now all tiles with land connection to the center tile are tagged*/
  common_mod.wheel = TRUE ;
  common_mod.map_making = TRUE ;
  common_mod.road = TRUE ;
  common_mod.irrigation = TRUE ;
  common_mod.mining = TRUE ;
  /*PRT_VAR((unsigned)(common_mod.gov),u)*/
  fill_idx_table( & CT, & common_mod,
                  0, /*no dont_use tiles*/
                  TEMP_TAG /*early tile improvements*/
                ) ;
  WORLD_FLAGS_FREE( TEMP_TAG )
  qsort( & idx_table, 20, sizeof(IDX_TABLE_ELEM), idx_sort_max_food ) ;
  /*DEB_STATEMENT((PRT_TAB))*/
  total.food = central->food + (idx_table [0]).res.food
                             + (idx_table [1]).res.food
                             + (idx_table [2]).res.food ;
  /*PRT_VAR((unsigned)(total.food),u)*/
  switch (total.food) {
  case 8:
    rating = 0 ;
    limit = min( limit, 8 ) ;
    break ;

  case 9:
    rating = 1 ;
    limit = min( limit, 10 ) ;
    break ;

  case 10:
    limit = min( limit, 41 ) ;
    rating = 12 ;
    break ;

  case 11:
    rating = 18 ;
    break ;

  case 12:
    rating = 24 ;
    break ;

  default:
    fprintf( log_fp, "4-tile-food is %u\n", (unsigned)(total.food) ) ;
    map_gen_exit() ;
  }
  /*PRT_VAR((unsigned)(limit),u)*/
  /*PRT_VAR((unsigned)(rating),u)*/

  qsort( & idx_table, 20, sizeof(IDX_TABLE_ELEM), idx_sort_max_mat ) ;
  /*DEB_STATEMENT((PRT_TAB))*/
  total.mat = central->mat + (idx_table [0]).res.mat
                           + (idx_table [1]).res.mat
                           + (idx_table [2]).res.mat ;
  /*PRT_VAR((unsigned)(total.mat),u)*/
  switch (total.mat) {
  case 0:
    /*rating += 0 ;*/
    return 0 ;
    break ;

  case 1:
    rating += 1 ;
    limit = min( limit, 3 ) ;
    break ;

  case 2:
    rating += 2 ;
    limit = min( limit, 8 ) ;
    break ;

  default: /*3 or more materials*/
    rating += 15 ;
  }
  /*PRT_VAR((unsigned)(limit),u)*/
  /*PRT_VAR((unsigned)(rating),u)*/


  is_coast_city = count_neighborhood_tiles( tile_idx, TTL_WATER ) > 0 ;


     /*find the top 4 sum producing tiles*/
  common_mod.gov = MONARCHY ;
  /*common_mod.epoch = MODERN_AGE ;*/
  /*common_mod.railroad = TRUE ;*/
  WORLD_FLAGS_ALLOC( TEMP_TAG )
  clear_flags( TEMP_TAG ) ;
  tag_city_tiles( tile_idx, TEMP_TAG ) ; /*all tiles reachable*/
  fill_idx_table( & CT, & common_mod, 0 /*no dont_use tiles*/, TEMP_TAG ) ;
  qsort( & idx_table, 20, sizeof(IDX_TABLE_ELEM), idx_sort_max_sum ) ;
  /*DEB_STATEMENT((PRT_TAB))*/
  total.food  = (idx_table [0]).res.food
              + (idx_table [1]).res.food
              + (idx_table [2]).res.food
              + (idx_table [3]).res.food ;
  total.mat   = (idx_table [0]).res.mat
              + (idx_table [1]).res.mat
              + (idx_table [2]).res.mat
              + (idx_table [3]).res.mat ;
  total.trade = (idx_table [0]).res.trade
              + (idx_table [1]).res.trade
              + (idx_table [2]).res.trade
              + (idx_table [3]).res.trade ;
  rating += total.food + total.mat + total.trade ;
  /*PRT_VAR((unsigned)(limit),u)*/
  /*PRT_VAR((unsigned)(rating),u)*/


     /*can city grow to size 8?*/
  common_mod.harbor = TRUE ;
  common_mod.forest2prairie = TRUE ;
  common_mod.swamp2grassland = TRUE ;
  fill_idx_table( & CT, & common_mod,
                  0, /*no dont_use tiles*/
                  TEMP_TAG /*early tile improvements*/
                ) ;
  WORLD_FLAGS_FREE( TEMP_TAG )
  qsort( & idx_table, 20, sizeof(IDX_TABLE_ELEM), idx_sort_max_food ) ;
  total.food = central->food ;
  for ( i = 0 ; i < 7 ; i++) {
    total.food += (idx_table [i]).res.food ;
  }
  if (total.food < 16) { /*1st city should be able to grow to size 8*/
    /*return 0 ;*/
    limit = 40 ;
  }




  if ( ! is_coast_city) { /*max. penalty 12 * 5 points*/
    penalty = 5 * (U16)count_city_tiles( tile_idx, TTL_WATER ) ;
    if (penalty > rating) {
      rating = 0 ;
    }
    else {
      rating -= penalty ;
    }
  }
  rating = min( limit, rating ) ;
  return rating ;
}

/*-------------------->   set_special_resource   <--------------- 2017-Feb-18
This function sets a special resource in the city radius of the given
tile index.  It uses the land tile with the least resources,
but not one of the tiles which produce the maximum food, if possible.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- tile_index    index for possible city place
		- special_mask  determines type of special resource
		                (to be OR'ed with a world tile)
Return value:	void
Exitcode:	x
---------------------------------------------------------------------------*/
void set_special_resource( U32 tile_index, U32 special_mask )
{
THIS_FUNC(set_special_resource)
  CITY_TILES CT ;
  MODIFIERS local_mod ;
  TILE* Tp ;
  U32* idxp ;
  RESOURCES* Rp ;
  TIDX temp_idx ;
  U8 land_tiles ;
  S8 i ; /*loop control*/


  local_mod.epoch = MODERN_AGE ;
  local_mod.gov = MONARCHY ;

  local_mod.center_tile = FALSE ;
  local_mod.road = TRUE ;
  local_mod.railroad = TRUE ;
  local_mod.irrigation = TRUE ;
  local_mod.farmland = TRUE ;
  local_mod.mining = TRUE ;

  local_mod.forest2prairie = TRUE ;
  local_mod.swamp2grassland = TRUE ;

  local_mod.courthouse = TRUE ;
  local_mod.harbor = TRUE ;
  local_mod.offshore_platform = TRUE ;
  local_mod.highways = TRUE ;

  local_mod.wheel = TRUE ;
  local_mod.map_making = TRUE ;

  local_mod.lighthouse = FALSE ;
  local_mod.space_port = TRUE ;


  /*PRT_VAR(tile_index,lu)*/
  set_city_tiles( tile_index, & CT ) ;
  land_tiles = 0 ; /*count how many land tile are in city radius*/
  for ( i = 0, Tp = &(CT.t1), idxp = &(CT.i1) ;
        i < 20 ;
        i++, Tp++, idxp++ ) {
    switch ( *Tp & BASIC_TILE_TYPE_MASK ) {
    case COAST:
    case NORTH_POLE & BASIC_TILE_TYPE_MASK:
    case SOUTH_POLE & BASIC_TILE_TYPE_MASK:
      break ;

    default:
      tile_resources( *Tp, & local_mod, & ((idx_table [land_tiles]).res) ) ;
      (idx_table [land_tiles]).idx = *idxp ;
      land_tiles++ ;
    }
  }
  /*PRT_TAB*/
  if (land_tiles == 0) {
    fprintf( stderr,
      "cannot place special resource -- no land tile available (%lu)\n",
      (unsigned long)tile_index ) ;
    fprintf( log_fp,
      "cannot place special resource -- no land tile available (%lu)\n",
      (unsigned long)tile_index ) ;
    map_gen_exit() ;
  }
  /*PRT_VAR((unsigned)land_tiles,u)*/
  qsort( & idx_table, land_tiles, sizeof(IDX_TABLE_ELEM), idx_sort_max_sum ) ;
  qsort( & idx_table, land_tiles, sizeof(IDX_TABLE_ELEM), idx_sort_max_food ) ;
       /*major sort criterium: food*/
       /*minor sort criterium: total production*/
  /*PRT_TAB*/


  i = land_tiles -1 ; /*start with last table entry (= least resources)*/
  while( i >= 0 ) {
    temp_idx = (idx_table [ i ]).idx ; /*drop special here*/
    if (((world [ temp_idx ]) & SPECIAL_MASK) != 0) { /*already used*/
      i-- ; /*try previous entry*/
      fprintf( log_fp, "set_special_resource: trying another tile\n" ) ;
      continue ;
    }
    fprintf( log_fp,
      "set_special_resource: replacing tile type %08lx at loc. code %lu\n",
      (unsigned long)(world [ temp_idx ]) & (BASIC_TILE_TYPE_MASK | BONUS_RESOURCES_MASK),
      (unsigned long)temp_idx ) ;
    /*world [ temp_idx ] |= special_mask ;*/
    world [ temp_idx ] &= BASIC_TILE_TYPE_MASK ; /*make basic tile DESERT*/
    world [ temp_idx ] |= (DESERT | special_mask) ;
    return ;
  }
  /*no suitable entry found -- abort*/

  fprintf( log_fp, "set_special_resource: all land tiles already used\n" ) ;
  map_gen_exit() ;
}
#endif /*MAP_GEN*/

/*-------------------->   tile_resources   <--------------------- 2016-Aug-17
This function returns the theoretical production of a tile
according to 'environmental' conditions.
It is e. g. assumed that a mine can be built; the production with a mine
is calculated no matter if there is a mine or not.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- tile: the tile to evaluate
		- mod:  the modifiers to apply
		- dest: the result is stored here
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
static void tile_resources( TILE tile, MODIFIERS* mod, RESOURCES* dest )
{
THIS_FUNC(tile_resources)
  TILE switch_tile ;
  U8 bonus_factor = (mod->space_port) ? 2 : 1 ; /*space port doubles bonus*/
  BIT bonus1, bonus2 ;
  BIT extra_trade = (mod->wheel && (mod->road || mod->railroad)) ;


  /*PRT_VAR((unsigned long)tile,lu)*/
  dest->food = 0 ; /*defaults*/
  dest->mat = 0 ;
  dest->trade = 0 ;

  if (tile == DUMMY_TILE) {
    return ;
  }

  bonus1 = (tile & BONUS_RESOURCE_1_MASK) != 0 ;
  bonus2 = (tile & BONUS_RESOURCE_2_MASK) != 0 ;
     /*bonus resource type 2 is not effective in the early ages*/
  if (mod->epoch == EARLY_AGE) {
    bonus2 = FALSE ;
  }


  switch_tile = tile & (DEAD_LANDS | BASIC_TILE_TYPE_MASK) ;
  if ((tile & DEAD_LANDS) != 0) {
    dest->mat = 1 ;
    dest->trade = 1 ;
  }
  else switch (switch_tile) {

  case COAST:
    dest->food = 1 ;
    if (bonus1) { /*fish, +4 food*/
      dest->food += bonus_factor * 4 ;
    }
    if (bonus2) { /*manganese, +5 material*/
      dest->mat = bonus_factor * 5 ;
    }
    dest->trade = 3 ;

    if (mod->harbor) {
      dest->food++ ;
    }
    if (mod->offshore_platform) {
      dest->mat++ ;
    }
    if (mod->lighthouse) {
      dest->mat++ ;
    }
    break ;

  case GRASSLAND:
    if (bonus1) { /* = plains*/
treat_as_grassland:
      dest->food = 2 ;
      dest->mat = 1 ;
      dest->trade = 1 ;
    }
    else {
      dest->food = 3 ;
      dest->trade = 1 ;
    }

    if (extra_trade) {
      (dest->trade)++ ;
    }
    if (mod->irrigation) {
      dest->food++ ;
    }
    if (mod->farmland) {
      dest->food += (dest->food / 2) ;
    }
    break ;

  case DESERT:
    dest->mat = 1 ;
    if (bonus1) { /*oasis, +3 food*/
      dest->food = bonus_factor * 3 ;
    }
    if (bonus2) { /*oil, +3 material*/
      dest->mat += bonus_factor * 3 ;
    }
    dest->trade = 1 ;

    if (mod->mining) {
      dest->mat++ ;
    }

    if (extra_trade) {
      (dest->trade)++ ;
    }
    if (mod->railroad) {
      dest->mat += (dest->mat / 2) ;
    }
    break ;
  
  case PRAIRIE:
treat_as_prairie:
    dest->food = 1 ;
    dest->mat = 1 ;
    dest->trade = 1 ;
    if (bonus1) { /*wheat, +2 food*/
      dest->food += bonus_factor * 2 ;
    }
    if (bonus2) { /*bauxite, +2 material*/
      dest->mat += bonus_factor * 2 ;
    }

    if (extra_trade) {
      (dest->trade)++ ;
    }
    if (mod->irrigation) {
      dest->food++ ;
    }
    if (mod->farmland) {
      dest->food += (dest->food / 2) ;
    }
    if (mod->railroad) {
      dest->mat += (dest->mat / 2) ;
    }
    break ;
  
  case TUNDRA:
    dest->food = 1 ;
    dest->trade = 1 ;
    if (bonus1) { /*gold, +5 trade*/
      dest->trade += bonus_factor * 5 ;
    }
    if (bonus2) { /*natural gas, +4 material*/
      dest->mat = bonus_factor * 4 ;
    }

    if (extra_trade) {
      (dest->trade)++ ;
    }
    if (mod->irrigation) {
      dest->food++ ;
    }
    if (mod->farmland) {
      dest->food += (dest->food / 2) ;
    }
    if (mod->railroad) {
      dest->mat += (dest->mat / 2) ;
    }
    break ;
  
  case ARCTIC:
    dest->mat = 1 ;
    if (mod->mining) {
      dest->mat = 4 ;
    }
    if (bonus1 || bonus2) { /*ivory, +3 food, +4 trade*/
      dest->food = bonus_factor * 3 ;
      dest->trade = bonus_factor * 4 ;
    }

    /*if (extra_trade) {*/
      /*(dest->trade)++ ;*/
    /*}*/
    if (mod->railroad) {
      dest->mat += (dest->mat / 2) ;
    }
    break ;
  
  case SWAMP:
    dest->food = 1 ;
    dest->trade = 1 ;
    if (bonus1) { /*peat, +4 material*/
      dest->mat = bonus_factor * 4 ;
    }
    else if (bonus2) { /*cotton, +1 material, +4 trade*/
      dest->mat = bonus_factor * 1 ;
      dest->trade += bonus_factor * 4 ;
    }
    else { /*irrigation to GRASSLAND !!*/
      if (mod->swamp2grassland) {
        goto treat_as_grassland ; /*Grassland or plain???*/
      }
    }

    if (extra_trade) {
      (dest->trade)++ ;
    }
    if (mod->railroad) {
      dest->mat += (dest->mat / 2) ;
    }
    break ;
  
  case FOREST:
    dest->food = 1 ;
    dest->mat = 2 ;
    dest->trade = 1 ;
    if (bonus1) { /*game, +2 food*/
      dest->food += bonus_factor * 2 ;
    }
    else if (bonus2) { /*silk, +3 trade*/
      dest->trade += bonus_factor * 3 ;
    }
    else {
      if (mod->forest2prairie) {
        goto treat_as_prairie ;
      }
    }
    if (mod->railroad) {
      dest->mat += (dest->mat / 2) ;
    }
    break ;

  case HILLS:
    dest->food = 1 ;
    if (mod->mining) {
      dest->mat = 3 ;
    }
    if (bonus1) { /*wine, +4 trade*/
      dest->trade = bonus_factor * 4 ;
    }
    if (bonus2) { /*coal, +2 material*/
      dest->mat += bonus_factor * 2 ;
    }
    if (mod->railroad) {
      dest->mat += (dest->mat / 2) ;
    }
    break ;

  case MOUNTAINS:
    dest->mat = 1 ;
    if (mod->mining) {
      dest->mat = 3 ;
    }
    if (bonus1) { /*iron, +3 material*/
      dest->mat += bonus_factor * 3 ;
    }
    if (bonus2) { /*diamonds, +7 trade*/
      dest->trade = bonus_factor * 7 ;
    }
    if (mod->railroad) {
      dest->mat += (dest->mat / 2) ;
    }
    break ;
  
  case DEAD_LANDS:
    /*dest->mat = 1 ;*/
    /*dest->trade = 1 ;*/
    /*break ;*/
    fprintf( log_fp, "tile_resources: unexpected DEAD_LANDS\n" ) ;
    map_gen_exit() ;

  default:
    fprintf( log_fp,
             "tile_resources: unexpected tile 0x%08lx (switch_tile=0x%08lx)\n",
                      (long)tile, (long)switch_tile ) ;
    map_gen_exit() ;
  } /*end switch   switch_tile*/

  if (mod->map_making) {
    if (tile & RIVER) {
      (dest->trade)++ ;
    }
  }



     /*Limitations*/
  if ( ! mod->courthouse) { /*or palace*/
    if (dest->trade > 3) {
      dest->trade = 3 ;
    }
    if ( ! mod->townhall) {
      dest->trade = 0 ;
    }
  }

  if (mod->center_tile) {
    dest->trade = 0 ;
  }

  if (mod->gov == DESPOTISM) {
    if (dest->food > 3) {
      dest->food = 3 ;
    }
    if (dest->mat > 2) {
      dest->mat = 2 ;
    }
    if (dest->trade > 2) {
      dest->trade = 2 ;
    }
  }
}

/*-------------------->   central_tile_resources   <------------- 2010-May-02
This function returns the resources of the city tile.
City tiles are always irrigated, but produce no trade.
Irrigation is counted by "central_tile_resources", don't count it again
in the calling function!
Farmland is ignored here.
Limitations due to the government form have to be handled by the calling
function.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
RESOURCES* central_tile_resources( TILE tile, EPOCH epoch )
{
THIS_FUNC(central_tile_resources)
static RESOURCES ret_val ;

  if (epoch == EARLY_AGE) { /*no resource_2 in early age*/
    tile &= BONUS_RESOURCE_1_MASK | BASIC_TILE_TYPE_MASK ;
  }
  else {
    tile &= BONUS_RESOURCES_MASK | BASIC_TILE_TYPE_MASK ;
  }
  ret_val.food = 0 ; /*default*/
  ret_val.mat = 0 ;
  ret_val.trade = 0 ;

  switch (tile) {
  case GRASSLAND:
    ret_val.food = 4 ;
    break ;

  case (GRASSLAND | BONUS_RESOURCE_1_MASK): /* = plains*/
    ret_val.food = 3 ;
    ret_val.mat = 1 ;
    break ;
  
  case PRAIRIE:
    ret_val.food = 2 ;
    ret_val.mat = 1 ;
    break ;
  
  case (PRAIRIE | BONUS_RESOURCE_1_MASK): /*wheat, +2 food*/
    ret_val.food = 4 ;
    ret_val.mat = 1 ;
    break ;
  
  case (PRAIRIE | BONUS_RESOURCE_2_MASK): /*bauxite, +2 material*/
    ret_val.food = 2 ;
    ret_val.mat = 3 ;
    break ;
  
  case TUNDRA:
    ret_val.food = 2 ;
    break ;
  
  case (TUNDRA | BONUS_RESOURCE_2_MASK): /*natural gas, +4 material*/
    ret_val.food = 2 ;
    ret_val.mat = 4 ;
    break ;
  
  case HILLS:
    ret_val.food = 2 ;
    break ;
  
  case (HILLS | BONUS_RESOURCE_2_MASK): /*coal, +2 material*/
    ret_val.food = 2 ;
    ret_val.mat = 2 ;
    break ;

  default:
    break ;
  }
  return & ret_val ;
}

#ifdef NEVER
/*-------------------->   potential_tile_resources   <----------- 2008-Jul-13
This function returns the resources of a tile that is reasonably improved.
Irrigation, road, railroad, mine.  The Wheel.  Map making. HARBOR.
MONARCHY, MODERN_AGE, no SUPERMARKET, no SPACE_PORT, no OFFSHORE_PLATFORM,
no AUTOBAHN
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
RESOURCES* potential_tile_resources( TILE tile )
{
THIS_FUNC(potential_tile_resources)
static RESOURCES ret_val ;
  TILE til ;

  til = tile & (DEAD_LANDS | BONUS_RESOURCES_MASK | BASIC_TILE_TYPE_MASK) ;
  ret_val.food = 0 ; /*default*/
  ret_val.mat = 0 ;
  ret_val.trade = 0 ;

  /*PRT_VAR(til,08lx)*/
  if (til & DEAD_LANDS) { /*special or pole*/
    /*PRT_VAR(til & DEAD_LANDS,08lx)*/
    if ((til & ~(BASIC_TILE_TYPE_MASK | SPECIAL_MASK)) == 0) {
    /*PRT_VAR(til & ~(BASIC_TILE_TYPE_MASK | SPECIAL_MASK),08lx)*/
      til = DEAD_LANDS ; /*... or special resource*/
      /*PRT_VAR(til | 0x0,08lx)*/
    }
  }

  switch (til) {
  case (NORTH_POLE & BASIC_TILE_TYPE_MASK) | BONUS_RESOURCES_MASK:
  case (SOUTH_POLE & BASIC_TILE_TYPE_MASK) | BONUS_RESOURCES_MASK:
    break ;

  case COAST: /*with HARBOR, w/o OFFSHORE_PLATFORM*/
    ret_val.food = is_coast_city ? 2 : 1 ;
    ret_val.trade = 3 ;
    break ;

  case (COAST | BONUS_RESOURCE_1_MASK): /*fish, +4 food*/
    ret_val.food = is_coast_city ? 6 : 5 ;
    ret_val.trade = 3 ;
    break ;

  case (COAST | BONUS_RESOURCE_2_MASK): /*manganese, +5 material*/
    ret_val.food = is_coast_city ? 2 : 1 ;
    ret_val.mat = 5 ;
    ret_val.trade = 3 ;
    break ;

  case GRASSLAND: /*with irrigation and road*/
    ret_val.food = 4 ;
    ret_val.trade = 2 ;
    break ;

  case (GRASSLAND | BONUS_RESOURCE_1_MASK): /* = plains*/
    ret_val.food = 3 ;
    ret_val.mat = 1 ;
    ret_val.trade = 2 ;
    break ;

  case DESERT:
    ret_val.mat = 3 ; /*with mine & railroad*/
    ret_val.trade = 2 ;
    break ;
  
  case (DESERT | BONUS_RESOURCE_1_MASK): /*oasis, +3 food*/
    ret_val.food = 3 ;
    ret_val.mat = 3 ;
    ret_val.trade = 2 ;
    break ;
  
  case (DESERT | BONUS_RESOURCE_2_MASK): /*oil, +3 material*/
    ret_val.food = 3 ;
    ret_val.mat = 3 ;
    ret_val.trade = 2 ;
    break ;
  
  case PRAIRIE:
    ret_val.food = 2 ;
    ret_val.mat = 1 ;
    ret_val.trade = 2 ;
    break ;
  
  case (PRAIRIE | BONUS_RESOURCE_1_MASK): /*wheat, +2 food*/
    ret_val.food = 4 ;
    ret_val.mat = 1 ;
    ret_val.trade = 2 ;
    break ;
  
  case (PRAIRIE | BONUS_RESOURCE_2_MASK): /*bauxite, +2 material*/
    ret_val.food = 2 ;
    ret_val.mat = 4 ; /*Railroad*/
    ret_val.trade = 2 ;
    break ;
  
  case TUNDRA:
    ret_val.food = 2 ;
    ret_val.trade = 1 ;
    break ;
  
  case (TUNDRA | BONUS_RESOURCE_1_MASK): /*gold, +5 trade*/
    ret_val.food = 2 ; /*with irrigation*/
    ret_val.trade = 6 ;
    break ;
  
  case (TUNDRA | BONUS_RESOURCE_2_MASK): /*natural gas, +4 material*/
    ret_val.food = 2 ; /*with irrigation*/
    ret_val.mat = 6 ; /*with mine*/
    ret_val.trade = 1 ;
    break ;
  
  case ARCTIC:
    ret_val.mat = 6 ; /*with mine & railroad*/
    break ;
  
  case (ARCTIC | BONUS_RESOURCE_1_MASK): /*ivory, +3 food, +4 trade*/
  case (ARCTIC | BONUS_RESOURCE_2_MASK): /*ivory, +3 food, +4 trade*/
    ret_val.food = 3 ;
    ret_val.mat = 6 ; /*with mine & railroad*/
    ret_val.trade = 4 ;
    break ;
  
  case SWAMP: /*irrigation to GRASSLAND !!*/
    ret_val.food = 3 ; /*Grassland or plain???*/
    ret_val.trade = 2 ;
    break ;
  
  case (SWAMP | BONUS_RESOURCE_1_MASK): /*peat, +4 material*/
    ret_val.food = 1 ;
    ret_val.mat = 6 ; /*with railroad*/
    ret_val.trade = 1 ;
    break ;
  
  case (SWAMP | BONUS_RESOURCE_2_MASK): /*cotton, +1 material, +4 trade*/
    ret_val.food = 1 ;
    ret_val.mat = 1 ;
    ret_val.trade = 5 ;
    break ;
  
  case FOREST:
    ret_val.food = 1 ;
    ret_val.mat = 3 ; /*with railroad*/
    ret_val.trade = 1 ;
    break ;
  
  case (FOREST | BONUS_RESOURCE_1_MASK): /*game, +2 food*/
    ret_val.food = 3 ;
    ret_val.mat = 3 ; /*with railroad*/
    ret_val.trade = 1 ;
    break ;
  
  case (FOREST | BONUS_RESOURCE_2_MASK): /*silk, +3 trade*/
    ret_val.food = 1 ;
    ret_val.mat = 3 ; /*with railroad*/
    ret_val.trade = 4 ;
    break ;

  case HILLS:
    ret_val.food = 1 ;
    ret_val.mat = 4 ; /*with mine & railroad*/
    break ;
  
  case (HILLS | BONUS_RESOURCE_1_MASK): /*wine, +4 trade*/
    ret_val.food = 1 ;
    ret_val.mat = 4 ; /*with mine & railroad*/
    ret_val.trade = 4 ;
    break ;
  
  case (HILLS | BONUS_RESOURCE_2_MASK): /*coal, +2 material*/
    ret_val.food = 1 ;
    ret_val.mat = 7 ; /*with mine & railroad*/
    break ;

  case MOUNTAINS:
    ret_val.mat = 4 ; /*with mine & railroad*/
    break ;
  
  case (MOUNTAINS | BONUS_RESOURCE_1_MASK): /*iron, +3 material*/
    ret_val.mat = 9 ; /*with mine & railroad*/
    break ;
  
  case (MOUNTAINS | BONUS_RESOURCE_2_MASK): /*diamonds, +7 trade*/
    ret_val.mat = 4 ; /*with mine & railroad*/
    ret_val.trade = 7 ;
    break ;
  
  case DEAD_LANDS:
    ret_val.mat = 1 ;
    ret_val.trade = 1 ;
    break ;

  default:
    fprintf( log_fp,
      "potential_tile_resources: unexpected tile %08lx\n",
      (unsigned long)tile ) ;
    map_gen_exit() ;
  } /*end switch*/

  if (tile & RIVER) {
    (ret_val.trade)++ ;
  }
  return & ret_val ;
}

/*-------------------->   early_potential_tile_resources   <----- 2016-Aug-17
This function returns the resources of a tile that is reasonably improved.
Irrigation, road, mine.  The Wheel.  Map making. No HARBOR yet.
MONARCHY, EARLY_AGE (no bonus 2 resources).
The calling function has to limit food to 3 and material and trade to 2 if
government is DESPOTISM.
It has to limit trade to 3 if there is no COURTHOUSE.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- tile: TILE to evaluate
		- dest: points to RESOURCES
Return value:	pointer to RESOURCES
Exitcode:	x
---------------------------------------------------------------------------*/
RESOURCES* early_potential_tile_resources( TILE tile, RESOURCES* dest )
{
THIS_FUNC(early_potential_tile_resources)
static RESOURCES res ;
  RESOURCES* ret_val ;
  TILE til ;

  ret_val = dest ;
  if (dest == NULL) {
    ret_val = & res ;
  }

  ret_val->food = 0 ; /*default*/
  ret_val->mat = 0 ;
  ret_val->trade = 0 ;

  til = tile & (DEAD_LANDS | BONUS_RESOURCE_1_MASK | BASIC_TILE_TYPE_MASK ) ;
  if (til & DEAD_LANDS) { /*special or pole*/
    if ((til & ~(BASIC_TILE_TYPE_MASK | SPECIAL_MASK)) == 0) {
      til = DEAD_LANDS ; /*... or special resource*/
    }
    else {
      til = DUMMY_TILE ;
      return ret_val ; /*avoid +1 trade for river at end of function!*/
    }
  }

  switch (til) {

  case COAST: /*w/o HARBOR, w/o OFFSHORE_PLATFORM*/
    ret_val->food = 1 ;
    ret_val->trade = 3 ;
    break ;

  case (COAST | BONUS_RESOURCE_1_MASK): /*fish, +4 food*/
    ret_val->food = 5 ;
    ret_val->trade = 3 ;
    break ;

  case GRASSLAND: /*with irrigation and road*/
    ret_val->food = 4 ;
    ret_val->trade = 2 ;
    break ;

  case (GRASSLAND | BONUS_RESOURCE_1_MASK): /* = plains*/
    ret_val->food = 3 ;
    ret_val->mat = 1 ;
    ret_val->trade = 2 ;
    break ;

  case DESERT:
    ret_val->mat = 2 ; /*with mine*/
    ret_val->trade = 2 ; /*with road*/
    break ;
  
  case (DESERT | BONUS_RESOURCE_1_MASK): /*oasis, +3 food*/
    ret_val->food = 3 ;
    ret_val->mat = 2 ;
    ret_val->trade = 2 ;
    break ;
  
  case PRAIRIE:
    ret_val->food = 2 ;
    ret_val->mat = 1 ;
    ret_val->trade = 2 ;
    break ;
  
  case (PRAIRIE | BONUS_RESOURCE_1_MASK): /*wheat, +2 food*/
    ret_val->food = 4 ;
    ret_val->mat = 1 ;
    ret_val->trade = 2 ;
    break ;
  
  case TUNDRA:
    ret_val->food = 2 ; /*with irrigation*/
    ret_val->trade = 1 ;
    break ;
  
  case (TUNDRA | BONUS_RESOURCE_1_MASK): /*gold, +5 trade*/
    ret_val->food = 2 ; /*with irrigation*/
    ret_val->trade = 6 ;
    break ;
  
  case ARCTIC:
    ret_val->mat = 4 ; /*with mine*/
    break ;
  
  case (ARCTIC | BONUS_RESOURCE_1_MASK): /*ivory, +3 food, +4 trade*/
    ret_val->food = 3 ;
    ret_val->mat = 4 ; /*with mine*/
    ret_val->trade = 4 ;
    break ;
  
  case SWAMP: /*irrigation to GRASSLAND !!*/
    ret_val->food = 3 ; /*Grassland or plain???*/
    ret_val->trade = 2 ;
    break ;
  
  case (SWAMP | BONUS_RESOURCE_1_MASK): /*peat, +4 material*/
    ret_val->food = 1 ;
    ret_val->mat = 4 ;
    ret_val->trade = 1 ;
    break ;
  
  case FOREST:
    ret_val->food = 1 ;
    ret_val->mat = 2 ;
    ret_val->trade = 1 ;
    break ;
  
  case (FOREST | BONUS_RESOURCE_1_MASK): /*game, +2 food*/
    ret_val->food = 3 ;
    ret_val->mat = 2 ;
    ret_val->trade = 1 ;
    break ;

  case HILLS:
    ret_val->food = 1 ;
    ret_val->mat = 3 ; /*with mine*/
    break ;
  
  case (HILLS | BONUS_RESOURCE_1_MASK): /*wine, +4 trade*/
    ret_val->food = 1 ;
    ret_val->mat = 3 ; /*with mine*/
    ret_val->trade = 4 ;
    break ;

  case MOUNTAINS:
    ret_val->mat = 3 ; /*with mine*/
    break ;
  
  case (MOUNTAINS | BONUS_RESOURCE_1_MASK): /*iron, +3 material*/
    ret_val->mat = 6 ; /*with mine*/
    break ;
  
  case DEAD_LANDS:
    ret_val->mat = 1 ;
    ret_val->trade = 1 ;
    break ;

  default:
    fprintf( log_fp,
      "early_potential_tile_resources: unexpected tile %08lx\n",
      (unsigned long)tile ) ;
    map_gen_exit() ;
  } /*end switch*/

  if (tile & RIVER) { /*even HILLS produce one trade (with 0.14)*/
    (ret_val->trade)++ ;
  }
  return ret_val ;
}

/*-------------------->   initial_tile_resources   <------------- 2010-May-02
This function returns the resources of a tile at the start of the game.
DESPOTISM, no irrigation, no roads, no type 2 bonus resources etc.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
RESOURCES* initial_tile_resources( TILE tile )
{
THIS_FUNC(initial_tile_resources)
static RESOURCES ret_val ;

  /*PRT_VAR((unsigned long)tile,08lx)*/
  tile &= BONUS_RESOURCE_1_MASK | BASIC_TILE_TYPE_MASK ;
  ret_val.food = 0 ; /*default*/
  ret_val.mat = 0 ;
  ret_val.trade = 0 ;

  switch (tile) {
  case COAST:
    ret_val.food = 1 ;
    ret_val.trade = 2 ;
    break ;

  case (COAST | BONUS_RESOURCE_1_MASK): /*fish, +4 food*/
    ret_val.food = 3 ;
    ret_val.trade = 2 ;
    break ;

  case GRASSLAND:
    ret_val.food = 3 ;
    ret_val.trade = 1 ;
    break ;

  case (GRASSLAND | BONUS_RESOURCE_1_MASK): /* = plains*/
    ret_val.food = 2 ;
    ret_val.mat = 1 ;
    ret_val.trade = 1 ;
    break ;

  case DESERT:
    ret_val.mat = 1 ;
    ret_val.trade = 1 ;
    break ;
  
  case (DESERT | BONUS_RESOURCE_1_MASK): /*oasis, +3 food*/
    ret_val.food = 3 ;
    ret_val.mat = 1 ;
    ret_val.trade = 1 ;
    break ;
  
  case PRAIRIE:
    ret_val.food = 1 ;
    ret_val.mat = 1 ;
    ret_val.trade = 1 ;
    break ;
  
  case (PRAIRIE | BONUS_RESOURCE_1_MASK): /*wheat, +2 food*/
    ret_val.food = 3 ;
    ret_val.mat = 1 ;
    ret_val.trade = 1 ;
    break ;
  
  case TUNDRA:
    ret_val.food = 1 ;
    ret_val.trade = 1 ;
    break ;
  
  case (TUNDRA | BONUS_RESOURCE_1_MASK): /*gold, +5 trade*/
    ret_val.food = 1 ;
    ret_val.trade = 2 ;
    break ;
  
  case ARCTIC:
    ret_val.mat = 1 ;
    break ;
  
  case (ARCTIC | BONUS_RESOURCE_1_MASK): /*ivory, +3 food, +4 trade*/
    ret_val.food = 3 ;
    ret_val.mat = 1 ;
    ret_val.trade = 2 ;
    break ;
  
  case SWAMP:
    ret_val.food = 1 ;
    ret_val.trade = 1 ;
    break ;
  
  case (SWAMP | BONUS_RESOURCE_1_MASK): /*peat, +4 material*/
    ret_val.food = 1 ;
    ret_val.mat = 2 ;
    ret_val.trade = 1 ;
    break ;
  
  case FOREST:
    ret_val.food = 1 ;
    ret_val.mat = 2 ;
    ret_val.trade = 1 ;
    break ;
  
  case (FOREST | BONUS_RESOURCE_1_MASK): /*game, +2 food*/
    ret_val.food = 3 ;
    ret_val.mat = 2 ;
    ret_val.trade = 1 ;
    break ;

  case HILLS:
    ret_val.food = 1 ;
    break ;
  
  case (HILLS | BONUS_RESOURCE_1_MASK): /*wine, +4 trade*/
    ret_val.food = 1 ;
    ret_val.trade = 2 ;
    break ;

  case MOUNTAINS:
    ret_val.mat = 1 ;
    break ;
  
  case (MOUNTAINS | BONUS_RESOURCE_1_MASK): /*iron, +3 material*/
    ret_val.mat = 2 ;
    break ;

  default:
    fprintf( log_fp, "initial_tile_resources: unexp. tile type %08lx\n",
             tile ) ;
    map_gen_exit() ;
  }
  return & ret_val ;
}
#endif /*NEVER*/

/*-------------------->   fill_idx_table   <--------------------- 2010-Jul-27
This function fills the internal array "idx_table" with the resources of
20 city tiles.
;Only Bonus 1 resources are counted right now.
;Maybe extend this function with an EPOCH parameter later.
-----------------------------------------------------------------------------
Used functions: initial_tile_resources, early_potential_tile_resources
Internals/Globals: idx_table
Parameters:	- p: pointer to CITY_TILES
		- government: DESPOTISM or MONARCHY
		- dont_use_mask: If one of these flags is set, the tile
		                 evaluates to a DUMMY tile (zero resources)
		- improve_mask: If the tile can be improved (road/irrigation/
		                mine), set one of these bits.  Can your 1st
		                settler reach this tile before MAP_MAKING?
Return value:	--
Exitcode:	--
---------------------------------------------------------------------------*/
static void fill_idx_table( CITY_TILES* p, MODIFIERS* mod,
                            U8 dont_use_mask, U8 improve_mask )
{
THIS_FUNC(fill_idx_table)
  MODIFIERS mod_for_unreachable_tiles ;
  TIDX idx2 ;
  TILE tile ;
  RESOURCES* res_p ;
  RESOURCES* Rp ;
  U8 i ; /*loop control*/


  memcpy( & mod_for_unreachable_tiles, mod, sizeof( MODIFIERS ) ) ;
  mod_for_unreachable_tiles.road = FALSE ;
  mod_for_unreachable_tiles.railroad = FALSE ;
  mod_for_unreachable_tiles.irrigation = FALSE ;
  mod_for_unreachable_tiles.farmland = FALSE ;
  mod_for_unreachable_tiles.mining = FALSE ;

  switch (mod->gov) {
  case DESPOTISM:
  case MONARCHY:
    break ; /*ok*/
  default:
    printf( "fill_idx_table: government %u not yet implemented\n",
            (unsigned)(mod->gov) ) ;
    map_gen_exit() ;
  }

  for ( i = 0 ; i < 20 ; i++ ) {
    idx2 = ( & (p->i1)) [i] ;
    (idx_table [i]).idx = idx2 ;
    res_p = & ((idx_table [i]).res) ;
    res_p->food  = 0 ; /*defaults*/
    res_p->mat   = 0 ;
    res_p->trade = 0 ;
    if (idx2 < total_no_of_tiles) { /*inside the map*/
      if (((world_flags [idx2]) & dont_use_mask) == 0) {
                                       /*we may use this tile*/
        tile = world [idx2] & (DEAD_LANDS | BONUS_RESOURCE_1_MASK
                             | BASIC_TILE_TYPE_MASK | RIVER) ;
        if ((world_flags [idx2] & improve_mask) != 0) {
          /*early_potential_tile_resources( tile, res_p ) ;*/
          tile_resources( tile, mod, res_p ) ;
        }
        else {
          /*Rp = initial_tile_resources( tile ) ;*/
          /*res_p->food  = Rp->food ;*/
          /*res_p->mat   = Rp->mat ;*/
          /*res_p->trade = Rp->trade ;*/
          tile_resources( tile, & mod_for_unreachable_tiles, res_p ) ;
        }
      }
    }

    if (mod->gov == DESPOTISM) {
      if (res_p->food > 3) {
        res_p->food = 3 ;
      }
      if (res_p->mat > 2) {
        res_p->mat = 2 ;
      }
      if (res_p->trade > 2) {
        res_p->trade = 2 ;
      }
    }
  } /*end city radius for loop*/
}

/*-------------------->   idx_sort_max_sum   <------------------- 2010-Apr-14
This function is called by qsort.
It sorts idx_table for max sum first, and if equal, for max food.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
static int idx_sort_max_sum( const void* elem1, const void* elem2 )
{
THIS_FUNC(idx_sort_max_sum)
  S8 temp ;

  temp = (int)(  ((IDX_TABLE_ELEM*)elem2)->res.food
               + ((IDX_TABLE_ELEM*)elem2)->res.mat
               + ((IDX_TABLE_ELEM*)elem2)->res.trade
               - ((IDX_TABLE_ELEM*)elem1)->res.food
               - ((IDX_TABLE_ELEM*)elem1)->res.mat
               - ((IDX_TABLE_ELEM*)elem1)->res.trade
              ) ;
  if (temp == 0) {
    return (int)(  ((IDX_TABLE_ELEM*)elem2)->res.food
                 - ((IDX_TABLE_ELEM*)elem1)->res.food
                ) ;
  }
  else {
    return (int)temp ;
  }
}

/*-------------------->   idx_sort_max_food   <------------------ 2010-Apr-14
This function is called by qsort.
It sorts idx_table for max food first, and if equal, for max resources.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
static int idx_sort_max_food( const void* elem1, const void* elem2 )
{
THIS_FUNC(idx_sort_max_food)
  S8 temp ;

  temp =   ((IDX_TABLE_ELEM*)elem2)->res.food
         - ((IDX_TABLE_ELEM*)elem1)->res.food ;

  if (temp == 0) {
    return (int)(  ((IDX_TABLE_ELEM*)elem2)->res.mat
                 + ((IDX_TABLE_ELEM*)elem2)->res.trade
                 - ((IDX_TABLE_ELEM*)elem1)->res.mat
                 - ((IDX_TABLE_ELEM*)elem1)->res.trade
                ) ;
  }
  else {
    return (int)temp ;
  }
}

/*-------------------->   idx_sort_max_mat   <------------------- 2010-May-14
This function is called by qsort.
It sorts idx_table for max mat first, and if equal, for max resources.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
static int idx_sort_max_mat( const void* elem1, const void* elem2 )
{
THIS_FUNC(idx_sort_max_mat)
  S8 temp ;

  temp =   ((IDX_TABLE_ELEM*)elem2)->res.mat
         - ((IDX_TABLE_ELEM*)elem1)->res.mat ;

  if (temp == 0) {
    return (int)(  ((IDX_TABLE_ELEM*)elem2)->res.food
                 + ((IDX_TABLE_ELEM*)elem2)->res.trade
                 - ((IDX_TABLE_ELEM*)elem1)->res.food
                 - ((IDX_TABLE_ELEM*)elem1)->res.trade
                ) ;
  }
  else {
    return (int)temp ;
  }
}

/*-------------------->   x   <---------------------------------- 2010-Jul-26
This function x
-----------------------------------------------------------------------------
Used functions:
Globals/Internals:
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
