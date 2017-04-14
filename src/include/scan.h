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
scan.h

Include file for scan_lib.c
Covers scanm.c, scanf.c, scang.c and scancomm.c
*****************************************************************************
History: (latest change first)
2014-Jan-25: - added scan_double()
             - extended SCAN_PARAMS ('double real' return value)
2013-Dec-24: - added scan_until_charset()
2013-Nov-08..09: - added scan_while_charset(), scan_until_charset()
                 - introduced SCAN_STORE_TOKEN
2013-Sep-28..30: changed SCAN_PARAMS structure
2013-Aug-16: extended SCAN_PARAMS, added scanm_U32_radix
2013-Apr-28: added members to READLINE_PARAMS
2013-Apr-09..10: - added "readline_*"
                 - added SCAN_SKIP_LEADING_WHITES
2012-Dec-28: added "scanm_U32"
2012-Nov-25..26: added SCANG_*_FUNC
2011-Aug-11: renamed HUNT_FOR_STR to HUNT_STR to match the func prefix
2011-Aug-10: added functions "instr", "hunt_str_*"
2010-Apr-16..17: initial version
****************************************************************************/
#ifndef SCAN_H
#define SCAN_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

#include <misc.h>
#include <stdio.h>


/*--  constants  ----------------------------------------------------------*/

   /*Flags*/
#define SCAN_NO_FLAGS            0x0000

#define SCAN_EXIT_ON_ERROR       0x0001
            /*Defaults to TRUE.*/

#define SCAN_BACKSLASH_VERBATIM  0x0002
#define SCAN_NO_HEAP             0x0004
#define SCAN_READ_BEYOND_ZERO    0x0008
#define SCAN_SKIP_LEADING_WHITES 0x0010
#define SCAN_STORE_TOKEN         0x0020
            /*If FALSE, no tokens are stored to "dest"*/
            /*Useful to skip long tokens which would cause a buffer overflow*/
            /*Defaults to TRUE.*/

#define SCAN_NL_TERMINATES       0x4000
#define SCAN_ACCEPT_INCOMPLETE   0x8000

   /*internally used -- do not use this one*/
#define _SCAN_ALLOW_EMPTY_STRING 0x0800


   /*return flags (in "U16 scan_stat")*/
#define SCANSTAT_NOTHING_SCANNED  0x0001
#define SCANSTAT_INCOMPLETE       0x0002


/*--  typedefs & enums  ---------------------------------------------------*/

typedef struct scan_params {
     /*params for scan_*/ /*callback function which returns the next chr*/
     /*predefined standard functions: scan_from_mem, scan_from_file*/
  char (*get)( struct scan_params* ) ;

     /*params for scanm_*/
  char* p ; /*pointer into memory (will be updated by scanm_)*/

     /*params for scanf_*/
  FILE* fp ;

     /*general params*/
  char* dest ; /*where to put the scanned token, must not be NULL*/
  U16 dest_size ; /*including '\0'*/
  U16 flags ;
  U8 radix ; /*for numerical scans*/

     /*return values*/
  U32 number ; /*numerical return value (also used internally for exponent)*/
  double real ; /*numerical return value*/
  U16 scan_stat ; /*error bits*/
  U16 scan_len ; /*number of characters scanned (incl. whites and/or double quotes)*/
  U16 token_len ; /*number of characters scanned (excl. whites and/or double quotes)*/

     /*internal data*/
  char putback_chr ;
  BIT  putback_in_use ; /*TRUE if a putback character is pending*/
  char* dest_invalid ; /*1st invalid dest, always "dest + dest_size"*/
  char* remember_dest ;
} SCAN_PARAMS ;

typedef char (*SCAN_GET_FUNC_P)( SCAN_PARAMS* ) ;


typedef struct {
  char* str ;
  char* p ;
} HUNT_STR ;

typedef struct {
  char* filename ;
  char* line_comment ; /*make this NULL to get complete line*/
  U32 max_line ; /*including '\0' and '\n' */ /*readline_init() allocates the buffer*/
  BIT exit_upon_error ;
  BIT accept_incomplete_last_line ;

     /*return values*/
  char* line ; /*w/o '\n'*/
  char* line_wo_comments ;
  char* line_wo_comments_and_whites ;
  U32 line_no ; /*line number (1st line is 1)*/
  U32 line_len ; /*w/o '\n' and '\0', i. e. an empty line has length 0*/
  BIT eof ;
     /*statistics*/
  U32 file_size ;
  U32 max_line_len ; /*max line length read so far*/
  U32 max_line_no ; /*number of line with max length*/

     /*private data*/
  FILE* fp ;
  U32 magic ;
  U32 magic_allocated ;
  char* buf1 ;
  char* buf2 ;
  char* buf3 ;
  U32 char_cnt ; /*to be checked against file_size*/
} READLINE_PARAMS ;


/*--  function prototypes  ------------------------------------------------*/

/*scanm_* functions read from m=memory*/ /*not for new development*/
/*scanf_* functions read from f=file*/ /*not for new development*/
/*scan_*  functions read thru a callback function*/


extern SCAN_PARAMS* scan_new( SCAN_GET_FUNC_P func ) ; /*contains init*/
extern void scan_init( SCAN_PARAMS* p, SCAN_GET_FUNC_P func ) ;

extern char scan_from_mem( SCAN_PARAMS* par ) ; /*two standard callback funcs*/
extern char scan_from_file( SCAN_PARAMS* par ) ;


extern char scan_get_next_chr ( SCAN_PARAMS* par ) ;
extern char scan_peep_next_chr( SCAN_PARAMS* par ) ;

              /*cmp to str, TRUE if not equal.  Can skip leading whites, too.*/
extern BIT  scan_check_str(     SCAN_PARAMS* par, char* str ) ;

extern BIT  scan_while_charset( SCAN_PARAMS* par, char* charset ) ;
extern BIT  scan_until_charset( SCAN_PARAMS* par, char* charset ) ;

extern BIT  scan_identifier(    SCAN_PARAMS* par ) ;
extern BIT  scan_U32(           SCAN_PARAMS* par ) ;
extern BIT  scan_double(        SCAN_PARAMS* par ) ;
extern BIT  scan_string(        SCAN_PARAMS* par ) ;

extern char* scanm_string( char** pp, char* dest, U16 flags ) ; /*NOT  FOR  NEW  DEVELOPMENT -- use scan_string*/
extern U32   scanm_U32(    char** pp,             U16 flags ) ; /*NOT  FOR  NEW  DEVELOPMENT -- use scan_U32*/
extern BIT   scanm_U32_radix( SCAN_PARAMS* par ) ; /*NOT  FOR  NEW  DEVELOPMENT -- use scan_U32*/

extern void scanf_skip_charset( FILE* fp, char* charset ) ;
extern char* scanf_string( FILE* fp, char* dest, U16 flags ) ;


extern BIT readsect_init( char* filename, char* section_name ) ;
extern int readsect_get_chr( void ) ; /*closes file at end of section*/

extern BIT readline_init( READLINE_PARAMS* p ) ; /*TRUE if file not found*/
extern BIT readline_get_line( READLINE_PARAMS* p ) ; /*TRUE on EOF*/


extern int instr( char chr, char* set ) ; /*similar to strchr*/

extern void hunt_str_init( HUNT_STR* this, char* str ) ;
extern BIT  hunt_str_test( HUNT_STR* this, char  chr ) ;


/*--  macros  -------------------------------------------------------------*/

/*--  global variables  ---------------------------------------------------*/

   /*wird im Moment noch fuer scanf und scanm gebraucht!*/
extern U16 scan_stat ; /*error bits*/
extern U16 scan_len ; /*number of characters scanned*/
extern U16 scan_string_len ;


/***************************************************************************/
#endif	/* SCAN_H */
/***************************************************************************/
