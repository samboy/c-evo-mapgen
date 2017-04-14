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
scanm.c

This library contains functions which scan null-terminated strings which
are located in memory ('m').
*****************************************************************************
History: (latest change first)
2013-Aug-16: added scanm_U32_radix()
2013-Apr-10: - changed functionality of "scanm_string":
               read  *behind*  trailing '"'
             - scanm_U32 evaluates scanflags
             - introduced new flag SCAN_SKIP_LEADING_WHITES
2013-Apr-06: Added comments
2012-Dec-28: Added "scanm_U32"
2010-Apr-16..17: initial version
*****************************************************************************
Global objects:
- char* scanm_string( char** pp, char* dest, U16 flags )
- U32 scanm_U32( char** pp, U16 flags )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*only for testing purposes*/



/*--  include files  ------------------------------------------------------*/

#include <misc.h>
#include <scan.h>

   /*for isdigit()*/
#include <ctype.h>

#include <stdio.h>
/*#include <stdlib.h>*/
#include <string.h>
#include <falloc.h>
/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/

/*static void usage( void ) ;*/


/*--  macros  -------------------------------------------------------------*/


/*--  global variables  ---------------------------------------------------*/

   /*additional info set by the functions*/
U16 scan_stat ; /*return status*/
U16 scan_len ; /*number of characters scanned*/
U16 scan_string_len ;


/*--  internal variables  -------------------------------------------------*/

#define COMMON_BUF_LEN  100
static char buf [ COMMON_BUF_LEN ] ; /*common buffer if no heap wanted*/


/*-------------------->   scanm_string   <----------------------- 2013-Apr-10
This function scans a string in memory and copies it to "dest".
Initial white characters (blank or tab) are skipped.
If the string begins with a double quote ("), a scan is done behind the
trailing double quote.
If it begins with another character, a scan is done until
a white character or until a '\0'.
If SCAN_NL_TERMINATES is given, the scan stops at a newline, too.
Leading and trailing double quotes (if any) are not copied to "dest".

NOT IMPLEMENTED:
Backslash sequences are treated in 'C style', except when
SCAN_BACKSLASH_VERBATIM is given.
-----------------------------------------------------------------------------
Used functions: falloc, strncpy, fprintf, exit
Globals:   scan_stat, scan_len, scan_string_len
Internals:
Parameters:
- pp pointer to pointer to string in memory ('"' or any other character)
- dest destination for string. If dest == NULL, the string is copied to heap
- flags
  SCAN_EXIT_ON_ERROR -- exit on any error
  SCAN_BACKSLASH_VERBATIM
  SCAN_NL_TERMINATES -- a newline stops the string
  SCAN_ACCEPT_INCOMPLETE -- don't complain about missing " at end
Return value:	pointer to a copy of the scanned string
		The external pointer (where pp points to) is updated
		scan_stat is 0 if no error
Exitcode:	x
---------------------------------------------------------------------------*/
char* scanm_string( char** pp, char* dest, U16 flags )
{
THIS_FUNC(scanm_string)
  char* ret_val ;
  char* p = *pp ;
  char* begin ;
  BIT ongoing = TRUE ; /*while loop control*/


  scan_stat = 0 ; /*default: nothing to report*/
  scan_len = 0 ;
  scan_string_len = 0 ;

  if (flags & SCAN_SKIP_LEADING_WHITES) {
    while ( *p == ' ' || *p == '\t' ) { /*skip initial white characters*/
      p++ ;
      scan_len++ ;
    }
  }

  if (*p == '"') { /*string is enclosed by double quotes*/
    p++ ;
    scan_len++ ;
    begin = p ;
    while (ongoing) {
      switch (*p) {
      case '"':
        p++ ;
        scan_len++ ;
        ongoing = FALSE ;
        break ;

      case '\n':
        ongoing = FALSE ;
        if (flags & SCAN_NL_TERMINATES) { /*newline allowed*/
          break ;
        }
        /*fall thru to case \0*/

      case '\0':
        ongoing = FALSE ;
        scan_stat |= SCANSTAT_INCOMPLETE ;
        if (flags & SCAN_EXIT_ON_ERROR) {
          fprintf( stderr, "scanm_string: missing double quote\n" ) ;
          exit( EXITCODE_SYNTAX_ERR ) ;
        }
        break ;


      /*backslash not yet implemented*/
      /*case '\\':*/
        /*break ;*/

      default:
        scan_len++ ;
        scan_string_len++ ;
        p++ ;
        break ;
      } /*end switch*/
    } /*end while*/
  } /*end: string is enclosed by double quotes*/


  else { /*string is not enclosed by double quotes*/
    begin = p ;
    while (ongoing) {
      switch (*p) {
      case ' ':
      case '\t':
      case '\n':
      case '\0':
        ongoing = FALSE ;
        break ;


      /*backslash not yet implemented*/
      /*case '\\':*/
        /*break ;*/

      default:
        scan_len++ ;
        scan_string_len++ ;
        p++ ;
        break ;
      }
    } /*end while*/
  } /*end: string is not enclosed by double quotes*/


  ret_val = dest ;
  PRT_VAR((unsigned)scan_string_len,u)
  if (dest == NULL) {
    if (flags & SCAN_NO_HEAP) {
      if (scan_string_len >= COMMON_BUF_LEN) { /*buf too short*/
        fprintf( stderr, "scanm_string: buf too short (strlen=%u)\n",
                          (unsigned)scan_string_len ) ;
        exit( EXITCODE_TABLE_FULL ) ;
      }
      ret_val = buf ;
    }
    else {
      ret_val = falloc( scan_string_len +1 ) ;
    }
  }
  if (scan_string_len == 0) {
    scan_stat |= SCANSTAT_NOTHING_SCANNED ;
    if (flags & SCAN_EXIT_ON_ERROR) {
      fprintf( stderr, "scanm_string: no string found\n" ) ;
      exit( EXITCODE_SYNTAX_ERR ) ;
    }
  }
  else {
    strncpy( ret_val, begin, scan_string_len ) ;
  }
  ret_val [ scan_string_len ] = '\0' ;

  /*scan_len =*/
  ASSERT((p - *pp) == scan_len)
  *pp = p ;
  return ret_val ;
}

/*-------------------->   scanm_U32_radix   <-------------------- 2013-Aug-16
This function scans an unsigned number in memory.
Hex numbers must  *not*  start with "0x".  Upper or lower case letters are
allowed.

Problem: Should "011014" be a legal input for a binary scan (result: 13d)
or should an error be signalled (illegal '4') ?
Similarly, should "faw" be a legal input for a hex scan (result: 250d) ?
Currently, these examples are valid!
-----------------------------------------------------------------------------
Used functions:
Globals:   --
Internals: --
Parameters:	- par     pointer to SCAN_PARAMS
Return value:	TRUE if scan failed
Exitcode:	EXITCODE_SYNTAX_ERR
---------------------------------------------------------------------------*/
BIT scanm_U32_radix( SCAN_PARAMS* par )
{
THIS_FUNC(scanm_U32_radix)
  char c ;
  U32 digit_value ; /*U8 would be sufficient, but U32 needs no type cast*/
  U32 radix = (U32)(par->radix) ; /*avoid type casts inside loop*/
  BIT ongoing = TRUE ; /*loop control*/


     /*check parameter range*/
  if (    (radix > 36) /*the A..Z extension would fail*/
       || (radix < 2)) {
    fprintf( stderr, "scanm_U32_radix: radix = %u\n", (unsigned)(radix) ) ;
    exit( EXITCODE_WRONG_PARAM ) ;
  }

  par->scan_len = 0 ;
  par->scan_stat = 0 ; /*default*/

  if (par->flags & SCAN_SKIP_LEADING_WHITES) {
       /*skip initial white characters*/
    while ( *(par->p) == ' ' || *(par->p) == '\t' ) {
      (par->p)++ ;
      scan_len++ ;
    }
  }

  par->token_len = 0 ; /*# digits*/
  par->number = 0 ; /*initial value*/
  while (ongoing) {
    c = *(par->p) ;
    /*PRT_VAR(c,c)*/
    if ((c >= '0') && (c <= '9')) {
      digit_value = c - '0' ;
    }
    else {
      c = (c | 0x20) ; /*upper case -> lower case*/
      if ((c >= 'a') && (c <= 'z')) {
        digit_value = c - 'a' + 10 ;
      }
      else {
        ongoing = FALSE ;
      }
    }
    if (digit_value >= radix) {
      ongoing = FALSE ;
    }

    if (ongoing) {
      par->number = par->radix * par->number + digit_value ;
      (par->p)++ ;
      (par->scan_len)++ ;
      (par->token_len)++ ;
    }
  } /*end digit while loop*/


  if (par->token_len == 0) {
    if (par->flags & SCAN_EXIT_ON_ERROR) {
      fprintf( stderr, "scanm_U32_radix: not a single digit found\n" ) ;
      exit( EXITCODE_SYNTAX_ERR ) ;
    }
    par->scan_stat |= SCANSTAT_NOTHING_SCANNED ;
    return TRUE ; /*nothing scanned*/
  }
  return FALSE ; /*scan ok*/
}

/*-------------------->   scanm_U32   <-------------------------- 2013-Apr-10
This function scans an unsigned integer value.
Fixed radix 10.    NOT  FOT  NEW  DEVELOPMENT !!!  Use "scanm_U32_radix"
Leading zeros are ignored.  The first non-digit character stops the scan.
-----------------------------------------------------------------------------
Used functions:
Globals:
Internals:
Parameters:
- pp pointer to pointer to string in memory (first digit)
- flags
Return value:	the scanned U32 value (zero if not a single digit was found)
                pp is advanced to point to the first character NOT read
                (if pp is unchanged not a single digit was found)
Exitcode:	--
---------------------------------------------------------------------------*/
U32 scanm_U32( char** pp, U16 flags )
{
THIS_FUNC(scanm_U32)
  U32 ret_val = 0 ; /*default*/
  char c ;
  U32 digit_cnt ;


  /*PRT_VAR(*pp,s)*/
  scan_len = 0 ;
  scan_stat = 0 ; /*default*/

  if (flags & SCAN_SKIP_LEADING_WHITES) {
    while ( **pp == ' ' || **pp == '\t' ) { /*skip initial white characters*/
      (*pp)++ ;
      scan_len++ ;
    }
  }

  digit_cnt = 0 ;
  while (isdigit( c = **pp )) {
    /*PRT_VAR(c,c)*/
    ret_val = 10 * ret_val + (c - '0') ;
    /*PRT_VAR((unsigned long)ret_val,lu)*/
    (*pp)++ ;
    scan_len++ ;
    digit_cnt++ ;
  }
  /*PRT_VAR(*pp,s)*/
  /*PRT_VAR((unsigned long)ret_val,lu)*/
  if (digit_cnt == 0) {
    scan_stat |= SCANSTAT_NOTHING_SCANNED ;
    if (flags & SCAN_EXIT_ON_ERROR) {
      fprintf( stderr, "scanm_U32: not a single digit found\n" ) ;
      exit( EXITCODE_SYNTAX_ERR ) ;
    }
  }
  return ret_val ;
}

/*-------------------->   x   <---------------------------------- 2012-Dec-28
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
