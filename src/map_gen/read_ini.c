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
read_ini.c

This file reads an ini file and puts its content into global variables.
*****************************************************************************
History: (latest change first)
2017-Jan-23: improved output in case of a syntax error
2016-Jul-16: adoptions for gcc
2013-Apr-06: added check for non-numerical values (when num expected)
2013-Feb-27..28: included "globals.i" and "table.i"
2013-Feb-23: included "read_ini.ini"
2013-Feb-14: - implemented check against redefinition of variables
             - all syntax errors are printed to stdout (not stderr)
               so they don't go to the logfile, but to the console
2012-Sep-30: implemented check for unknown identifiers in map_gen.ini
2012-Sep-28: added a lot of variables for scenario Great Plains
2012-Aug-17: added "comp_opponents_area?" and "human_start_pos"
2012-Jun-09: added "water_width"
2012-Jun-06: Clarified error message text
2010-Jun-09..10: added read_ini_checksum
2010-May-14: Introduction of 'found' flags
2010-Apr-17: Make use of new lib func "readsect"
2010-Apr-17: Make use of new lib func "scanm_string"
2010-Jan-24: 'Fixed' bug with long lines (limit 81->400)
2009-Apr-11..12: Error messages with "line_no", Debugging
2009-Mar-15: Added entries for "Arctic"
2009-Jan-09..17: initial version
*****************************************************************************
Global objects:
- all global vars & 'found' flags
- BIT read_ini( void )
- U32 read_ini_checksum( U32 start_value )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/




/*--  include files  ------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <misc.h>
#include <openfile.h>
#include <scan.h>
#include <name_idx.h>
#include <stdarg.h>

#include "map_gen.h"

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

#define MAP_TYPE_SECT "map_type"

/*#define MAXLINE 81*/
#define MAXLINE 400

/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/

/*--  local function prototypes  ------------------------------------------*/

static void set_var_and_value( void ) ;
static char* skip_equalsign( char* p ) ;
static char* skip_whites( char* p ) ;
/*static void syntax_err( char* fmt, char* var_name ) ;*/
static void syntax_err( char* fmt, ... ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/

char map_type_name [MAXLINE] ;
#include "globals.i"


/*--  internal variables  -------------------------------------------------*/

      /*passes the filename from read_ini() to syntax_err()*/
static char* ini_file_name ;

static char zeile [MAXLINE] ;
static char var [MAXLINE] ;
static char value [MAXLINE] ;
static U16 line_no ;

#include "table.i"
static char* type_table [] = { "BIT", "U8", "U16", "char*", NULL } ;


/*-------------------->   read_ini_checksum   <------------------ 2013-Feb-28
This function calculates a checksum (signatur) of the .ini file.
The signature is returned.
CR & LF are ignored to provide compatibility across Unix/Windows.
Ctrl-Z is   ignored to provide compatibility with editors which append a
            Ctrl-Z at the end of the file.
The ini file is closed upon entry and exit.
-----------------------------------------------------------------------------
Used functions:
Globals/Internals:
Parameters:     - filename      the file to read
                - start_value   a 'salt' value
Return value:	a 32-bit signature
Exitcode:	--
---------------------------------------------------------------------------*/
U32 read_ini_checksum( char* filename, U32 start_value )
{
THIS_FUNC(read_ini_checksum)
  U32 ret_val ;
  int c ;
  FILE* fp ;

  fp = forced_fopen_rb( filename ) ;
  ret_val = start_value ;
  while ((c = fgetc( fp )) != EOF) {
    if (    (c != 0xa) /*LF*/
         && (c != 0xd) /*CR*/
         && (c != ('Z' & 0x1f)) /*Ctrl-Z, text file terminator
                                  under CP/M and early DOS*/
       ) {
      ret_val = ret_val * 1103515245 + ((U32)c) ;
    }
  }
  forced_fclose( fp ) ;
  return ret_val ;
}

/*-------------------->   read_ini   <--------------------------- 2016-Jul-16
Function see file header
-----------------------------------------------------------------------------
Used functions: readsect_init, readsect_get_chr
                skip_whites, set_var_and_value,
                strcmp, strcpy,
                printf, exit
Globals:    --
Internals:  ini_file_name w/o,
            ri_table, ri_entries r/o
Parameters:     - filename      the file to read
Return value:	TRUE if ini file error
Exitcode:	EXITCODE_SYNTAX_ERR
---------------------------------------------------------------------------*/
BIT read_ini( char* filename )
{
THIS_FUNC(read_ini)
  char* p ;
  int c ; /*in conformance with C stdlib definition (can be EOF)*/
  S32 ri_table_idx ;
  U32 temp_U32 ;
  BIT ongoing ; /*loop control*/
  BIT map_type_defined = FALSE ;
  BIT do_range_check ; /*check range of value from ini file*/
  BIT do_numerical_check ; /*check if value was numerical*/


  ENTRY
#if defined DEBUG
  for ( ri_table_idx = 0 ; ri_table_idx < ri_entries ; ri_table_idx++ ) {
    ASSERT_ALT(
       ((ri_table [ri_table_idx]).minv <= (ri_table [ri_table_idx]).maxv),
       PRT_VAR((name_tab [ri_table_idx]),s) )
  }
#endif /*DEBUG*/


  ini_file_name = filename ; /*for syntax_err()*/
  if (readsect_init( filename, MAP_TYPE_SECT )) {
    printf( "section \"%s\" in file \"%s\" not found\n",
             MAP_TYPE_SECT, filename ) ;
    exit( EXITCODE_SYNTAX_ERR ) ;
  }

  DEB((stderr, "-----  reading from 'map_type' section  -----\n"))
  line_no = 0 ;
  ongoing = TRUE ;
  while (ongoing) {
    p = zeile ;
    do {
      c = readsect_get_chr() ;
      if (c == EOF) {
        DEB((stderr,"c == EOF\n"))
        ongoing = FALSE ;
      }
      else {
        *p++ = (char)c ;
      }
    } while (c != '\n' && c != EOF) ;
    *p++ = '\n' ;
    *p = '\0' ;
    PRT_VAR(zeile,s)
    PRT_VAR((int)strlen(zeile),d)
    line_no++ ;
    p = skip_whites( zeile ) ;
    if ((*p == '#') || (*p == '\n')) { /*readsect_get_chr filters out all CRs*/
      continue ; /*this is a comment/blank line, proceed to next line*/
    }
    set_var_and_value() ;

    if (strcmp( var, "map_type" ) == 0) {
      if (map_type_defined) { /*more than one definition*/
        printf( "[map_type]: more than one map_type enabled\n" ) ;
        exit( EXITCODE_SYNTAX_ERR ) ;
      }
      strcpy( map_type_name, value ) ;
      PRT_VAR(map_type_name,s)
      map_type_defined = TRUE ;
    }
  }
  if ( ! map_type_defined) {
    printf( "[map_type]: no map_type definition found\n" ) ;
    exit( EXITCODE_SYNTAX_ERR ) ;
  }




  if (readsect_init( filename, map_type_name )) {
    printf( "section [%s] in file \"%s\" not found\n",
             map_type_name, filename ) ;
    exit( EXITCODE_SYNTAX_ERR ) ;
  }

  DEB((stderr, "-----  reading from 2nd section  -----\n"))
  line_no = 0 ;
  ongoing = TRUE ;
  while (ongoing) {
    p = zeile ;
    do {
      c = readsect_get_chr() ;
      if (c == EOF) {
        ongoing = FALSE ;
      }
      else {
        *p++ = (char)c ;
      }
    } while (c != '\n' && c != EOF) ;
       /*one complete line is in buffer "zeile" now*/
    *p++ = '\n' ;
    *p = '\0' ;
    line_no++ ;
    /*PRT_VAR(zeile,s)*/
    p = skip_whites( zeile ) ;
    if ((*p == '#') || (*p == '\n')) { /*readsect_get_chr filters out all CRs*/
      continue ; /*this is a comment/blank line, proceed to next line*/
    }
    set_var_and_value() ;
    PRT_VAR(var,s)

    ri_table_idx = name_idx( var, name_tab, (S32)ri_entries ) ;
    if (ri_table_idx == -1) {
      syntax_err( "Identifier \"%s\" unknown\n", var ) ;
    }

    if (*((ri_table [ri_table_idx]).flag)) {
      syntax_err( "Redefinition of \"%s\"\n", name_tab [ri_table_idx] ) ;
    }
    *((ri_table [ri_table_idx]).flag) = TRUE ;


    /*temp_U32 = atol( value ) ;*/ /*returns 0 for non-numerical strings*/
    p = value ;
    temp_U32 = scanm_U32(  & p, SCAN_NO_FLAGS ) ;

    do_range_check = TRUE ; /*default*/
    do_numerical_check = TRUE ; /*default*/
    switch (name_idx( (ri_table [ri_table_idx]).type, type_table,
                      NAME_IDX_AUTO )) {
    case 0: /*BIT*/
      *((BIT*)((ri_table [ri_table_idx]).var)) = (BIT)temp_U32 ;
      break ;

    case 1: /*U8*/
      *((U8*)((ri_table [ri_table_idx]).var)) = (U8)temp_U32 ;
      break ;

    case 2: /*U16*/
      *((U16*)((ri_table [ri_table_idx]).var)) = (U16)temp_U32 ;
      break ;

    case 3: /*char* */
      *((char**)((ri_table [ri_table_idx]).var)) = strdup( value ) ;
      do_range_check = FALSE ;
      do_numerical_check = FALSE ;
      break ;

    case -1: /*not in type_table*/
    default:
      fprintf( stderr,
               "read_ini(): unexpected error: type %s not in type table\n",
               (ri_table [ri_table_idx]).type ) ;
      exit( EXITCODE_UNEXPECTED_ERR ) ;
    } /*end switch*/

    if (do_numerical_check) {
      if (p == value) {
        syntax_err( "Expected numerical value, found:\n%s\n", zeile ) ;
      }
    }
    if (do_range_check) {
      if (   (temp_U32 < (ri_table [ri_table_idx]).minv)
          || (temp_U32 > (ri_table [ri_table_idx]).maxv)
         ) {
        syntax_err( "\"%s\" must be between %lu and %lu\n", var,
                    (unsigned long)(ri_table [ri_table_idx]).minv,
                    (unsigned long)(ri_table [ri_table_idx]).maxv
                  ) ;
      }
    }
  } /*end while*/
  return FALSE ;
}

/*-------------------->   skip_equalsign   <--------------------- 2017-Jan-23
This function skips optional white characters, a mandatory equal sign,
and more optional white characters.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- p  points to the first optional white or to the '='
Return value:	char pointer behind last optional white
Exitcode:	EXITCODE_SYNTAX_ERR
---------------------------------------------------------------------------*/
static char* skip_equalsign( char* p )
{
THIS_FUNC(skip_equalsign)
  /*ENTRY*/
  /*PRT_VAR(p,s)*/
  p = skip_whites( p ) ;
  if (*p == '=') { /*ok*/
    p++ ;
  }
  else {
    printf( "Line is: \"%s\"\n", zeile ) ;
    syntax_err( "equal sign missing after \"%s\"\n", var ) ;
  }
  p = skip_whites( p ) ;
  /*EXIT*/
  return p ;
}

/*-------------------->   set_var_and_value   <------------------ 2010-Apr-17
This function scans "zeile" and puts copies into "var" and "value" of the
respective parts of "zeile".
The line must not be empty and must not consist of white characters.
-----------------------------------------------------------------------------
Used functions: scanm_string
Globals:
Internals:  var, value
Parameters:	--
Return value:	--
Exitcode:	SCAN_EXIT_ON_ERROR => EXITCODE_xxx
---------------------------------------------------------------------------*/
static void set_var_and_value( void )
{
THIS_FUNC(set_var_and_value)
  char* p = zeile ;
  char* target ;

  ENTRY
  PRT_VAR(p,s)
  *var = '\0' ;
  *value = '\0' ;
  target = var ;
  while( *p != ' ' && *p != '\t' && *p != '=' ) {
    if (*p == '\0') {
      fprintf( stderr, "map_gen::read_ini::set_var_and_value: empty line -- aborting\n" ) ;
      exit( 1 ) ;
    }
    *target++ = *p++ ;
  }
  *target = '\0' ;
  /*PRT_VAR(var,s)*/

  p = skip_equalsign( p ) ;

  if (*p == '"') { /*string*/
    scanm_string( & p, value, SCAN_EXIT_ON_ERROR ) ;
    PRT_VAR(value,s)
  }
  else { /*no string*/
    target = value ;
    while( *p != ' ' && *p != '\t' && *p != '#' && *p != '\n' && *p != '\0' ) {
      *target++ = *p++ ;
    }
    *target = '\0' ;
  }
  PRT_VAR(value,s)
  EXIT
}

/*-------------------->   skip_whites   <------------------------ 2009-Apr-12
This function skips white characters (= blanks and tabs).
Newlines do not count as whites here.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- p   pointer to string
Return value:	pointer to string
Exitcode:	--
---------------------------------------------------------------------------*/
static char* skip_whites( char* p )
{
THIS_FUNC(skip_whites)
  while( *p == ' ' || *p == '\t' ) {
    p++ ;
  }
  return p ;
}

/*-------------------->   syntax_err   <------------------------- 2016-Jul-16
This function displays an error message (to stdout), then exits.
-----------------------------------------------------------------------------
Used functions: printf, vprintf, exit
Globals/Internals: ini_file_name, map_type_name r/o
Parameters:	- fmt  contains the error message.  It serves as a format
                       string for vprintf
		- ...  params for vprintf
Return value:	void (does not return)
Exitcode:	EXITCODE_SYNTAX_ERR
---------------------------------------------------------------------------*/
static void syntax_err( char* fmt, ... )
{
THIS_FUNC(syntax_err)
#ifdef unix
  va_list ap ;
#endif


  printf( "\"%s\": section [%s]: ", ini_file_name, map_type_name ) ;
#ifdef unix
  va_start( ap, fmt ) ;
  vprintf( fmt, ap ) ;
  va_end( ap ) ;
#else
  vprintf( fmt, (va_list)( (& fmt) +1 ) ) ;
#endif
  exit( EXITCODE_SYNTAX_ERR ) ;
}

/*-------------------->   x   <---------------------------------- 2013-Feb-28
This function x
-----------------------------------------------------------------------------
Used functions:
Globals:
Internals:
Parameters:     - x
                - x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
/*x*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/
/***************************************************************************/
