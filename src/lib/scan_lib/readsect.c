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
readsect.c

This library reads a section in a file (.ini or .inf format).
Sections start with a line with [section_name].
Sections end when another section begins or at EOF.
*****************************************************************************
History: (latest change first)
2016-Jul-16: fixed Unix/DOS issue (LF <-> CR/LF)
2010-Apr-17: initial version
*****************************************************************************
Global objects:
- 
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/



/*--  include files  ------------------------------------------------------*/

#include <stdio.h>
/*#include <stdlib.h>*/

#include <misc.h>
#include <scan.h>

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

#if unix
# define OPEN_MODE "r"
#else /*DOS/Windows*/
# define OPEN_MODE "rt"
#endif


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/

/*static void usage( void ) ;*/


/*--  macros  -------------------------------------------------------------*/


/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/

static BIT in_use = FALSE ; /*protection against recursive calls*/
static FILE* fp ;
static int c ; /*next char from file*/

static U8 state ; /*state machine*/
#define BEGIN_OF_LINE              0
#define IN_LINE                    1
#define SECTION_TEST               2
#define SECTION_FOUND              3
#define BEGIN_OF_DESIRED_SECTION   4


/*-------------------->   readsect_init   <---------------------- 2016-Jul-16
This function opens the file to scan and 'fast forwards' to the desired
section.
-----------------------------------------------------------------------------
Used functions: fprintf, exit, fopen, fclose, fgetc
Parameters:	- filename: file to scan
		- sectname: name of section (Sectname may be NULL.  The lines
		            before the first section are selected then)
Return value:	TRUE if failed, FALSE if ok
Exitcode:	EXITCODE_WRONG_CALL
---------------------------------------------------------------------------*/
BIT readsect_init( char* filename, char* sectname )
{
THIS_FUNC(readsect_init)
  char* cmp ;

  if (in_use) {
    fprintf( stderr, "readsect_init: already in use\n" ) ;
    exit( EXITCODE_WRONG_CALL ) ;
  }

  if ((fp = fopen( filename, OPEN_MODE )) == NULL) {
    return TRUE ; /*file not found*/
  }
  state = BEGIN_OF_LINE ;

  if (sectname != NULL) { /*fast forward to desired section*/
    while (state != BEGIN_OF_DESIRED_SECTION) {
      c = fgetc( fp ) ;
      if (c == EOF) { /*premature EOF*/
        fclose( fp ) ;
        return TRUE ; /*section not found*/
      }

      switch (state) {

      case BEGIN_OF_LINE:
        if (c == '[') { /*new section begins*/
          state = SECTION_TEST ;
          cmp = sectname ;
        }
        else if (c != '\n') {
          state = IN_LINE ;
        }
        break ;

      case IN_LINE:
        if (c == '\n') {
          state = BEGIN_OF_LINE ;
        }
        break ;

      case SECTION_TEST:
        if (c == ']') {
          if (*cmp == '\0') {
            state = SECTION_FOUND ; /*read to end of this line*/
          }
          else {
            state = IN_LINE ;
          }
        }
        else if (c != *cmp++) { /*this is a stepwise strcmp*/
          state = IN_LINE ;
        }
        break ;

      case SECTION_FOUND: /*read to end of this line*/
        if (c == '\n') {
          state = BEGIN_OF_DESIRED_SECTION ;
        }
        break ;

      default:
        fprintf( stderr, "readsect_init: unexpected state %u\n",
                 (unsigned)state ) ;
        exit( EXITCODE_UNEXPECTED_ERR ) ;
      } /*end switch*/
    } /*end while*/

  } /*end sectname != NULL*/

  state = BEGIN_OF_LINE ; /*for "readsect_get_chr"*/
  in_use = TRUE ;
  return FALSE ; /*everything ok so far*/
}

/*-------------------->   readsect_get_chr   <------------------- 2016-Jul-16
This function returns the next character of the desired section.
If the section is completed, EOF is returned and the input file is closed
automatically.
-----------------------------------------------------------------------------
Used functions: fgetc, fclose, fprintf, exit
Parameters:	--
Return value:	next char from section or EOF
Exitcode:	EXITCODE_WRONG_CALL
---------------------------------------------------------------------------*/
int readsect_get_chr( void )
{
THIS_FUNC(readsect_get_chr)

  if ( ! in_use) {
    fprintf( stderr, "readsect_get_chr: not initialized\n" ) ;
    exit( EXITCODE_WRONG_CALL ) ;
  }

#ifdef unix
  do {          /*skip all CRs*/
    c = fgetc( fp ) ;
  } while (c == '\r') ;
#else /*DOS/Windows*/
  c = fgetc( fp ) ;
#endif

  if (    c == EOF                                /*end of file*/
       || (c == '[' && state == BEGIN_OF_LINE)    /*begin of next section*/
     ) {
    if (fclose( fp ) != 0) {
      fprintf( stderr, "readsect_get_chr: cannot close input file\n" ) ;
      exit( EXITCODE_CANNOT_CLOSE ) ;
    }
    in_use = FALSE ;
    return EOF ;
  }

  state = (c == '\n') ? BEGIN_OF_LINE : IN_LINE ;

  return c ;
}

/*-------------------->   x   <---------------------------------- 2010-Apr-17
This function x
-----------------------------------------------------------------------------
Used functions:
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
