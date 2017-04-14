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
cevo_map.h

Include file for C-Evo-Map-Tools.
*****************************************************************************
History: (latest change first)
2016-Aug-08: clean log file implementation
2016-Jul-16: adoptions for gcc
2015-Jun-06: added "tag_adjacent_tiles"
2013-Apr-12: added "tag_whole_area" and "tag_on_flag_pattern_match"
2012-Sep-25: new function "untag_inside_radius"
2012-Sep-21: dist_tile_to_group with world flag matching
2012-Aug-26: nested include "misc.h" for U8
2012-Jun-06: moved func prototypes for "map_gen" to "map_gen.h"
2012-Feb-14: - changed return type of "draw_on_flag_pattern_match"
               and "add_lake" to TIDX
             - added define for "INVALID_TIDX"
             - added func "best_starting_pos_simple"
2010-Jul-27: masked out some resource-Funktions (should be internal)
2010-Jun-22: added "flags_in_use", "tag_whole_TTL"
2009-Jan-16: tag_whole_ocean(), tag_inside_radius()
2008-Jul-12: TIDX (tile index) typedef'ed, added function "best_starting_pos"
2008-Apr-25: Lakes with RAGGED or SMOOTH coastlines
2008-Apr-06: added function "tag_city_tiles", "tag_neighborhood_tiles"
2008-Mar-19..24: macros WR_WORLD, WR_WORLD_FLAGS
2008-Mar-17: TTLs (Tile Type Lists)
2008-Mar-16: more DEFINEs
2007-Mar-16..20: added more functions
2007-Mar-15: world_flags
2006-Mar-29: changed a lot of U32 to TILE
2006-Mar-27: added typedefs
2006-Mar-26: added function "cevo_lib_init"
2006-Mar-25: added function "is_land_tile"
2006-Feb-24: added struct CITY_TILES
2006-Feb-22: initial version (created from map_dump.c)
****************************************************************************/
#ifndef CEVO_MAP_H
#define CEVO_MAP_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

#include <misc.h>


/*--  constants  ----------------------------------------------------------*/

#define BASIC_TILE_TYPE_MASK  0x0000000f
#define BONUS_RESOURCE_1_MASK  0x20
#define BONUS_RESOURCE_2_MASK  0x40
#define BONUS_RESOURCES_MASK  (BONUS_RESOURCE_1_MASK | BONUS_RESOURCE_2_MASK)

   /*basic tile types*/
#define OCEAN     0x00000000
#define COAST     0x00000001
#define GRASSLAND 0x00000002
#define DESERT    0x00000003
#define PRAIRIE   0x00000004
#define TUNDRA    0x00000005
#define ARCTIC    0x00000006
#define SWAMP     0x00000007

#define FOREST    0x00000009
#define HILLS     0x0000000a
#define MOUNTAINS 0x0000000b

#define RIVER     0x00000080

   /*Tile Type List (TTL) definitions*/
   /*parameters for func "count_xxx_tiles"*/
#define TTL_OCEAN     0x00000001
#define TTL_COAST     0x00000002
#define TTL_WATER     0x00000003

#define TTL_GRASSLAND 0x00000004
#define TTL_DESERT    0x00000008
#define TTL_PRAIRIE   0x00000010
#define TTL_TUNDRA    0x00000020
#define TTL_ARCTIC    0x00000040
#define TTL_SWAMP     0x00000080
#define TTL_FOREST    0x00000200
#define TTL_HILLS     0x00000400
#define TTL_MOUNTAINS 0x00000800
#define TTL_LAND      0x00000efc

#define TTL_ANY       0x00000eff

#define TTL_POLE      0x00001000
#define TTL_RIVER     0x00002000


   /*tile improvements*/
#define IMPROVEMENT_MASK  0x00007700
#define ROAD              0x00000100
#define RAILROAD          0x00000200
#define CANAL             0x00000400
#define IRRIGATION        0x00001000
#define FARMLAND          0x00002000
#define MINE              0x00003000
#define FORT              0x00004000
#define MILITARY_BASE     0x00005000

#define POLLUTION         0x00010000


#define STARTPOS_MASK   0x00600000
#define HUMAN_STARTPOS  0x00200000
#define NORMAL_STARTPOS 0x00400000


#define SPECIAL_MASK  0x07000000
#define DEAD_LANDS    0x01000000
#define COBALT        0x03000000
#define URANIUM       0x05000000
#define MERCURY       0x07000000

   /*special return values for indices and tile types*/
#define INVALID_TIDX 0xffffffff
#define NORTH_POLE   0xffffffff
#define SOUTH_POLE   0xfffffffe
                 /*a dummy tile produces nothing*/
                 /*it is e. g. outside the map*/
#define DUMMY_TILE   0xfffffffd

   /*world_flags*/
#define ADD_LAND_HERE0        0x01 /*must be LSB coz it is a 3 bit counter*/
#define ADD_HERE              0x01
#define ADD_HERE0             0x01
#define ADD_LAND_HERE1        0x02
#define ADD_HERE1             0x02
#define ADD_LAND_HERE2        0x04
#define ADD_HERE2             0x04
#define ADD_HERE_ALL          0x07
#define ADD_LAND_HERE_ALL     0x07

#define TEMP_TAG              0x08

#define      COUNTED          0x10
#define      TAGGED           0x10

#define PREV_COUNTED          0x20
#define PREV_TAGGED           0x20
#define IS_SHORE              0x20

#define NO_BIG_ISLAND_HERE    0x40
#define NO_LAKE_HERE          0x40
#define FISH_HERE             0x40
#define DONT_USE              0x40

#define NO_SMALL_ISLAND_HERE  0x80
#define NO_RIVER_HERE         0x80
#define MANGANESE_HERE        0x80
#define DONT_SETTLE_DOWN      0x80



/*--  typedefs & enums  ---------------------------------------------------*/

typedef U8 EPOCH ;
#define EARLY_AGE       0x01
#define INDUSTRIAL_AGE  0x02
#define MODERN_AGE      0x04
#define FUTURE_AGE      0x08

typedef U8 GOV ;
#define ANARCHY         0x01
#define DESPOTISM       0x02
#define MONARCHY        0x04
#define REPUBLIC        0x08
#define DEMOCRACY       0x10
#define COMMUNISM       0x20
#define FUNDAMENTALISM  0x40
#define FUTURE_SOCIETY  0x80

typedef U32 CITY_IMP ;
#define HARBOR             0x00000004
#define TOWN_HALL          0x00000008
#define COURTHOUSE         0x00000010

typedef U16 STATE_IMP ;
#define PALACE             0x0001
#define SPACE_PORT         0x0100



typedef U32 TILE ;
typedef U32 TIDX ; /*tile index*/
typedef U32 TTL ; /*Tile Type List*/

typedef struct {
   /*relative tile positions in linear tile array*/
   /*n = north tile, sw = south west tile and so on*/
  TILE n_tile ;
  TILE ne_tile ;
  TILE e_tile ;
  TILE se_tile ;
  TILE s_tile ;
  TILE sw_tile ;
  TILE w_tile ;
  TILE nw_tile ;

     /*indices of the above tiles*/
  U32 n_idx ;
  U32 ne_idx ;
  U32 e_idx ;
  U32 se_idx ;
  U32 s_idx ;
  U32 sw_idx ;
  U32 w_idx ;
  U32 nw_idx ;
} NEIGHBORHOOD ;

typedef struct {
  TILE t1 ; /*         t1    t2        */
  TILE t2 ; /*      t3    t4    t5     */
  TILE t3 ; /*   t6    t7    t8    t9  */
  TILE t4 ; /*      t10   X     t11    */
  TILE t5 ; /*   t12   t13   t14   t15 */
  TILE t6 ; /*      t16   t17   t18    */
  TILE t7 ; /*         t19   t20       */
  TILE t8 ;
  TILE t9 ;
  TILE t10 ;
  TILE t11 ;
  TILE t12 ;
  TILE t13 ;
  TILE t14 ;
  TILE t15 ;
  TILE t16 ;
  TILE t17 ;
  TILE t18 ;
  TILE t19 ;
  TILE t20 ;

     /*indices of the above tiles*/
  U32 i1 ;
  U32 i2 ;
  U32 i3 ;
  U32 i4 ;
  U32 i5 ;
  U32 i6 ;
  U32 i7 ;
  U32 i8 ;
  U32 i9 ;
  U32 i10 ;
  U32 i11 ;
  U32 i12 ;
  U32 i13 ;
  U32 i14 ;
  U32 i15 ;
  U32 i16 ;
  U32 i17 ;
  U32 i18 ;
  U32 i19 ;
  U32 i20 ;
} CITY_TILES ;

typedef struct {
  U8 food ;
  U8 mat ;
  U8 trade ;
} RESOURCES ;


/*--  function prototypes  ------------------------------------------------*/

void cevo_lib_init( void ) ;

NEIGHBORHOOD* set_neighborhood_tiles( U32 index, NEIGHBORHOOD* p ) ;
CITY_TILES* set_city_tiles( U32 index, CITY_TILES* p ) ;
U8 count_adjacent_tiles( U32 index, TTL list ) ;
U8 count_neighborhood_tiles( U32 index, TTL list ) ;
U8 count_city_tiles( U32 index, TTL list ) ;
/*U8 mark_neighborhood_tiles( U32 index, TTL list, U8 mask ) ;*/
/*U8 mark_city_tiles( U32 index, TTL list, U8 mask ) ;*/

BIT is_land_tile( U32 index ) ;
BIT is_in_TTL( TIDX index, TTL list) ;
void clear_flags( U8 mask ) ;
void tag_city_tiles( TIDX idx, U8 mask ) ;
void tag_neighborhood_tiles( TIDX idx, U8 mask ) ;
void tag_adjacent_tiles( TIDX idx, U8 mask ) ;
U32  tag_whole_area( U8 area_mask, U8 area_flags, TTL list,
                     /*CLUSTER_METHOD method,*/ U8 tag_mask, U8 untag_mask ) ;
                     /*CLUSTERs tbd in cevo_map.h*/
U32  tag_on_flag_pattern_match( U8 mask, U8 match, U8 set, U8 clr ) ;
void tag_whole_TTL( TTL list, U8 mask ) ;
void tag_whole_land( void ) ;
void tag_whole_ocean( U8 mask ) ;
void tag_whole_coast_water( U8 island_mask, U8 coast_mask ) ;
void   tag_inside_radius( TIDX index, U16 radius, U8 mask ) ;
void untag_inside_radius( TIDX index, U16 radius, U8 mask ) ;

U16 dist_tile_to_tile( U32 index1, U32 index2 ) ;
U16 dist_tile_to_group( TIDX index, U8 mask, U8 flags  ) ;
U16 direction_tile_to_tile( U32 from_index, U32 to_index ) ;

U16 rate_1st_pos( TIDX tile_index ) ;
/*U16 rateXXX_start_pos( U32 tile_index ) ;*/
TIDX best_starting_pos( void ) ;
TIDX best_starting_pos_simple( U8 add_here_mask, U8 add_here,
                               U8 mark_city, U8 mark_radius, U16 radius ) ;
void set_special_resource( U32 idx, U32 spec ) ;
/*RESOURCES* tile_resources( TILE tile, EPOCH epoch, CITY_IMP city_imp,*/
                        /*GOV gov, STATE_IMP state_imp ) ;*/
RESOURCES* central_tile_resources( TILE tile, EPOCH epoch ) ;
/*RESOURCES* potential_tile_resources( TILE tile ) ;*/
/*RESOURCES* early_potential_tile_resources( TILE tile, RESOURCES* dest ) ;*/
/*RESOURCES* initial_tile_resources( TILE tile ) ;*/


/*--  macros  -------------------------------------------------------------*/

#define WR_WORLD_FLAGS(idx,val)  \
  if ((idx) < total_no_of_tiles) {  \
    world_flags [idx] = val ;  \
  }

#define SET_WORLD_FLAGS(idx,mask)  \
  if ((idx) < total_no_of_tiles) {  \
    world_flags [idx] |= mask ;  \
  }

#define CLR_WORLD_FLAGS(idx,mask)  \
  if ((idx) < total_no_of_tiles) {  \
    world_flags [idx] &= ~(mask) ;  \
  }

#define WR_WORLD(idx,val)  \
  if ((idx) < total_no_of_tiles) {  \
    world [idx] = val ;  \
  }

#define WORLD_FLAGS_ALLOC(mask) \
  if ((flags_in_use & (mask)) != 0) { \
    fprintf( log_fp, "%s: flags already in use: 0x%02x\n", \
                     _this_func, \
                     (unsigned)flags_in_use ) ; \
    map_gen_exit() ; \
  } \
  flags_in_use |= (mask) ;

#define WORLD_FLAGS_FREE(mask) \
  if ((flags_in_use & (mask)) != (mask)) { \
    fprintf( log_fp, "%s: flags already free??: 0x%02x\n", \
                     _this_func, \
                     (unsigned)flags_in_use ) ; \
    map_gen_exit() ; \
  } \
  flags_in_use &= (U8)~(mask) ;



/*--  global variables  ---------------------------------------------------*/

extern U32 LX ;
extern U32 LY ;
extern U32 total_no_of_tiles ; /*tile indices run from zero to
                                 total_no_of_tiles -1 */
extern U32 LX2 ;
extern U32 LX3 ;
extern U32 LX4 ;
extern U32 LY2 ;
extern U32 LY3 ;
extern U32 LY4 ;

extern TILE* world ;

extern U8* world_flags ;
extern U8 flags_in_use ; /*1-bit means "in use"*/


/***************************************************************************/
#endif	/* CEVO_MAP_H */
/***************************************************************************/
