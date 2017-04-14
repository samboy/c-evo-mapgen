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
read_ini.h

Include file for read_ini.c
*****************************************************************************
History: (latest change first)
2013-Feb-28: functions take filename as argument
2013-Feb-27: included "extern.i"
2013-Feb-23: included "read_ini.ini"
2012-Sep-28: added a lot of variables for Scenario Great Plains
2012-Aug-17: added "comp_opponents_area?" and "human_start_pos"
2012-Jun-09: added "water_width"
2010-Jun-09..10: added read_ini_checksum
2010-May-14: Implementation of 'found' flags
2009-Apr-12: extern "mapfile"
2009-Mar-15: Added entries for "Arctic"
2009-Jan-09..17: initial version
****************************************************************************/
#ifndef READ_INI_H
#define READ_INI_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

/*#include <.h>*/


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  typedefs & enums  ---------------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  function prototypes  ------------------------------------------------*/

U32 read_ini_checksum( char* filename, U32 start_value ) ;
BIT read_ini( char* filename ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/

extern char map_type_name [] ;

#include "extern.i"


/***************************************************************************/
#endif	/* READ_INI_H */
/***************************************************************************/
