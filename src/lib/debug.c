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
debug.c

This file contains global variables and functions for debugging purposes.
*****************************************************************************
History: (latest change first)
2011-Apr-26: ifdef for getch/getchar_unlocked
2010-May-14: added: "debug_time_stamp"
2010-Apr-13: added: "debug_pause"
2006-Jul-08: initial version
*****************************************************************************
Global objects:
- BIT debug_trace_flag
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*only for testing purposes*/



/*--  include files  ------------------------------------------------------*/

#include <now.h>
#include <stdio.h>
/*#include <stdlib.h>*/
#ifdef _MS_C_
#  include <conio.h>
#elif __TINYC__
#  include <conio.h>
#elif __GNUC__
//#  include <curses.h>
//extern int getch(void) ;
#else
#  error  debug.c: no compiler specified
#endif


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/

/*static void usage( void ) ;*/


/*--  macros  -------------------------------------------------------------*/


/*--  global variables  ---------------------------------------------------*/

BIT debug_trace_flag = TRUE ;

BIT debug_ignore_pause = FALSE ;
/*U32 debug_pause_cnt = 0 ;*/


/*--  internal variables  -------------------------------------------------*/


/*-------------------->   debug_time_stamp   <------------------- 2010-May-14
This function prints the current date & time to stderr, followed by a
user supplied message.
-----------------------------------------------------------------------------
Used functions: now, fprintf
Globals/Internals:
Parameters:	- msg: user supplied message
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
void debug_time_stamp( char* msg )
{
/*THIS_FUNC(debug_time_stamp)*/
  VERSATILE_TIME_STRUCT vts ;

  now( & vts, NOW_ISO | NOW_NO_NEWLINE ) ;
  fprintf( stderr, "%s: %s\n", vts.output_string, msg ) ;
}

/*-------------------->   debug_pause   <------------------------ 2011-Apr-26
This function is called by the macro "PAUSE".
It wait for user response and continues program execution dependent on what
the user typed.
-----------------------------------------------------------------------------
Used functions: printf, getch
Parameters:	- func_name: the name of the calling function
Return value:	--
Exitcode:	EXITCODE_USER_ABORT
---------------------------------------------------------------------------*/
void debug_pause( char* func_name )
{
/*THIS_FUNC(debug_pause)*/
  /*int key ;*/

  if (debug_ignore_pause) {
    return ;
  }

  printf( "%s: PAUSE (type h for help)\n", func_name ) ;
  while (1) {
#ifdef __unix__
    switch ( getchar_unlocked() ) {
#else
    switch ( getch() ) {
#endif

    case 'h':
    case 'H':
      printf( "t - terminate program\n"
              "r - run till end (ignore future PAUSE points)\n"
              "h - show this help\n"
              "Hit any other key to continue (and stop at next PAUSE point)\n"
            ) ;
      break ;

    case 't':
    case 'T':
      exit( EXITCODE_USER_ABORT ) ;

    case 'r':
    case 'R':
      debug_ignore_pause = TRUE ;

      /*falls through to 'default'*/

    default:
      return ;
    }
  }
}

/*-------------------->   x   <---------------------------------- 2010-May-14
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
