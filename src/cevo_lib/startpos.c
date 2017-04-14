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
startpos.c

This file contains functions which find suitable city starting positions,
both at the start of the game and after discovering new territory.
*****************************************************************************
History: (latest change first)
2017-Feb-27: changed include pathes for source code release
2016-Aug-09..2017-Feb-18: clean log file implementation
2016-Jul-15: adoptions for gcc
2013-Apr-13: additional error message with map_gen_exit() instead of ASSERT
2012-Aug-17: allocated memory ffree'd
2012-Aug-16: changed order of include files (falloc.h)
2012-Feb-14: debugging (ASSERT failed) -- bug not found, algorithm unstable?
             - added new function "best_starting_pos_simple"
2012-Feb-10: temporary fix (map_gen_exit)
2010-May-14: less NULL pointers -- more object oriented
2010-May-02: Debugging
2009-Jan-12: Documentation added
2008-Jul-12: derived from "scn_nav.c"
*****************************************************************************
Global objects:
- TIDX best_starting_pos()
- TIDX best_starting_pos_simple()
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*only for testing purposes*/



/*--  include files  ------------------------------------------------------*/

/*#define DEBUG*/
#include <debug.h>

#include "cevo_map.h"
#ifdef unix
# include "../map_gen/read_ini.h"
#else
# include "..\map_gen\read_ini.h"
#endif
#include <falloc.h>

    /*temporary fix*/
    /*proper solution: WORLD_FLAG_ALLOC shouldn't use "map_gen_exit"*/
#ifdef unix
# include "../map_gen/map_gen.h" /*for map_gen_exit()*/
#else
# include "..\map_gen\map_gen.h" /*for map_gen_exit()*/
#endif


/*--  constants  ----------------------------------------------------------*/



/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/



/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/



/*--  internal variables  -------------------------------------------------*/



/*-------------------->   best_starting_pos   <------------------ 2017-Feb-18
This function returns the best starting position of all TAGGED tiles.
Special care is taken for compliance with future foundations in the TAGGED
area (optimized area exploitation).
The function was designed for Navigation_required and is currently not used
anywhere else.  Moreover, it has not been tested for anything else as rather
small islands.
-----------------------------------------------------------------------------
Used functions:
Parameters:	--
Globals/Internals: world_flags
  Input: TAGGED
  Output/Modified: ADD_HERE_ALL, PREV_TAGGED, FISH_HERE, MANGANESE_HERE -> all
Return value:	tile_index of best starting position
Exitcode:	x
---------------------------------------------------------------------------*/
TIDX best_starting_pos( void )
{
/*THIS_FUNC(best_starting_pos)*/
char* _this_func = "best_starting_pos" ; /*for WORLD_FLAGS*/
  CITY_TILES CT ;
  NEIGHBORHOOD NT ;
  TIDX tile_index, best_index ;
  TILE tile ;
  U32 temp ;
  CITY_TILES* CTp ;
  NEIGHBORHOOD* NTp ;
  TILE* Tp ;
  TIDX* idxp ;
  TIDX* remember_idxp ;
  int i, j ; /*loop control*/
  U16 rating, best_rating ;
  U8 land_tiles ;
  U8 coast_count ;
  U8 rating1, rating2 ;
  U8 flags ; /*copy of world_flags*/


  WORLD_FLAGS_ALLOC( ADD_HERE_ALL | PREV_TAGGED | FISH_HERE | MANGANESE_HERE )
  clear_flags( ADD_HERE_ALL | PREV_TAGGED | FISH_HERE | MANGANESE_HERE ) ;
     /*tag all nearby water with PREV_TAGGED*/
  tag_whole_coast_water( TAGGED, PREV_TAGGED ) ;

     /*go thru all COAST tiles to identify those which can be exploited
       by only one single land tile*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if ((world_flags [ tile_index ]) & PREV_TAGGED) { /*COAST water*/
      CTp = set_city_tiles( tile_index, & CT ) ;
      land_tiles = 0 ;
         /*count all land tiles that can reach this COAST tile*/
      for ( j = 0, Tp = & (CTp->t1), idxp = & (CTp->i1) ;
            j < 20 ;
            j++, Tp++, idxp++ ) {
        switch ((*Tp) & BASIC_TILE_TYPE_MASK) {
        case GRASSLAND:
        case DESERT: /*can be transformed to PRAIRIE*/
        case PRAIRIE:
        case TUNDRA: /*can be transformed to GRASSLAND*/
        case SWAMP: /*can be irrigated to GRASSLAND*/
        case FOREST: /*can be irrigated to PRAIRIE*/
        case HILLS: /*can be transformed to GRASSLAND*/
          land_tiles++ ;
          remember_idxp = idxp ;
             /*if there is only one land tile, it is this one*/
          break ;

        case COAST:
        case OCEAN:
        case ARCTIC:
        case MOUNTAINS:
        case (NORTH_POLE & BASIC_TILE_TYPE_MASK):
        case (SOUTH_POLE & BASIC_TILE_TYPE_MASK):
          break ;

        default:
          fprintf( stderr,
              "best_starting_pos: unexp. tile type %08lx\n", (long)(*Tp) ) ;
          fprintf( log_fp,
              "best_starting_pos: unexp. tile type %08lx\n", (long)(*Tp) ) ;
          map_gen_exit() ;
        }
      }
      /*PRT_VAR((int)land_tiles,d)*/
         /*special treatment for those COAST tiles which can be reached*/
         /*only from one land tile*/
      if (land_tiles == 1) { /*no special treatment if landtiles > 1*/
        tile = world [ tile_index ] ; /*should always be COAST here*/
        ASSERT((tile & BASIC_TILE_TYPE_MASK) == COAST)

        temp = *remember_idxp ; /*temp is index of the only land tile ...*/
        ASSERT(temp < total_no_of_tiles)

        flags = world_flags [ temp ] ; /*flags of the only land tile ...*/
           /*count the COAST tiles which can be exploited only by this land*/
           /*Counting is done in the ADD_HERE_ALL bits.*/
        coast_count = flags & ADD_HERE_ALL ; /*get prev counter value*/
        coast_count++ ; /*another COAST which can be exploited only
                          by this land tile*/
        ASSERT(coast_count < 8) /*is true for non-1tile-islands*/
        flags = (flags & ~ADD_HERE_ALL) | coast_count ; /*mix counter value
                                                          with flags*/
        if (tile & BONUS_RESOURCE_1_MASK) { /*COAST tile*/
          flags |= FISH_HERE ;
        }
        if (tile & BONUS_RESOURCE_2_MASK) { /*COAST tile*/
          flags |= MANGANESE_HERE ;
        }
        world_flags [ temp ] = flags ; /*store flags for land tile*/
      }
    } /*end if  coast water*/
  } /*end coast tile loop*/
  /*DEB((stderr,"coast tile loop finished\n")) ;*/
  /*Now those land tiles which can exclusively exploit one or more COAST*/
  /*tiles are marked by non-zero ADD_HERE_ALL flags*/
  /*The ADD_HERE_ALL counter tells how many COAST tiles this land tile*/
  /*can exploit exclusively*/


     /*now check if all marked coast city places (ADD_HERE_ALL)*/
     /*could survive (food >= 4)*/
  /*for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {*/
    /*if ((world_flags [ tile_index ]) & ADD_LAND_HERE_ALL) {*/
      /*to be implemented*/
    /*}*/
  /*}*/
     /*Now avoid cities in close (direct) neighborhood*/
     /*Look for city clusters and delete the 'looser' places*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    flags = world_flags [ tile_index ] ;
    coast_count = flags & ADD_LAND_HERE_ALL ;
    if (coast_count > 0) {
      rating1 = 5 * coast_count ;
      if (flags & FISH_HERE) {
        rating1 += 8 ;
      }
      if (flags & MANGANESE_HERE) {
        rating1 += 10 ;
      }
      NTp = set_neighborhood_tiles( tile_index, & NT ) ;
      for ( j = 0, idxp = & (NTp->n_idx) ; j < 8 ; j++, idxp++ ) {
        flags = world_flags [ *idxp ] ;
        coast_count = flags & ADD_HERE_ALL ;
        if (coast_count) { /*found city in neighborhood*/
          rating2 = 5 * coast_count ;
          if (flags & FISH_HERE) {
            rating2 += 8 ;
          }
          if (flags & MANGANESE_HERE) {
            rating2 += 10 ;
          }
          temp = (rating2 > rating1) ? tile_index : (*idxp) ;
          world_flags [ temp ] &= /*delete looser place*/
              ~(ADD_LAND_HERE_ALL | FISH_HERE | MANGANESE_HERE) ;
        }
      }
    }
  }
  WORLD_FLAGS_FREE( FISH_HERE | MANGANESE_HERE )
  fprintf( log_fp, "city clusters removed\n" ) ;

     /*Tag city radius of remaining coast places as 'don't use'*/
     /*Tag all tiles, including coast*/
     /*Tag neighboorhood tiles as 'don't settle down'*/
  WORLD_FLAGS_ALLOC( DONT_USE | DONT_SETTLE_DOWN )
  clear_flags( DONT_USE | DONT_SETTLE_DOWN ) ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    flags = world_flags [ tile_index ] ;
    if ((flags & ADD_HERE_ALL) > 0) { /*coast place*/
      tag_city_tiles( tile_index, DONT_USE ) ;
      tag_neighborhood_tiles( tile_index, DONT_SETTLE_DOWN ) ;
         /*but do settle down in center tile*/
      world_flags [ tile_index ] &= ~DONT_SETTLE_DOWN ;
    }
  }
  WORLD_FLAGS_FREE( ADD_HERE_ALL | PREV_TAGGED )
  fprintf( log_fp, "city vicinity tagged\n" ) ;


  best_rating = 0 ;
  best_index = total_no_of_tiles ; /*is invalid value*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    flags = world_flags [ tile_index ] ;
    if ((flags & TAGGED) && ((flags & DONT_SETTLE_DOWN) == 0)) {

      rating = rate_1st_pos( tile_index ) ; /*uses TEMP_TAG*/

      if (rating > best_rating) {
        best_rating = rating ;
        best_index = tile_index ;
      }
    }
  }
  /*ASSERT(best_rating > 0)*/
  if (best_rating == 0) {
    fprintf( stderr, "best_starting_pos: best_rating = 0\n" ) ;
    fprintf( log_fp, "best_starting_pos: best_rating = 0\n" ) ;
    map_gen_exit() ;
  }
  ASSERT(best_index != total_no_of_tiles) /*happened 2012-Feb-14*/
  /*PRT_VAR(best_rating,u)*/
  fprintf( log_fp, "best_starting_pos: best_rating = %u\n", best_rating ) ;
  WORLD_FLAGS_FREE( DONT_USE | DONT_SETTLE_DOWN )
  return best_index ;
}

/*-------------------->   best_starting_pos_simple   <----------- 2016-Jul-15
This function returns (simply) the best starting position of all tagged tiles.
NO care is taken for compliance with future foundations in the tagged
area (NO optimized area exploitation).
The start position is NOT written into the world map, so this function can
support the placing of special resources.
-----------------------------------------------------------------------------
Used functions:
Globals:        startpos_min_rating
                world_flags (determined by parameters)
Internals:
Parameters:	- add_here_mask     mask for the following parameter
		- add_here          flag combination: where to search
		                    start positions
		- mark_city         tag city radius of returned index with
		                    this mask (make it 0x00 for no effect)
		- mark_radius       tag radius around returned index with
		                    this mask (make it 0x00 for no effect)
		- radius            radius for above parameter (make it
		                    100 if not used to keep runtime short)
Return value:	best index in area which is determined by the first two
                parameters or
                INVALID_TIDX if no suitable startpos is found
Exitcode:	--
---------------------------------------------------------------------------*/
TIDX best_starting_pos_simple( U8 add_here_mask, U8 add_here,
          U8 mark_city, U8 mark_radius, U16 radius )
{
THIS_FUNC(best_starting_pos_simple)
  U32 tile_index, best_index ;
  U8* ratings ; /*size: total_no_of_tiles*/
  U16 best_rating ;


  /*TIME_STAMP("Entry best_starting_pos_simple")*/
  /*PRT_VAR((unsigned)startpos_min_rating,u)*/
  /*PRT_VAR(add_here_mask,02x)*/
  /*PRT_VAR(add_here,02x)*/
  /*PRT_VAR(mark_radius,02x)*/

     /*step 1: rate all tiles on map which match add_here*/
  ratings = (U8*)falloc( (unsigned)total_no_of_tiles ) ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (((world_flags [ tile_index ]) & add_here_mask) == add_here) {
      ratings [ tile_index ] = rate_1st_pos( tile_index ) ; /*uses TEMP_TAG*/
    }
    else {
      ratings [ tile_index ] = 0 ;
    }
  }
  /*TIME_STAMP("all ratings done")*/

     /*step 2: select from previously rated tiles*/
  best_rating = 0 ;
  best_index = 0 ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (ratings [ tile_index ] > best_rating) {
      best_rating = ratings [ tile_index ] ;
      best_index = tile_index ;
    }
  }
  /*PRT_VAR(best_rating,u)*/
  fprintf( log_fp, "best_starting_pos_simple: best_rating = %u\n",
                                              best_rating ) ;
  /*PRT_VAR((unsigned long)best_index,lu)*/
  fprintf( log_fp, "best_starting_pos_simple: best_index = %lu\n",
                               (unsigned long)best_index ) ;

  ffree( ratings ) ;
  if (best_rating < startpos_min_rating) {
    return INVALID_TIDX ;
  }

     /*tag city tiles for some reasons (service for calling function)*/
  tag_city_tiles( best_index, mark_city ) ;

     /*tag radius around city for some reasons (service for calling function)*/
  tag_inside_radius( best_index, radius, mark_radius ) ;

  return best_index ;
}

/*-------------------->   x   <---------------------------------- 2013-Apr-13
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
