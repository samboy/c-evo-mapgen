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
file_io.h

Include file for file_io.c
*****************************************************************************
History: (latest change first)
2006-Feb-05: Big endian-functions added
2004-Aug-22: initial version
****************************************************************************/
#ifndef FILE_IO_H
#define FILE_IO_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

/*#include <.h>*/


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  typedefs & enums  ---------------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  function prototypes  ------------------------------------------------*/

extern U8  get_U8(                FILE* fp ) ;
extern U16 get_U16_little_endian( FILE* fp ) ;
extern U32 get_U32_little_endian( FILE* fp ) ;
extern U16 get_U16_big_endian( FILE* fp ) ;
extern U32 get_U32_big_endian( FILE* fp ) ;

extern void put_U8(                FILE* fp,  U8 val ) ;
extern void put_U16_little_endian( FILE* fp, U16 val ) ;
extern void put_U32_little_endian( FILE* fp, U32 val ) ;
extern void put_U16_big_endian( FILE* fp, U16 val ) ;
extern void put_U32_big_endian( FILE* fp, U32 val ) ;


/*--  macros  -------------------------------------------------------------*/

/*--  global variables  ---------------------------------------------------*/

/*extern */


/***************************************************************************/
#endif	/* FILE_IO_H */
/***************************************************************************/
