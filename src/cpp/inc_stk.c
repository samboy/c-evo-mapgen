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
inc_stk.c

This file implements an include file stack.
It is meant for use with the C preprocessor.
*****************************************************************************
History: (latest change first)
2016-Apr-01: omit CR before LF for unix version
2016-Mar-29: changed array index from 'S8' to 'int' (for gcc)
2013-Feb-26: Debugging
2012-Nov-14: changed MAX_PATH to FILE_SYS_MAX_PATH
2003-Aug-14..2004-Apr-09: initial version
*****************************************************************************
Global objects:
- void inc_stk_init( char* option_I )
- void inc_stk_open( char* filename )
-  int inc_stk_get_chr( void )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*only for testing purposes*/



/*--  include files  ------------------------------------------------------*/

#include <misc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <file_sys.h>
#include <openfile.h>
/*#include <exitcode.h>*/

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

#define MAX_NESTING  9
#define INC_ENV_VAR  "INCLUDE"


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/

static void append_path_list( char* list ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/



/*--  internal variables  -------------------------------------------------*/

static char search_list [ 300 ] ;  /*include search path list*/
   /*semicolon-separated, even under unix !!*/

static FILE* infile ; /*currently effective input file, switched by the
                        stack mechanism*/

static FILE* handle_tab [ MAX_NESTING +1 ] ;
static char*   name_tab [ MAX_NESTING +1 ] ;

   /*S8 would work, but gcc complains about it*/
/*static S8 file_index ;*/ /*points to the next free entry*/
static int file_index ; /*points to the next free entry*/
/*
   file_index = -1  -> no file open
   file_index = 0  -> 'main' file open (entry '0' holds main file info)
   file_index = 1  -> 1st include file open
   etc.
*/


/*-------------------->   inc_stk_init   <-------------------- 2003-Nov-07 --
This function x
-----------------------------------------------------------------------------
Used functions:
Internals: file_index
Parameters:	- option_I   is a pointer to the -I option
                             (=include search path list)
                             NULL or empty string if option was not used
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
void inc_stk_init( char* option_I )
{
THIS_FUNC(inc_stk_init)

   /*fprintf( stderr, "inc_stk_init: option_I=%s\n", option_I ) ;*/
   /*fprintf( stderr, "inc_stk_init: %s=%s\n", INC_ENV_VAR, getenv( INC_ENV_VAR ) ) ;*/

   file_index = -1 ; /*no file open*/

   search_list [ 0 ] = '\0' ; /*default: no search list*/
   append_path_list( option_I ) ;
   append_path_list( getenv( INC_ENV_VAR ) ) ;
   /*fprintf( stderr, "inc_stk_init: search_list=%s\n", search_list ) ;*/
}

/*-------------------->   append_path_list   <---------------- 2003-Nov-07 --
This function appends parameter 'list' to internal static var 'search_list'.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
static void append_path_list( char* list )
{
THIS_FUNC(append_path_list)
   char* temp ;
   U16 len ;

   if (list != NULL) { /*copy list to search_list*/
      /*fprintf( stderr, "append_path_list: list=%s\n", list ) ;*/
      while (*list) {
         if (strlen( search_list ) > 0) {
            strcat( search_list, ";" ) ;
         }
         temp = strchr( list, ';' ) ; /*multi-path spec?*/
         if (temp == NULL) { /*single path spec
                               or last item in multi-path spec*/
            strcat( search_list, list ) ;
            list = "" ; /*finish*/
         }
         else { /*multi-path spec*/
            *temp = '\0' ; /*snip!*/
            strcat( search_list, list ) ;
            list = temp +1 ; /*next item of multi-path*/
         }

         len = strlen( search_list ) ;
         temp = & (search_list [ len -1 ]) ; /*point to last char in string*/
         if (*temp != PATH_SEPARATOR) {
            *++temp = PATH_SEPARATOR ;
            *++temp = '\0' ;
         }
      }
   }
}

/*-------------------->   inc_stk_open   <----------------------- 2013-Feb-26
This function opens the file "filename" and includes its content into
the input stream.
-----------------------------------------------------------------------------
Used functions:
Internals: file_index, name_tab, handle_tab, search_list
Parameters:	- filename      "name"  resp.  <name>   resp.  name
Return value:	void
Exitcode:	EXIT_CODE_TABLE_FULL
---------------------------------------------------------------------------*/
void inc_stk_open( char* filename )
{
THIS_FUNC(inc_stk_open)
   char* resolved_path ;
   char c ;


   ASSERT(filename != NULL)
   PRT_VAR(filename,s)

   if (file_index >= MAX_NESTING) {
      fprintf( stderr, "Too many include levels\n" ) ;
      exit( EXITCODE_TABLE_FULL ) ;
   }


   DEB(( stderr, "searching for *%s*\n", filename ))
   c = *filename ; /*'\"' or '<'*/


      /*remove the double quotes resp. the angle brackets*/
   switch (c) {
   case '<':
   case '\"':
      filename++ ;
      filename [ strlen( filename ) -1 ] = '\0' ;
      break ;
   default:
      ;
   }
   PRT_VAR(filename,s)


   if (file_sys_is_abs_path( filename )) {
      resolved_path = filename ;
   }
   else { /*not an absolute path*/
      PRT_VAR(c,c)
      switch (c) {
      case '\"':
            /*1. search in "."*/
         /*if (file_sys_file_exists( filename )) {*/ /*}*/
         if (file_sys_file_is_plain( filename )) {
            resolved_path = filename ;
            break ;
         }
         /*else if () {*/ /*search in '.' dirs of previous inc files*/
         /*}*/

         /*fall thru to next case*/
      case '<': /*search in search_list*/
         PRT_VAR(search_list,s)
         resolved_path = file_sys_search_plain( filename, search_list ) ;
         break ;

      default:
         if (file_index == -1) {
                  /*neither '<' nor '\"' is required for first file*/
               /*search in "."*/
            if (file_sys_file_is_plain( filename )) {
               resolved_path = filename ;
               break ;
            }
         }
         fprintf( stderr,
            "inc_stk_open has been called with invalid parameter\n" ) ;
         exit( EXITCODE_WRONG_PARAM ) ;
      } /*end switch*/
   } /*endif: not an absolute path*/


   if (resolved_path == NULL) {
     fprintf( stderr, "cpp: file xxx, line yyy: cannot find include file %s\n",
                      filename ) ;
     fprintf( stderr, "search_list=%s\n", search_list ) ;
     exit( EXITCODE_FILE_NOT_FOUND ) ;
   }

   DEB(( stderr, "now opening *%s*\n", resolved_path ))
   infile = forced_fopen_rt( resolved_path ) ;

   file_index++ ;
   name_tab   [ file_index ] = strdup( filename ) ;
   handle_tab [ file_index ] = infile ;
}

/*-------------------->   inc_stk_get_chr   <-------------------- 2016-Apr-01
This function returns the next character from input file(s).
Input files are closed automatically when EOF is encountered.

The function calls itself  =>  REENTRANT !!!
-----------------------------------------------------------------------------
Used functions:
Globals:
Internals: infile, file_index, name_tab, handle_tab
Parameters:     --
Return value:	next character or EOF
Exitcode:	x
---------------------------------------------------------------------------*/
int inc_stk_get_chr( void )
{
THIS_FUNC(inc_stk_get_chr)
   int ret_val ;


   /*ENTRY*/
   ASSERT(file_index >= 0)

      /*ALL characters read from any input file come thru this statement*/
   ret_val = getc( infile ) ;

   if (ret_val == EOF) { /*close file and pop old FILE*/
      DEB(( stderr, "closing file \"%s\"\n", name_tab [ file_index ] ))
      ASSERT(infile != NULL)
      forced_fclose( infile ) ;

      file_index-- ;
      if (file_index < 0) {
         DEB(( stderr, "no more files on stack\n" ))
         /*EXIT*/
         return EOF ;
      }
      else {
         infile = handle_tab [ file_index ] ; /*pop old FILE from stack*/
         DEB(( stderr, "continuing to process file \"%s\"\n",
            name_tab [ file_index ] ))
         /*EXIT*/
         return inc_stk_get_chr() ; /*recursive call*/
      }
   }
   else { /*not EOF*/
#ifdef __unix__
        /*handle DOS files correctly*/
     if (ret_val == 0x0d) { /*omit this extra Ctrl-M (CR)*/
       ret_val = getc( infile ) ;
       if (ret_val != '\n') {
         fprintf( stderr, "inc_stk_get_chr: found CR w/o LF -- aborting\n" ) ;
         exit( EXITCODE_SYNTAX_ERR ) ;
       }
     }
#endif
      /*EXIT*/
      return ret_val ;
   }
}

/*-------------------->   x   <----------------------------------- 2016-Apr-01
This function x
------------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters: - x
            - x
Return value: x
Exitcode:     x
----------------------------------------------------------------------------*/
/*x()*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/
/****************************************************************************/
