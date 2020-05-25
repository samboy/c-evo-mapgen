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
random.h

Include file for random.c
*****************************************************************************
History: (latest change first)
2017-Mar-22: added RANDOM_MAX_RAND
2010-May-02: initial version
****************************************************************************/
#ifndef RANDOM_H
#define RANDOM_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

#include <misc.h>


/*--  constants  ----------------------------------------------------------*/

#define  RANDOM_MAX_RAND 0x7fff


/*--  typedefs & enums  ---------------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  function prototypes  ------------------------------------------------*/

uint64_t random_init( BIT use_seed, uint64_t seed, uint64_t seed2 ) ;
U16 random_draw( void ) ; /*returns integer [0..RANDOM_MAX_RAND]*/
U16 random_draw_range( U16 min, U16 max ) ;


/*--  macros  -------------------------------------------------------------*/

/*--  global variables  ---------------------------------------------------*/

/*extern */


/***************************************************************************/
#endif	/* RANDOM_H */
/***************************************************************************/
