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
name_idx.h

Include file for name_idx.c
*****************************************************************************
History: (latest change first)
2013-Feb-27: initial version
****************************************************************************/
#ifndef NAME_IDX_H
#define NAME_IDX_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

#include <misc.h> /*for S32*/


/*--  constants  ----------------------------------------------------------*/

   /*flags for name_idx()*/
   /*tells the last entry is a NULL pointer*/
#define NAME_IDX_NULL   -1

  /*tells the last entry is a "" string*/
#define NAME_IDX_EMPTY  -2

   /*checks both of the above conditions*/
#define NAME_IDX_AUTO   -3


/*--  typedefs & enums  ---------------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  function prototypes  ------------------------------------------------*/

S32 name_idx( char* name, char** name_tab, S32 no_of_entries ) ;
                /*looks up a name in name_tab*/
                /*Let no_of_entries be NAME_IDX_* for unknown size*/
                /*Returns index of entry or -1 if not found*/


/*--  macros  -------------------------------------------------------------*/

/*--  global variables  ---------------------------------------------------*/

/*extern */


/***************************************************************************/
#endif  /* NAME_IDX_H */
/***************************************************************************/
