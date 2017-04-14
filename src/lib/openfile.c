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
*	openfile.c
* Diese Datei enthaelt Subroutinen zum Oeffnen / Schliessen von Files.
*****************************************************************************
Aenderungen:
2016-Sep-13: do not check for TEXT/BIN under Unix
#2016-Mar-31: no open_wt under unix
2014-Jun-20: implemented flag ~_APPEND
2011-Aug-11: changed "forced_fopen_w", introducing flags
             !!! old programs using this func need rework !!!
2011-Apr-26: Changes for Tiny-C (io.h for access() )
2011-Feb-24: included "compiler.h"
2003-Nov-02: bei Fehlermeldungen Double-Quotes um Dateinamen
2002-Jul-09: neue Funktionen: _rb, _wt, _wb
2001-03-28: Ersterstellung
*****************************************************************************
Globale Objekte:
- FILE* forced_fopen_rt( char* filename )
- FILE* forced_fopen_r( char* filename )
- FILE* forced_fopen_rb( char* filename )
- FILE* forced_fopen_w( char* filename, U8 flags )
- FILE* forced_fopen_wt( char* filename )    not under Unix!
- FILE* forced_fopen_wb( char* filename )
- void  forced_fclose( FILE* fp )
****************************************************************************/

/***  Include files  *******************************************************/

/*#include <compiler.h>*/
#include <misc.h>
#include <stdio.h>
/*#include <stdlib.h>*/
#include <file_sys.h>
#include <openfile.h>

#ifdef __TINYC__
   /*for access()*/
# include <io.h>
#endif

#ifdef __unix__
     /* for access() */
#  include <unistd.h>
#endif

#include <exitcode.h>
/*#include <getopts.h>*/
/*#define DEBUG*/
#include <debug.h>

/***  Constants  ***********************************************************/

   /*for the 't' versions*/
#ifdef __unix__
# define   WT   "w"
# define   RT   "r"
#else
# define   WT   "wt"
# define   RT   "rt"
#endif


/***  Type declarations ****************************************************/
/*typedef struct {*/
/*} NEW_TYPE ;*/

/***  Function prototypes  *************************************************/

/***  Makros  **************************************************************/

/***  Global variables  ****************************************************/

/***  Internal variables  **************************************************/

/*-------------------->   forced_fopen_rt   <-------------------- 2016-Mar-31
Diese Funktion x
-----------------------------------------------------------------------------
benutzte Funktionen:
Parameter:	- x
		- x
glob.var. r/o	x
glob.var. w/o	x
glob.var. r/w	x
Returnvalue:	x
Exitcode(s):	x
---------------------------------------------------------------------------*/
FILE* forced_fopen_rt( char* filename )
{
THIS_FUNC(forced_fopen_rt)
   FILE* ret_val ;
#define EXIST 0

   if ( access( filename, EXIST )) {
      fprintf( stderr, "cannot open \"%s\": doesn't exist\n", filename ) ;
      exit( EXITCODE_FILE_NOT_FOUND ) ;
   }
   if ((ret_val = fopen( filename, RT )) == NULL) {
      fprintf( stderr, "cannot open \"%s\": permission denied\n", filename ) ;
      exit( EXITCODE_PERM_DENIED ) ;
   }
   return ret_val ;
}
/*-------------------->   forced_fopen_r   <--------------------- 2003-Nov-02*/
/*-------------------->   forced_fopen_rb   <-------------------- 2003-Nov-02
Diese Funktion x
-----------------------------------------------------------------------------
benutzte Funktionen:
Parameter:	- x
		- x
glob.var. r/o	x
glob.var. w/o	x
glob.var. r/w	x
Returnvalue:	x
Exitcode(s):	x
---------------------------------------------------------------------------*/
FILE* forced_fopen_r( char* filename )
{
THIS_FUNC(forced_fopen_r)
   FILE* ret_val ;
#define EXIST 0

   if ( access( filename, EXIST )) {
      fprintf( stderr, "cannot open \"%s\": doesn't exist\n", filename ) ;
      exit( EXITCODE_FILE_NOT_FOUND ) ;
   }
   if ((ret_val = fopen( filename, "rb" )) == NULL) {
      fprintf( stderr, "cannot open \"%s\": permission denied\n", filename ) ;
      exit( EXITCODE_PERM_DENIED ) ;
   }
   return ret_val ;
}

FILE* forced_fopen_rb( char* filename )
{
THIS_FUNC(forced_fopen_rb)
   FILE* ret_val ;
#define EXIST 0

   if ( access( filename, EXIST )) {
      fprintf( stderr, "cannot open \"%s\": doesn't exist\n", filename ) ;
      exit( EXITCODE_FILE_NOT_FOUND ) ;
   }
   if ((ret_val = fopen( filename, "rb" )) == NULL) {
      fprintf( stderr, "cannot open \"%s\": permission denied\n", filename ) ;
      exit( EXITCODE_PERM_DENIED ) ;
   }
   return ret_val ;
}

/*-------------------->   forced_fopen_w   <--------------------- 2016-Sep-13
This function opens a file for writing.
Tests are done according to 'flags' parameter.
"File_sys" is used instead of "access" to be more independent of file system.
-----------------------------------------------------------------------------
Used functions: fopen, file_sys_get_fileinfo, fprintf, exit
Globals/Internals: --
Parameters:	- filename     file to open
		- flags
Return value:	pointer to opened FILE
Exitcode:	WRONG_PARAM, FILE_EXISTS, PERM_DENIED
---------------------------------------------------------------------------*/
FILE* forced_fopen_w( char* filename, U8 flags )
{
THIS_FUNC(forced_fopen_w)
  FILE_SYS_FILEINFO* fileinfo ;
  FILE* ret_val ;
  //char* mode = "w" ; /*default*/
  char mode [3] ;

   /*don't care under Unix*/
#ifndef __unix__
#define TEMP  (OPENFILE_TEXT | OPENFILE_BIN)
  if ((flags & TEMP) == TEMP) {
    fprintf( stderr, "forced_fopen_w: illegal flag combination (text, bin)\n" ) ;
    exit( EXITCODE_WRONG_PARAM ) ;
  }
#undef TEMP
#endif

#define TEMP  (OPENFILE_DONT_OVERWRITE | OPENFILE_APPEND)
  if ((flags & TEMP) == TEMP) {
    fprintf( stderr, "forced_fopen_w: illegal flag combination (r/o, append)\n" ) ;
    exit( EXITCODE_WRONG_PARAM ) ;
  }
#undef TEMP

  mode [0] = 'w' ; /*default*/
  mode [1] = '\0' ;
  mode [2] = '\0' ;
  if (flags & OPENFILE_TEXT) {
    /*mode = "wt" ;*/
    mode [1] = 't' ;
  }
  if (flags & OPENFILE_BIN) {
    /*mode = "wb" ;*/
    mode [1] = 'b' ;
  }
  if (flags & OPENFILE_APPEND) {
    /**mode = 'a' ;*/
    mode [0] = 'a' ;
  }
  if (flags & OPENFILE_DONT_OVERWRITE) {
    fileinfo = file_sys_get_fileinfo( filename, NULL, FILE_SYS_NULL_NAME ) ;
    if (fileinfo != NULL) {
      fprintf( stderr, "cannot open \"%s\": already exists\n", filename ) ;
      exit( EXITCODE_FILE_EXISTS ) ;
    }
  }

  if ((ret_val = fopen( filename, mode )) == NULL) {
    fprintf( stderr, "cannot open \"%s\": permission denied\n", filename ) ;
    exit( EXITCODE_PERM_DENIED ) ;
  }
  return ret_val ;
}

/*-------------------->   forced_fopen_wt   <-------------------- 2016-Mar-31*/
/*-------------------->   forced_fopen_wb   <-------------------- 2003-Nov-02
Diese Funktion x
-----------------------------------------------------------------------------
benutzte Funktionen:
Parameter:	- x
		- x
glob.var. r/o	x
glob.var. w/o	x
glob.var. r/w	x
Returnvalue:	x
Exitcode(s):	x
---------------------------------------------------------------------------*/

   /*There is no reason to use forced_open_wt() under Unix.*/
   /*In fact, the 't' option is not even defined under Unix!*/
   /*BUT: Code has to compile under both DOS ans Unix, so the programs*/
   /*will always contain the wt function.  Unix must deal with it!*/
FILE* forced_fopen_wt( char* filename )
{
THIS_FUNC(forced_fopen_wt)
   FILE* ret_val ;

   if ((ret_val = fopen( filename, WT )) == NULL) {
      fprintf( stderr, "cannot open \"%s\": permission denied\n", filename ) ;
      exit( EXITCODE_PERM_DENIED ) ;
   }
   return ret_val ;
}

FILE* forced_fopen_wb( char* filename )
{
THIS_FUNC(forced_fopen_wb)
   FILE* ret_val ;

   if ((ret_val = fopen( filename, "wb" )) == NULL) {
      fprintf( stderr, "cannot open \"%s\": permission denied\n", filename ) ;
      exit( EXITCODE_PERM_DENIED ) ;
   }
   return ret_val ;
}

/*-------------------->   forced_fclose   <----------------------- 2001-03-28
Diese Funktion x
-----------------------------------------------------------------------------
benutzte Funktionen:
Parameter:	- x
		- x
glob.var. r/o	x
glob.var. w/o	x
glob.var. r/w	x
Returnvalue:	x
Exitcode(s):	x
---------------------------------------------------------------------------*/
void forced_fclose( FILE* fp )
{
THIS_FUNC(forced_fclose)
   if (fclose( fp ) != 0) {
      /*fprintf( stderr, "cannot close %s: disk full?\n", filename ) ;*/
      fprintf( stderr, "cannot close: disk full?\n" ) ;
      exit( EXITCODE_DISK_FULL ) ;
   }
   return ;
}

/*-------------------->   x   <---------------------------------- 2011-Aug-11
This function x
-----------------------------------------------------------------------------
Used functions: x
Globals/Internals: x
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
