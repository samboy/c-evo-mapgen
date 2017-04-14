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
inc_stk.h

Include file for inc_stk.c
*****************************************************************************
History: (latest change first)
2003-Aug-14..Nov-07: initial version
****************************************************************************/
#ifndef INC_STK_H
#define INC_STK_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

/*#include <.h>*/


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  typedefs & enums  ---------------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  function prototypes  ------------------------------------------------*/

void inc_stk_init( char* option_I ) ;
void inc_stk_open( char* filename ) ; /*opens a new level*/
/*void inc_stk_close( void ) ;*/
int  inc_stk_get_chr( void ) ;


/*--  macros  -------------------------------------------------------------*/

/*--  global variables  ---------------------------------------------------*/

/*extern */


/***************************************************************************/
#endif	/* INC_STK_H */
/***************************************************************************/
