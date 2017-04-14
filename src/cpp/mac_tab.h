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
mac_tab.h

Include file for mac_tab.c
*****************************************************************************
History: (latest change first)
2013-Feb-26: included <misc.h> for BIT
2003-Aug-14..24: initial version
****************************************************************************/
#ifndef MAC_TAB_H
#define MAC_TAB_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

#include <misc.h>


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  typedefs & enums  ---------------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  function prototypes  ------------------------------------------------*/

void mac_tab_init( void ) ;
void mac_tab_add( char* name, char** arg_list, char* body ) ;
void mac_tab_del( char* name ) ;
BIT  mac_tab_is_defined( char* name ) ;
void mac_tab_expand( char* name, char** arg_list ) ;
int  mac_tab_get_chr( void ) ;
void mac_tab_put_back_chr( int chr ) ;

void mac_tab_print( void ) ;


/*--  macros  -------------------------------------------------------------*/

/*--  global variables  ---------------------------------------------------*/

/*extern */


/***************************************************************************/
#endif	/* MAC_TAB_H */
/***************************************************************************/
