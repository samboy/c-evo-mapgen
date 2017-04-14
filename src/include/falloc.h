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
falloc.h

Include file for falloc.c
*****************************************************************************
History: (latest change first)
2013-Jul-06: added "fstrdup"
2012-Aug-16: - function prototype with arg spec (NOT a compiler problem,
               order of include files in client app)
2012-Aug-05: - function prototype w/o  arg spec (ToDo: Compiler problems)
2012-Jul-07: - new filehead.h format
             - function prototype with arg spec
             - added ffree()
1996-Apr-02: last change in old include file
1989-Aug-15: initial version
****************************************************************************/
#ifndef FALLOC_H
#define FALLOC_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

/*#include <types.h>*/


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  typedefs & enums  ---------------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  function prototypes  ------------------------------------------------*/

void* falloc( size_t size ) ;
void  ffree( void* memblock ) ;
char* fstrdup( char* str ) ;


/*--  macros  -------------------------------------------------------------*/

/*--  global variables  ---------------------------------------------------*/

/*extern */


/***************************************************************************/
#endif	/* FALLOC_H */
/***************************************************************************/
