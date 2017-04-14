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
cpp_getc.h

Include file for cpp_getc.c
*****************************************************************************
History: (latest change first)
2013-Feb-26: included <misc.h> for U16
2003-Aug-15: initial version
****************************************************************************/
#ifndef CPP_GETC_H
#define CPP_GETC_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

#include <misc.h>


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  typedefs & enums  ---------------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  function prototypes  ------------------------------------------------*/

   /*This function returns the next char to process.*/
   /*It evaluates the following sources, given in order of priority*/
   /*1. A macro expansion*/
   /*2. A char which has been put back previously*/
   /*3. Chars from input file resp. include file*/
int cpp_get_chr( void ) ;

   /*max. ONE char may be put back*/
void cpp_put_back_chr( int chr ) ;

/*--  macros  -------------------------------------------------------------*/

/*--  global variables  ---------------------------------------------------*/

   /*these flags always correspond to the char returned by cpp_get_chr()*/
extern BIT flag_is_white ;
extern BIT flag_is_digit ;
extern BIT flag_is_letter_or_underscore ;
extern BIT flag_is_alphanum_or_underscore ;

extern U16 cpp_line_no ;
extern char* cpp_file_name ;


/***************************************************************************/
#endif	/* CPP_GETC_H */
/***************************************************************************/
