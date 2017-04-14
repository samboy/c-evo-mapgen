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
cpp_getc.c

This file x
*****************************************************************************
History: (latest change first)
2013-Feb-25: changed local includes from <...> to "..."
2003-Aug-15..Oct-31: initial version
*****************************************************************************
Global objects:
- see "cpp_getc.h"
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*only for testing purposes*/



/*--  include files  ------------------------------------------------------*/

#include <misc.h>
#include <stdio.h>
#include <openfile.h>
/*#include <stdlib.h>*/
#include <ctype.h>

#include "inc_stk.h"
#include "mac_tab.h"

/*#define DEBUG*/
#include <debug.h>

extern FILE* infile ; /*defined in "cpp_main"*/


/*--  constants  ----------------------------------------------------------*/

/*#define MAX_ID_LENGTH 255*/


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/



/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/

BIT flag_is_white ; /*tab or blank*/
BIT flag_is_digit ;
BIT flag_is_letter_or_underscore ; /*identifiers must start with*/
BIT flag_is_alphanum_or_underscore ;

U16 cpp_line_no ;
char* cpp_file_name ;


/*--  internal variables  -------------------------------------------------*/

static int put_back_chr ; /*holds the character which was put back*/
static BIT flag_put_back = FALSE ;

static enum {
   LAST_SOURCE_NONE,
   LAST_SOURCE_FILE,
   LAST_SOURCE_MAC_TAB,
   LAST_SOURCE_PUT_BACK_QUEUE
} last_source = LAST_SOURCE_NONE ;


/*-------------------->   cpp_get_chr   <--------------------- 2003-Nov-02 --
This function returns the next character to process.
It evaluates several sources (given in order of priority):
  1. A macro expansion (nested)
  2. A character which has been put back previously
  3. Characters from input file resp. from include files

It tests the character it returns for several properties and sets the
flags accordingly (flag_is_*).
-----------------------------------------------------------------------------
Used functions: getc
Parameters:	--
Return value:	chr resp. EOF
Exitcode:	--
---------------------------------------------------------------------------*/
int cpp_get_chr( void )
{
THIS_FUNC(cpp_get_chr)
   int ret_val ;
   /*DEB_STATEMENT(char* debug_ret_val)*/
   int temp ;

   if ((ret_val = mac_tab_get_chr()) != EOF) {
      last_source = LAST_SOURCE_MAC_TAB ;
      DEB((stderr,"from mac_tab: >%c<\n", ret_val))
   }

   else if (flag_put_back) { /*read from put back queue*/
      ret_val = put_back_chr ;
      flag_put_back = FALSE ;
      last_source = LAST_SOURCE_PUT_BACK_QUEUE ;
      DEB((stderr,"from putback queue: >%c<\n", ret_val))
   }

   else { /*read from file*/
      /*ret_val = getc( infile ) ;*/
      ret_val = inc_stk_get_chr() ;
      last_source = LAST_SOURCE_FILE ;
      DEB((stderr,"from file: >%c<\n", (ret_val == EOF) ? '~' : ret_val))
   }

                 /*defaults*/
   flag_is_white = FALSE ;
   flag_is_digit = FALSE ;
   flag_is_letter_or_underscore = FALSE ;
   flag_is_alphanum_or_underscore = FALSE ;

      /*now set the flags according to the character value*/
   switch (ret_val) {
   case ' ':
   case '\t':
      flag_is_white = TRUE ;
      break ;

   case '_':
      flag_is_letter_or_underscore = TRUE ;
      flag_is_alphanum_or_underscore = TRUE ;
      break ;

   default:
      temp = toupper( ret_val ) ;
      if ((temp >= 'A') && (temp <= 'Z')) {
         flag_is_letter_or_underscore = TRUE ;
         flag_is_alphanum_or_underscore = TRUE ;
      }
      else {
         if ((ret_val >= '0') && (ret_val <= '9')) {
            flag_is_digit = TRUE ;
            flag_is_alphanum_or_underscore = TRUE ;
         }
      }
   } /*end switch*/

   return ret_val ;
}

/*-------------------->   cpp_put_back_chr   <---------------- 2003-Aug-16 --
This function puts one character back into the queue.
-----------------------------------------------------------------------------
Used functions: fprintf, exit, mac_tab_put_back_chr
Parameters:	--
Return value:	--
Exitcode:	EXITCODE_TABLE_FULL, EXITCODE_WRONG_CALL
---------------------------------------------------------------------------*/
void cpp_put_back_chr( int chr )
{
THIS_FUNC(cpp_put_back_chr)
   switch (last_source) {
   case LAST_SOURCE_FILE:
   case LAST_SOURCE_PUT_BACK_QUEUE:
      DEB((stderr,"to put_back queue: >%c<\n", chr))
      if (flag_put_back) {
         fprintf( stderr, "cannot put back more than one character\n" ) ;
         exit( EXITCODE_TABLE_FULL ) ;
      }
      else {
         flag_put_back = TRUE ;
         put_back_chr = chr ;
      }
      break ;

   case LAST_SOURCE_MAC_TAB:
      DEB((stderr,"to mac_tab: >%c<\n", chr))
      mac_tab_put_back_chr( chr ) ;
      break ;

   default:
      fprintf( stderr, "cpp_put_back_chr: put_back without previous get\n" ) ;
      exit( EXITCODE_WRONG_CALL ) ;
   }
}
/***************************************************************************/
