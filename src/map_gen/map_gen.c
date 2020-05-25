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
map_gen.c

This program generates a C-Evo-map file.
*****************************************************************************
History: (latest change first)
2017-Mar-24: GNU copyright message and option -g
2017-Feb-27: don't overwrite log file by default
2017-Feb-18: debugging (segmentation fault)
2016-Oct-21: close/reopen log file to save seed
2016-Aug-08: clean log file implementation
2016-Jul-15: adoptions for gcc
2016-Jul-11: Version # in line 1 of log file
2015-Nov-14: modified distance in "add_starting_positions"
2015-Aug-01: added scenario "Fjords"
2015-Jun-03: added scenario "Desert"
2015-Feb-14: add_starting_positions() now writes error msg to stdout, too
2015-Feb-13: added scenario "Volcano Islands"
2014-Oct-08: Debugging (probability vector)
2013-Apr-12: VERSION_STR "1.0"
2013-Apr-07: clarified usage text
2013-Apr-06: write message to stderr when missing "map_gen.ini"
2013-Feb-28: option -i for alternate .ini file
2013-Feb-14: added '--version' and '--help' options
2012-Sep-25: improved "add_starting_positions"
2012-Sep-24: introduction of "map_size_idx"
2012-Jun-06: added scenario "Great Plains"
2012-Mar-14: Debugging (15 comp start positions)
2012-Feb-14: - changed return type of "draw_on_flag_pattern_match" to TIDX
             - fixed bug in same function (off by one index!)
             - changed order of special resources
2012-Feb-13: added scenario "big_river"
2012-Feb-10..12: - Added scenario "The big river"
                 - added comments
                 - renamed old scenarios to english names
2010-Jul-27: Timing optimizations
2010-Jun-21: init lib here, not in the scenarios
2010-Jun-09: Include a checksum (signature) into log file
2010-May-09: Put "random" functions in library
2010-May-02: Put all scenarios in separate files (finished)
2010-Apr-20..22: new func "rate_1st_pos"
2010-Apr-18: Put all scenarios in separate files (to be continued)
2010-Jan-24: Minor cosmetic changes
2009-Apr-12: Use of global var "mapfile"
2009-Mar-16: Debugging
2009-Jan-11: ini file implementation
2008-Aug-30: Debugging
2008-Jul-13: "add_starting_positions" now respects "dont_settle_down" flags
2008-Jun-01: Auslagern von "Nav_required"
2008-Apr-26: - Parameter modification for "Gebirge"
             - set_ocean_and_coast & set_bonus_resources more versatile
2008-Mar-01..Apr-06: Scenario "Gebirge"
2007-Dec-01: Scenario "Inseln/Navigation required" with more than 2 opponents
2007-May-21: Bug in "tag_city_radius_as_DONT_USE" fixed
2007-May-12..13: Scenario "Suedsee" Many small islands, clustered closely
2007-Mar-15..20: Scenario "Inseln/Navigation required"
2006-Feb-22..Mar-04: initial version
*****************************************************************************
Global objects:
- void main(int argc, char** argv)
- void write_map_to_file( char* filename )
- TILE draw_tile( PROBABILITY_VECTOR* pv )
- void eliminate_one_tile_islands( void )
- void set_ocean_and_coast( void )
- void set_bonus_resources( void )
- void add_starting_positions( STARTPOS_PARAMS* params )
- U32 draw_on_flag_pattern_match( U8 world_flags_mask, U8 pattern )
- void map_gen_exit( void )
- void log_with_timestamp( char* msg )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /*for "strcmp"*/

#include <misc.h>
#include <file_sys.h>
#include <file_io.h>
#include <openfile.h>
#include <getopts.h>
#include <falloc.h>
#include <now.h>
#include <stdint.h>

/*#define DEBUG*/
#include <debug.h>

#include "map_gen.h"
#include "read_ini.h"


/*--  constants  ----------------------------------------------------------*/

/*#define VERSION_STR  "1.0"*/ /*for 1st release*/
#define VERSION_STR  "1.1.0" /*for 2nd release*/
#define VERSION_CNT  2 /*release enumeration*/

#define DEFAULT_INI_FILE_NAME "map_gen.ini"
#define DEFAULT_LOG_FILE_NAME "map_gen.log"


/*--  type declarations & enums  ------------------------------------------*/



/*--  local function prototypes  ------------------------------------------*/

static void usage( void ) ;
static void gnu_message( void ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/

FILE* log_fp ; /*for the log file*/

int map_size_idx ; /*0 for 35%, 1 for 50%, ... , 5 for 230%*/
      /*map_size_idx can be used to select parameters dependent on map size*/

TILE start_pos_mask [16] ; /*0=human, 1..14=comp*/ /*+1 comp pos*/
TIDX start_pos_idx  [16] ; /*0=human, 1..14=comp*/
   /*why size 16?? (and not 15?)*/
   /*Because "add_starting_positions" writes from index 1 thru 15*/

TILE special_resource_tab [12] = {
/*The order of resources is important:
- The first three resources are sufficient to make a map playable
- Even and odd resource indices give each side (big river) a monopol*/
  COBALT,
              URANIUM,
  MERCURY,
              COBALT,
  MERCURY,
              URANIUM,

  DEAD_LANDS, DEAD_LANDS,
  DEAD_LANDS, DEAD_LANDS,
  DEAD_LANDS, DEAD_LANDS
} ;


/*--  internal variables  -------------------------------------------------*/


	/* Options */
static char flag_no_copyright ;
static char flag_dont_append_to_log ;
static char flag_ini_file ;
static char* ini_file_arg = DEFAULT_INI_FILE_NAME ;
static char flag_count ;
static unsigned long count ;

OPTION_LIST(optlist)
/*OPTION_WO_ARG('x',flag_x)*/
OPTION_WO_ARG('g',flag_no_copyright)
OPTION_WO_ARG('a',flag_dont_append_to_log)
OPTION_W_ARG('i',flag_ini_file,ini_file_arg)
/*OPTION_W_ARG('s',flag_seed,seed_arg)*/
OPTION_NUMBER(flag_count,count)
OPTION_LIST_END

/*-------------------->   main   <------------------------------- 2017-Mar-24
Purpose see file header
-----------------------------------------------------------------------------
Used functions: get_opts, usage, printf, fprintf, exit,
                strcmp,
                scenario_*
Parameters:	- argc, argv
Return value:	- 0: ok
		- 1: error
Exitcode:	x
---------------------------------------------------------------------------*/
int main( int argc, char** argv )
{
THIS_FUNC(main)
  uint64_t seed , seed2 ;
  FILE *seeYear;
  U8 i ; /*loop control*/

  if (argc == 2) {
    if (strcmp( argv [1], "--version" ) == 0) {
      printf( "Map-Gen Version %s, a configurable map generator for C-Evo\n",
        VERSION_STR ) ;
      gnu_message() ;
      exit( EXITCODE_OK ) ;
    }
    if (strcmp( argv [1], "--help" ) == 0) {
      usage() ;
    }
  }


  OPTION_GET(optlist) /*read and evaluate all command line args*/
  if ( argc != 0 ) {
     usage() ;
  }
  gnu_message() ;

  if ( ! file_sys_file_is_plain( ini_file_arg )) {
    fprintf( stderr, "map_gen: cannot find %s\n", ini_file_arg ) ;
    exit( EXITCODE_FILE_NOT_FOUND ) ;
  }

  /*REDIRECT("map_gen.log")*/
  if (flag_dont_append_to_log) {
    log_fp = forced_fopen_w( DEFAULT_LOG_FILE_NAME, OPENFILE_TEXT ) ;
  }
  else {
    log_fp = forced_fopen_w( DEFAULT_LOG_FILE_NAME,
                  OPENFILE_TEXT | OPENFILE_APPEND ) ;
    fprintf( log_fp,
"########   Separator line between map_gen runs   ###########################\n"
           ) ;
  }

  /*TIME_STAMP( "generated by \"map_gen.exe\" Version " VERSION_STR )*/
  log_with_timestamp( "generated by \"map_gen\" Version " VERSION_STR ) ;

  fprintf( log_fp, "signature=0x%08lx\n",
                (long)read_ini_checksum( ini_file_arg, VERSION_CNT )) ;

     /*init random generator*/
  seed2 = 0;
  seeYear = fopen("seed2.txt","r");
  if(seeYear) {
	int see;
	while(!feof(seeYear)) {
		see = getc(seeYear);
		if(see >= '0' && see <= '9') {
			seed2 *= 10;
			seed2 += see - '0';
		}
	}
  } else {
	seed2 = 777;
  }
  if(flag_count) { seed2 = 0; }
  seed = random_init( flag_count, count, seed2 ) ;
  fprintf( log_fp, "Seed value %llu\n", seed) ;
  fprintf( log_fp, "Seed2 value %llu (not used if 0)\n", seed2) ;

  fprintf( log_fp,
     "close/reopen log file to save seed in case of a segmentation fault\n" ) ;
  forced_fclose( log_fp ) ;
  log_fp = forced_fopen_w( DEFAULT_LOG_FILE_NAME,
                           OPENFILE_TEXT | OPENFILE_APPEND ) ;

     /*Read map_gen.ini and put all values in global variables*/
  if ( read_ini( ini_file_arg ) ) {
    fprintf( log_fp, "map_gen: read_ini error\n" ) ;
    printf( "map_gen: read_ini error\n" ) ;
    exit( EXITCODE_SYNTAX_ERR ) ;
  }

  fprintf( log_fp, "map_type_name = %s\n", map_type_name ) ;
  fprintf( log_fp, "map_size = %u\n", (unsigned)map_size ) ;

  /* NOTE: LX and LY can be changed by map_x and map_y */
  switch ( map_size ) {
  case 35: /*1380 tiles = 33%*/
     LX = 30 ;
     LY = 46 ;
     map_size_idx = 0 ;
     break ;

  case 50: /*2080 tiles = 50%*/
     LX = 40 ;
     LY = 52 ;
     map_size_idx = 1 ;
     break ;

  case 70: /*3000 tiles = 71%*/
     LX = 50 ;
     LY = 60 ;
     map_size_idx = 2 ;
     break ;

  case 100: /*4200 tiles = 100%*/
     LX = 60 ;
     LY = 70 ;
     map_size_idx = 3 ;
     break ;

  case 150: /*6150 tiles = 146%*/
     LX = 75 ;
     LY = 82 ;
     map_size_idx = 4 ;
     break ;

  case 230: /*9600 tiles = 229%*/
     LX = 100 ;
     LY = 96 ;
     map_size_idx = 5 ;
     break ;

  default:
    fprintf( log_fp, "Invalid map size\n" ) ;
     printf(         "Invalid map size\n" ) ;
    exit ( EXITCODE_WRONG_PARAM ) ;
  }

  /* While map_size must be set, we can set the actual size by hand */
  if(map_x > 22 && map_y > 33) { LX = map_x; LY= map_y; }
  LY=LY & 0xfe; /* Odd numbers for LY not allowed, as per map spec */

     /*allocate & initialize map*/
  cevo_lib_init() ;

  /*PRT_VAR((unsigned)comp_opponents,u)*/
  fprintf( log_fp, "comp_opponents = %u\n", (unsigned)comp_opponents ) ;

  start_pos_mask [0] = HUMAN_STARTPOS ;
  for ( i = 1 ; i <= comp_opponents ; i++ ) {
    start_pos_mask [i] = NORMAL_STARTPOS ;
  }

     /*default minimum requirement for startpos*/
  if ( ! found_startpos_min_rating) {
    startpos_min_rating = 41 ;
  }

     /*pass control to the respective scenario module*/
  if (strcmp( map_type_name, "Navigation_required" ) == 0) {
    scenario_nav_required() ;
  }
  else if (strcmp( map_type_name, "Hard_fight" ) == 0) {
    scenario_hard_fight() ;
  }
  else if (strcmp( map_type_name, "Micronesia" ) == 0) {
    scenario_micronesia() ;
  }
  else if (strcmp( map_type_name, "Mountains" ) == 0) {
    scenario_mountains() ;
  }
  else if (strcmp( map_type_name, "Desert" ) == 0) {
    scenario_desert() ;
  }
  else if (strcmp( map_type_name, "Arctic" ) == 0) {
    scenario_arctic() ;
  }
  else if (strcmp( map_type_name, "The_big_river" ) == 0) {
    scenario_big_river() ;
  }
  else if (strcmp( map_type_name, "Great_Plains" ) == 0) {
    scenario_great_plains() ;
  }
  else if (strcmp( map_type_name, "Volcano_Islands" ) == 0) {
    scenario_volcano_islands() ;
  }
  else if (strcmp( map_type_name, "Fjords" ) == 0) {
    scenario_fjords() ;
  }
  else {
    fprintf( log_fp, "unknown map type: %s\n", map_type_name ) ;
     printf(         "unknown map type: %s\n", map_type_name ) ;
    exit( EXITCODE_SYNTAX_ERR ) ;
  }

  write_map_to_file( mapfile ) ;
  /*TIME_STAMP( "Map written" )*/
  log_with_timestamp( "Map written" ) ;
  printf( "Map \"%s\" written successfully\n", mapfile ) ;

  forced_fclose( log_fp ) ;
  return EXITCODE_OK ;
}

/*-------------------->   usage   <------------------------------ 2017-Mar-24
This function displays an ultra-short usage instruction.
-----------------------------------------------------------------------------
Used functions: gnu_message, printf, exit
Parameters:	--
Return value:	-- (doesn't return)
Exitcode:	EXITCODE_USAGE
---------------------------------------------------------------------------*/
static void usage( void )
{
   gnu_message() ;
   printf(
"usage: map_gen [options]\n"
" Options:\n"
" --version     prints a version string\n"
" --help        prints this help message (for further help see\n"
"               the \"readme.txt\" file that came with map_gen)\n"
" -i filename   use filename as configuration file (default: %s)\n"
" -a            overwrite log file \"%s\", do not append\n"
" -g            suppress the GNU copyright message\n"
" -number       a decimal positive number.  Serves as a seed value\n"
"               for map generation.  For debugging purposes only.\n"
          ,DEFAULT_INI_FILE_NAME
          ,DEFAULT_LOG_FILE_NAME
         ) ;
   exit( EXITCODE_USAGE ) ;
}

/*-------------------->   gnu_message   <------------------------ 2017-Mar-24
This function prints the GNU copyright message to stdout, unless -g is given.
-----------------------------------------------------------------------------
Used functions: printf
Globals:   --
Internals: flag_no_copyright
Parameters:   --
Return value: void
Exitcode:     --
---------------------------------------------------------------------------*/
static void gnu_message( void )
{
THIS_FUNC(gnu_message)
  if ( ! flag_no_copyright) {
    printf(
      "Copyright (C) 2017  Ulrich Krueger\n"
      "Map_gen comes with ABSOLUTELY NO WARRANTY.\n"
      "This is free software: you are free to change and redistribute\n"
      "it under certain conditions; for details see \"gpl-3.0.txt\".\n"
          ) ;
  }
}

/*-------------------->   write_map_to_file   <------------------ 2008-Mar-23
This function the map to a file.
-----------------------------------------------------------------------------
Used functions: forced_fopen_wb, fprintf, put_U32_little_endian,
                forced_fclose
Parameters:	- filename   name of the file the map will be written to
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
void write_map_to_file( char* filename )
{
THIS_FUNC(write_map_to_file)
  FILE* outfile ;
  U32 tile_index ;

  outfile = forced_fopen_wb( filename ) ;
  fprintf( outfile, "cEvoMap%c", '\0' ) ;
  put_U32_little_endian( outfile, 0 ) ; /*format version*/
  put_U32_little_endian( outfile, 0 ) ; /*reserved for future use (MaxTurn)*/
  put_U32_little_endian( outfile, LX ) ;
  put_U32_little_endian( outfile, LY ) ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    put_U32_little_endian( outfile, 0x78000000 | world [ tile_index ] ) ;
  }
  forced_fclose( outfile ) ;
}

/*-------------------->   draw_tile   <-------------------------- 2014-Oct-08
This function returns a basic tile type according to a given probability
vector.
-----------------------------------------------------------------------------
Used functions: random_draw
Parameters:	- pv   !! probability vector is being modified !!
Return value:	basic tile type
Exitcode:	--
---------------------------------------------------------------------------*/
TILE draw_tile( PROBABILITY_VECTOR* pv )
{
THIS_FUNC(draw_tile)
  TILE tile ;
  U32 sum ;
  unsigned temp ;


  /*printf( "DRAW_TILE\n" ) ;*/
  /*DEB_STATEMENT(print_pv( pv ) ;)*/
  sum = pv->water
      + pv->grassland
      + pv->desert
      + pv->prairie
      + pv->tundra
      + pv->arctic
      + pv->swamp
      + pv->forest
      + pv->hills
      + pv->mountains ;

  pv->water = (pv->water * 0x7fff) / sum ;
  pv->grassland = pv->water + (pv->grassland * 0x7fff) / sum ;
  pv->desert = pv->grassland + (pv->desert * 0x7fff) / sum ;
  pv->prairie = pv->desert + (pv->prairie * 0x7fff) / sum ;
  pv->tundra = pv->prairie + (pv->tundra * 0x7fff) / sum ;
  pv->arctic = pv->tundra + (pv->arctic * 0x7fff) / sum ;
  pv->swamp = pv->arctic + (pv->swamp * 0x7fff) / sum ;
  pv->forest = pv->swamp + (pv->forest * 0x7fff) / sum ;
  pv->hills = pv->forest + (pv->hills * 0x7fff) / sum ;
  pv->mountains = pv->hills + (pv->mountains * 0x7fff) / sum ;
  /*DEB_STATEMENT(print_pv( pv ) ;)*/
  /*PAUSE*/

  temp = (unsigned)random_draw() ; /*0..0x7fff*/

  if (temp < pv->water) {
    tile = OCEAN ;
  }
  else if (temp < pv->grassland) {
    tile = GRASSLAND ;
  }
  else if (temp < pv->desert) {
    tile = DESERT ;
  }
  else if (temp < pv->prairie) {
    tile = PRAIRIE ;
  }
  else if (temp < pv->tundra) {
    tile = TUNDRA ;
  }
  else if (temp < pv->arctic) {
    tile = ARCTIC ;
  }
  else if (temp < pv->swamp) {
    tile = SWAMP ;
  }
  else if (temp < pv->forest) {
    tile = FOREST ;
  }
  else if (temp < pv->hills) {
    tile = HILLS ;
  }
  else {
    tile = MOUNTAINS ;
  }
  return tile ;
}

#ifdef DEBUG
/*-------------------->   print_pv   <--------------------------- 2014-Oct-08
This function prints a probability vector to stdout.
-----------------------------------------------------------------------------
Used functions:
Globals:
Internals:
Parameters:   - x
              - x
Return value: x
Exitcode:     x
---------------------------------------------------------------------------*/
void print_pv( PROBABILITY_VECTOR* pv )
{
THIS_FUNC(print_pv)
  printf( "water:     %5lu\n"
          "grassland: %5lu\n"
          "desert:    %5lu\n"
          "prairie:   %5lu\n"
          "tundra:    %5lu\n"
          "arctic:    %5lu\n"
          "swamp:     %5lu\n"
          "forest:    %5lu\n"
          "hills:     %5lu\n"
          "mountains: %5lu\n",
          (unsigned long)(pv->water),
          (unsigned long)(pv->grassland),
          (unsigned long)(pv->desert),
          (unsigned long)(pv->prairie),
          (unsigned long)(pv->tundra),
          (unsigned long)(pv->arctic),
          (unsigned long)(pv->swamp),
          (unsigned long)(pv->forest),
          (unsigned long)(pv->hills),
          (unsigned long)(pv->mountains)
        ) ;
}
#endif /*DEBUG*/

/*-------------------->   eliminate_one_tile_islands   <--------- 2016-Aug-08
This function eliminates all one-tile islands from the map,
except for ARCTIC one-tile islands.

Reason: Cities on one-tile islands are too easy to defend in the beginning
of the game (neither long-range guns nor planes available).
          [Update:] "long-range guns" are now called "Ballistics"
This function must be called before converting OCEAN to COAST tiles.
-----------------------------------------------------------------------------
Used functions: set_neighborhood_tiles, is_land_tile
Parameters:	- total_no_of_tiles (global)
		- world (global)
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
void eliminate_one_tile_islands( void )
{
THIS_FUNC(eliminate_one_tile_islands)
  NEIGHBORHOOD NT ;
  U32 tile_index ;
  U32 eliminated_one_tile_islands = 0 ;
  TILE tile ;
  TILE* Tp ;
  NEIGHBORHOOD* NTp ;
  U8 i ;
  BIT land_found ;


  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_land_tile( tile_index ) && (world [ tile_index ] != ARCTIC)) {
                       /* Muss nicht BASIC_TILE_TYPE_MASK gesetzt werden???*/
      NTp = set_neighborhood_tiles( tile_index, & NT ) ;
      land_found = FALSE ; /*default*/
      for ( i = 0, Tp = & (NTp->n_tile) ; i < 8 ; i++, Tp++ ) {
        tile = (*Tp) & BASIC_TILE_TYPE_MASK ;
        switch (tile) {
        case OCEAN:
        case (NORTH_POLE & BASIC_TILE_TYPE_MASK):
        case (SOUTH_POLE & BASIC_TILE_TYPE_MASK):
          break ;

        default:
          land_found = TRUE ;
          break ;
        }
      }
      if ( ! land_found) { /*one tile island*/
        /*PRT_VAR((unsigned long)tile_index,lu)*/
        /*PRT_VAR(world [ tile_index ],08lx)*/
        world [ tile_index ] = OCEAN ;
        eliminated_one_tile_islands++ ;
      }
    }
  }
  fprintf( log_fp,
     "Eliminated one-tile islands: %lu\n",
     (unsigned long)eliminated_one_tile_islands ) ;
}

/*-------------------->   set_ocean_and_coast   <---------------- 2008-Apr-26
This function converts each WATER tile (OCEAN or COAST) to the correct type
according to the vicinity of land tiles.
It must be called before setting the bonus resources.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- total_no_of_tiles (global)
		- world (global)
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
void set_ocean_and_coast( void )
{
THIS_FUNC(set_ocean_and_coast)
  U32 tile_index ;

  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (is_in_TTL( tile_index, TTL_WATER )) {
      if (count_city_tiles( tile_index, TTL_LAND & ~TTL_ARCTIC ) > 0) {
        world [ tile_index ] = COAST ;
      }
      else {
        world [ tile_index ] = OCEAN ;
      }
    }
  }
}

/* Function added by Sam Trenholme
   Given an (x,y) value, see if any bonus resources are in an adjacent
   tile.  This is ugly because of the iso tiling. */
U32 adjacent_resource(int x, int y) {
	int a;
	int offsetX, offsetY, viewX, viewY;
  	U16 tile_index ;
  	U32 tile ;
	U32 out = 0;
	for(a = 0; a < 8; a++) {
		/* Handle the ISO tiling with a complex switch statement */
		switch(a) {
			case 0:
				offsetX = 0; offsetY = -2;
				break;
			case 1:
				if(y % 2 == 0) {
				  offsetX = -1; offsetY = -1;
				} else {
				  offsetX = 0; offsetY = -1;
				}
				break;
			case 2:
				if(y % 2 == 0) {
				  offsetX = 0; offsetY = -1;
				} else {
				  offsetX = 1; offsetY = -1;
				}
				break;
			case 3:
				offsetX = -1; offsetY = 0;
				break;
			case 4:
				offsetX = 1; offsetY = 0;
				break;
			case 5:
				if(y % 2 == 0) {
				  offsetX = -1; offsetY = 1;
				} else {
				  offsetX = 0; offsetY = 1;
				}
				break;
			case 6:
				if(y % 2 == 0) {
				  offsetX = 0; offsetY = 1;
				} else {
				  offsetX = 1; offsetY = 1;
				}
				break;
			case 7:
				offsetX = 0; offsetY = 2;
				break;
			default:
				offsetX = 0; offsetY = 0;
				break;
		}
		viewX = x + offsetX;
		viewY = y + offsetY;
		if(viewX < 0) { viewX += LX; }
		if(viewX >= LX) { viewX -= LX; }
		if(viewY < 0 || viewY >= LY) { continue; } /* Off the map */
		tile_index = (viewY * LX) + viewX;
		tile = world [ tile_index ];
		tile &= (BONUS_RESOURCE_1_MASK | BONUS_RESOURCE_2_MASK);
		out |= tile;	
	}		
	return out;
}

/*-------------------->   set_bonus_resources   <---------------- 2008-Apr-26
This function adds bonus resources to the world map,
including GRASSLAND to PLAINS conversion.

Caveat:
OCEAN to COAST conversion must be done before calling this function!
-----------------------------------------------------------------------------
Used functions:
Parameters:	none (uses global map_resources value)
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
void set_bonus_resources( void )
{
/*THIS_FUNC(add_bonus_resources)*/
THIS_FUNC(set_bonus_resources)
  U16 line, col ;
  U16 col_offset ;
  U16 tile_index ;
  U32 tile ;

static U16 land1_offsets [] = {
/*line, col*/
     2, 3,
     4, 6,
     4, 6,
     4, 6,
     2, 3,
     0
} ;

static U16 land2_offsets [] = {
/*line, col*/
     2, 3,
     8, 2,
     2, 3,
     4, 6,
     0
} ;

static U16 plains_offsets [] = {
/*col*/
  2,
  3,
  0
} ;

  U16* program ;
  U16* program_p ;


     /*clear all bonus resources*/
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    world [ tile_index ] &= ~BONUS_RESOURCES_MASK ;
  }


  program = land1_offsets ;
  program_p = program ;
  line = 0 ;
  col_offset = 0 ;
  while (line < LY) {
    if(map_resources == 0) { col = col_offset ; } else { col = 0; }
      while (col < LX) {
      tile_index = line * LX + col ;
      tile = world [ tile_index ] ;
      if(map_resources == 0 || ((random_draw_range( 1,100 ) < map_resources)
	&& (tile & BONUS_RESOURCE_1_MASK) == 0  
        && (tile & BONUS_RESOURCE_2_MASK) == 0
	&& adjacent_resource(col,line) == 0)) {
        switch (tile & BASIC_TILE_TYPE_MASK) {
          case PRAIRIE:
          case DESERT:
          case SWAMP:
          case TUNDRA:
          case ARCTIC:
          case FOREST:
          case HILLS:
          case MOUNTAINS:
              world [ tile_index ] = tile | BONUS_RESOURCE_1_MASK ;
              break ;

          default:
              break ;
        }
      }
      /* Let the user add more oases as desired via map_oasis */
      if(map_resources > 0 && (tile & BASIC_TILE_TYPE_MASK) == DESERT &&
         (tile & BONUS_RESOURCE_1_MASK) == 0 &&
         (tile & BONUS_RESOURCE_2_MASK) == 0 && map_oasis > 0 &&
	 adjacent_resource(col,line) == 0 &&
         (random_draw_range( 1,100 ) < map_oasis)) {
          world [ tile_index ] = tile | BONUS_RESOURCE_1_MASK ;
      }
      if(map_resources == 0) { col += 10 ; } else { col += 1 ; }
    }
    /*end column, next line*/
    if(map_resources == 0) {
      if (*program_p == 0) { /*end of program*/
        program_p = program ; /*restart program from beginning*/
      }
      line += *program_p++ ;
    } else { line++; }
    col_offset = (col_offset + *program_p++) % 10 ;
  }

  program = land2_offsets ;
  program_p = program ;
  line = 2 ;
  col_offset = 8 ;
  while (line < LY) {
    if(map_resources == 0) { col = col_offset ; } else { col = 0; }
    while (col < LX) {
      tile_index = line * LX + col ;
      tile = world [ tile_index ] ;
      if(map_resources == 0 || ((random_draw_range( 1,100 ) < map_resources)
	&& (tile & BONUS_RESOURCE_1_MASK) == 0  
        && (tile & BONUS_RESOURCE_2_MASK) == 0
        && adjacent_resource(col,line) == 0)) {
          switch (tile & BASIC_TILE_TYPE_MASK) {
          case PRAIRIE:
          case DESERT:
          case SWAMP:
          case TUNDRA:
          case FOREST:
          case HILLS:
          case MOUNTAINS:
            world [ tile_index ] = tile | BONUS_RESOURCE_2_MASK ;
            break ;

          case ARCTIC:
            world [ tile_index ] = tile | BONUS_RESOURCE_1_MASK ;
            break ;

          default:
            break ;
        }
      }
      if(map_resources == 0) { col += 10 ; } else { col += 1 ; }
    }
    /*end column, next line*/
    if(map_resources == 0) {
      if (*program_p == 0) { /*end of program*/
        program_p = program ; /*restart program from beginning*/
      }
      line += *program_p++ ;
    } else { line++; }
    col_offset = (col_offset + *program_p++) % 10 ;
  }

     /*coast resource 1 (fish)*/
  line = 2 ;
  col_offset = 3 ;
  while (line < LY) {
    col = col_offset ;
    while (col < LX) {
      tile_index = line * LX + col ;
      tile = world [ tile_index ] ;
      if ((tile & BASIC_TILE_TYPE_MASK) == COAST) {
        world [ tile_index ] = tile | BONUS_RESOURCE_1_MASK ;
      }
      col += 10 ;
    }
    /*end column, next line*/
    line += 4 ;
    col_offset = (col_offset + 6) % 10 ;
  }

     /*coast resource 2 (manganese)*/
  line = 0 ;
  col_offset = 0 ;
  while (line < LY) {
    col = col_offset ;
    while (col < LX) {
      tile_index = line * LX + col ;
      tile = world [ tile_index ] ;
      if ((tile & BASIC_TILE_TYPE_MASK) == COAST) {
        world [ tile_index ] = tile | BONUS_RESOURCE_2_MASK ;
      }
      col += 10 ;
    }
    /*end column, next line*/
    line += 16 ;
    col_offset = (col_offset + 4) % 10 ;
  }

  program = plains_offsets ;
  program_p = program ;
  line = 0 ;
  col_offset = 1 ;
  while (line < LY) {
    col = col_offset ;
    while (col < LX) {
      tile_index = line * LX + col ;
      tile = world [ tile_index ] ;
      if ((tile & BASIC_TILE_TYPE_MASK) == GRASSLAND) {
        world [ tile_index ] = tile | BONUS_RESOURCE_1_MASK ;
      }
      col++ ;
      if (col < LX) {
        tile_index = line * LX + col ;
        tile = world [ tile_index ] ;
        if ((tile & BASIC_TILE_TYPE_MASK) == GRASSLAND) {
          world [ tile_index ] = tile | BONUS_RESOURCE_1_MASK ;
        }
      }
      col += 3 ;
    }
    /*end column, next line*/
    if (*program_p == 0) {
      program_p = program ;
    }
    line++ ;
    col_offset = (col_offset + *program_p++) % 4 ;
  }
}

/*-------------------->   add_starting_positions   <------------- 2016-Aug-08
This function adds computer starting positions all over the map.
Existing "dont_settle_down" flags are respected.
-----------------------------------------------------------------------------
Used functions:
Globals: world [], world_flags []
         start_pos_idx [1..number] is set
Internals: log_fp
Parameters:	- params   ... for placing start positions
Return value:	void
Exitcode:	EXITCODE_UNSPECIFIED_ERR
---------------------------------------------------------------------------*/
void add_starting_positions( STARTPOS_PARAMS* params )
{
THIS_FUNC(add_starting_positions)
  U32 tile_index, best_index ;
  U8* ratings ; /*size: total_no_of_tiles*/
  /*U16 rating, best_rating ;*/
  U16 best_rating ;
  U8 flags ;
  U8 i ; /*loop control*/


  /*TIME_STAMP("Entry add_starting_positions")*/
  log_with_timestamp( "Entry add_starting_positions" ) ;

  ASSERT(params->number < 16)
  params->actual_number = 0 ;
  clear_flags( params->city_tag ) ; /*clear all city_tag world_flags*/
     /*leave existing "dont_settle_down" flags untouched*/
  ASSERT((params->city_tag & params->dont_settle_down) == 0)

  /*PRT_VAR(params->city_dist,u)*/
  /*PRT_VAR((unsigned)(params->min_rating),u)*/
  fprintf( log_fp, "add_starting_positions: params->city_dist = %u\n",
                   params->city_dist ) ;
  fprintf( log_fp, "add_starting_positions: params->min_rating = %u\n",
                   (unsigned)(params->min_rating) ) ;

     /*step 1: rate all tiles on map, unless "dont_settle_down" is set*/
  ratings = (U8*)falloc( (unsigned)total_no_of_tiles ) ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if (((world_flags [ tile_index ]) & params->dont_settle_down) != 0) {
      ratings [ tile_index ] = 0 ;
    }
    else {
      ratings [ tile_index ] = rate_1st_pos( tile_index ) ; /*uses TEMP_TAG*/
    }
  }
  /*TIME_STAMP("all ratings done")*/
  log_with_timestamp( "all ratings done" ) ;

     /*step 2: select from previously rated tiles*/
  for ( i = 1 ; i <= params->number ; i++ ) {
    /*PRT_VAR((unsigned)i,u)*/
    best_rating = 0 ;
    best_index = 0 ;
    for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
      /*PRT_VAR((unsigned long)tile_index,lu)*/
      flags = world_flags [ tile_index ] ;
      if ( ! (flags & params->dont_settle_down)) {
        if (ratings [ tile_index ] > best_rating) {
          best_rating = ratings [ tile_index ] ;
          best_index = tile_index ;
        }
      }
    }
    /*PRT_VAR(best_rating,u)*/
    /*PRT_VAR((unsigned long)best_index,lu)*/
    fprintf( log_fp, "add_starting_positions: best_rating = %u\n",
                     best_rating ) ;
    fprintf( log_fp, "add_starting_positions: best_index = %lu\n",
                     (unsigned long)best_index ) ;

    /*TIME_STAMP("found best position")*/
    if (best_rating < params->min_rating) {
      /*if (params->try_irrigation) {*/
      /*}*/
      /*else {*/
        fprintf( log_fp,
          "add_starting_positions: could place only %u starting positions\n",
          (unsigned)(i -1) ) ;
        printf(
          "add_starting_positions: could place only %u starting positions\n",
          (unsigned)(i -1) ) ;
        map_gen_exit() ;
      /*}*/
    }
    if ((params->make_worst_pos_human) && (i == params->number)) {
      world [ best_index ] |= HUMAN_STARTPOS ;
    }
    else {
      world [ best_index ] |= NORMAL_STARTPOS ;
    }
    ASSERT(i < 16)
    start_pos_idx [i] = best_index ;

       /*tag city tiles for some reasons (service for calling function)*/
    tag_city_tiles( best_index, params->city_tag ) ;

    tag_inside_radius( best_index, params->city_dist - 50,
                                   params->dont_settle_down ) ;

    (params->actual_number)++ ;
  } /*end for loop*/
  /*for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {*/
    /*if (world_flags [tile_index] & params->dont_settle_down) {*/
      /*world [tile_index] &= ~BASIC_TILE_TYPE_MASK ;*/
      /*world [tile_index] |= GRASSLAND ;*/
    /*}*/
  /*}*/
  /*TIME_STAMP("Exit add_starting_positions")*/
  log_with_timestamp( "Exit add_starting_positions" ) ;
}

/*-------------------->   draw_on_flag_pattern_match   <--------- 2016-Aug-08
This function returns the index of one of all tiles
whose world_flags match a certain pattern.
The tile is picked randomly.
-----------------------------------------------------------------------------
Used functions: random_draw_range, fprintf, exit
Parameters:	- world_flags_mask
		- pattern
Return value:	Tile index, or 0xffffffff if no appropriate tile is found
Exitcode:	;EXITCODE_UNEXPECTED_ERR
---------------------------------------------------------------------------*/
TIDX draw_on_flag_pattern_match( U8 world_flags_mask, U8 pattern )
{
THIS_FUNC(draw_on_flag_pattern_match)
  U32 tile_index ;
  U16 count ;
  U16 random_number ;

     /*count tiles with matching flag pattern*/
  count = 0 ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if ((world_flags [ tile_index ] & world_flags_mask) == pattern) {
      count++ ;
    }
  }
  /*PRT_VAR((unsigned)count,u)*/
  if (count == 0) {
    /*return 0xffffffff ;*/
    return INVALID_TIDX ;
  }
     /*choose one of appropriate tiles*/
  random_number = random_draw_range( 1, count ) ;
  /*ASSERT(random_number >= 1)*/
  /*ASSERT(random_number <= count)*/
  count = 0 ;
  for ( tile_index = 0 ; tile_index < total_no_of_tiles ; tile_index++ ) {
    if ((world_flags [ tile_index ] & world_flags_mask) == pattern) {
      count++ ;
      if (count == random_number) {
        return tile_index ;
      }
    }
  }
  fprintf( log_fp, "draw_on_flag_pattern_match: unexp. error\n" ) ;
  /*exit( EXITCODE_UNEXPECTED_ERR ) ;*/
  map_gen_exit() ;
  return 0 ; /*unreachable code, just to make gcc happy*/
}

/*-------------------->   map_gen_exit   <----------------------- 2013-Aug-08
This function writes a debug map, outputs an abort message and exits.
-----------------------------------------------------------------------------
Used functions: write_map_to_file, printf, exit
Parameters:	--
Return value:	-- (does not return)
Exitcode:	EXITCODE_UNSPECIFIED_ERR
---------------------------------------------------------------------------*/
void map_gen_exit( void )
{
THIS_FUNC(map_gen_exit)

  TIME_STAMP( "writing \"debug.cevo map\"" )
  write_map_to_file( "debug.cevo map" ) ;
  printf( "Map generation aborted.  See \"map_gen.log\" for details.\n"
          "CAUTION: The map \"debug.cevo map\" is not playable!!\n" ) ;
  forced_fclose( log_fp ) ;
  exit( EXITCODE_UNSPECIFIED_ERR ) ;
}

/*-------------------->   log_with_timestamp   <----------------- 2016-Aug-08
This function prints the current date & time to log_fp, followed by a
caller supplied message.
-----------------------------------------------------------------------------
Used functions: now, fprintf
Globals: --
Internals: log_fp
Parameters:   - msg: caller supplied message
Return value: void
Exitcode:     --
---------------------------------------------------------------------------*/
void log_with_timestamp( char* msg )
{
THIS_FUNC(log_with_timestamp)
  VERSATILE_TIME_STRUCT vts ;

  now( & vts, NOW_ISO | NOW_NO_NEWLINE ) ;
  fprintf( log_fp, "%s: %s\n", vts.output_string, msg ) ;
}

/*-------------------->   x   <---------------------------------- 2017-Mar-24
This function x
-----------------------------------------------------------------------------
Used functions:
Globals:   x
Internals: x
Parameters:   - x
              - x
Return value: x
Exitcode:     x
---------------------------------------------------------------------------*/
/*x*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/
/***************************************************************************/
