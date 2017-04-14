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
name_idx.c

This library contains a function which helps in string table look-up.
*****************************************************************************
History: (latest change first)
2013-Feb-27: initial version
*****************************************************************************
Global objects:
- S32 name_idx( char* name, char** name_tab, S32 no_of_entries )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/




/*--  include files  ------------------------------------------------------*/

#include <name_idx.h>
#include <string.h>
/*#include <stdio.h>*/
/*#include <stdlib.h>*/
/*#include <getopts.h>*/

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


/*--  internal variables  -------------------------------------------------*/



/*--  library functions  --------------------------------------------------*/

/*-------------------->   name_idx   <--------------------------- 2013-Feb-27
This function looks up an entry in a table of strings and returns the index.
-----------------------------------------------------------------------------
Used functions: strcmp
Globals:   --
Internals: --
Parameters:     - name             ... to search
                - name_tab         pointer to an array of pointers to
                                   '\0' terminated strings
                - no_of_entries    tells the array size.  Three special values
  instruct this func to determine the table end on its own hands:
  NAME_IDX_NULL   tells the last entry is a NULL pointer
  NAME_IDX_EMPTY  tells the last entry is a "" string
  NAME_IDX_AUTO   checks both of the above conditions
Return value:   index of entry or -1 if not found
Exitcode:       --
---------------------------------------------------------------------------*/
S32 name_idx( char* name, char** name_tab, S32 no_of_entries )
{
THIS_FUNC(name_idx)
  S32 ret_val ;
  BIT check_null  = FALSE ; /*default*/
  BIT check_empty = FALSE ; /*default*/


  if (no_of_entries == NAME_IDX_NULL) {
    check_null = TRUE ;
  }
  if (no_of_entries == NAME_IDX_EMPTY) {
    check_empty = TRUE ;
  }
  if (no_of_entries == NAME_IDX_AUTO) {
    check_null = TRUE ;
    check_empty = TRUE ;
  }

  if (check_null || check_empty) { /*previously unknown array size*/
    ret_val = 0 ;
    while (TRUE) {
      if (check_null) {
        if (name_tab [ ret_val ] == NULL) {
          return -1 ;
        }
      }
      if (check_empty) {
        if (*(name_tab [ ret_val ]) == '\0') { /*empty string*/
          return -1 ;
        }
      }
      if (strcmp( name, name_tab [ ret_val ] ) == 0) {
        return ret_val ;
      }
      ret_val++ ;
    }
  }
  else { /*previously known array size*/
    for ( ret_val = 0 ; ret_val < no_of_entries ; ret_val++ ) {
      if (strcmp( name, name_tab [ ret_val ] ) == 0) {
        return ret_val ;
      }
    }
    return -1 ;
  }
}

/*-------------------->   x   <---------------------------------- 2013-Feb-27
This function x
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters:     - x
                - x
Return value:   x
Exitcode:       x
---------------------------------------------------------------------------*/
/*x()*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/
/***************************************************************************/
