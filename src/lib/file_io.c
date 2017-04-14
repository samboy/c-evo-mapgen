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
file_io.c

This is a library for file I/O.
It includes binary I/O as well as character and line I/O.
*****************************************************************************
History: (latest change first)
2013-Jul-06: clarified error message in get_U8()
2006-Feb-05: Big endian-functions added
2004-Aug-22: Initial version extracted from "wave_sed.c"
*****************************************************************************
Global objects:
  see function prototypes below
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*only for testing purposes*/



/*--  include files  ------------------------------------------------------*/

#include <misc.h>
#include <stdio.h>
/*#include <stdlib.h>*/
#include <exitcode.h>
/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

/*#define x 1*/


/*--  type declarations & enums  ------------------------------------------*/



/*--  function prototypes  ------------------------------------------------*/

U8  get_U8( FILE* fp ) ;
U16 get_U16_little_endian( FILE* fp ) ;
U32 get_U32_little_endian( FILE* fp ) ;
U16 get_U16_big_endian( FILE* fp ) ;
U32 get_U32_big_endian( FILE* fp ) ;

void put_U8( FILE* fp, U8 val ) ;
void put_U16_little_endian( FILE* fp, U16 val ) ;
void put_U32_little_endian( FILE* fp, U32 val ) ;
void put_U16_big_endian( FILE* fp, U16 val ) ;
void put_U32_big_endian( FILE* fp, U32 val ) ;


/*--  macros  -------------------------------------------------------------*/



/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/



/*-------------------->   get_U8   <-------------------------- 2013-Jul-06 --*/
/*-------------------->   get_U16_little_endian   <----------- 2004-Aug-22 --*/
/*-------------------->   get_U32_little_endian   <----------- 2003-Nov-24 --*/
/*-------------------->   get_U16_big_endian   <-------------- 2006-Feb-05 --*/
/*-------------------->   get_U32_big_endian   <-------------- 2006-Feb-05 --
This function x
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
U8 get_U8( FILE* fp )
{
THIS_FUNC(get_U8)
   U8 ret_val ;
   int temp ;

   temp  = getc( fp ) ;
   if (temp == EOF) {
      fprintf( stderr, "file_io: get_U8: premature end of file\n" ) ;
      exit( EXITCODE_SYNTAX_ERR ) ;
   }
   ret_val  = (U8)temp ;
   return ret_val ;
}


U16 get_U16_little_endian( FILE* fp )
{
THIS_FUNC(get_U16_little_endian)
   volatile U16 ret_val ; /*'volatile' to suppress optimization which might
                            invert the order of the two 'getc' calls*/

   ret_val  = (U16)get_U8( fp ) ;
   ret_val |= (U16)get_U8( fp ) << 8 ;
   return ret_val ;
}


U32 get_U32_little_endian( FILE* fp )
{
THIS_FUNC(get_U32_little_endian)
   volatile U32 ret_val ; /*'volatile' to suppress optimization which might
                            invert the order of the two calls*/

   ret_val  = (U32)get_U16_little_endian( fp ) ;
   ret_val |= (U32)get_U16_little_endian( fp ) << 16 ;
   return ret_val ;
}


U16 get_U16_big_endian( FILE* fp )
{
THIS_FUNC(get_U16_big_endian)
   volatile U16 ret_val ; /*'volatile' to suppress optimization which might
                            invert the order of the two 'getc' calls*/

   ret_val  = (U16)get_U8( fp ) << 8 ;
   ret_val |= (U16)get_U8( fp ) ;
   return ret_val ;
}


U32 get_U32_big_endian( FILE* fp )
{
THIS_FUNC(get_U32_big_endian)
   volatile U32 ret_val ; /*'volatile' to suppress optimization which might
                            invert the order of the two calls*/

   ret_val  = (U32)get_U16_little_endian( fp ) << 16 ;
   ret_val |= (U32)get_U16_little_endian( fp ) ;
   return ret_val ;
}

/*-------------------->   put_U8   <-------------------------- 2003-Dec-22 --*/
/*-------------------->   put_U16_little_endian   <----------- 2003-Dec-22 --*/
/*-------------------->   put_U32_little_endian   <----------- 2003-Dec-22 --*/
/*-------------------->   put_U16_big_endian   <-------------- 2006-Feb-05 --*/
/*-------------------->   put_U32_big_endian   <-------------- 2006-Feb-05 --
This function x
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
void put_U8( FILE* fp, U8 val )
{
THIS_FUNC(put_U8)
   fputc( (int)val, fp ) ;
}


void put_U16_little_endian( FILE* fp, U16 val )
{
THIS_FUNC(put_U16_little_endian)
   put_U8( fp, (U8)(val & 0xff) ) ;
   put_U8( fp, (U8)((val & 0xff00) >> 8) ) ;
}


void put_U32_little_endian( FILE* fp, U32 val )
{
THIS_FUNC(put_U32_little_endian)
   /*U16 temp_U16 ;*/

   put_U16_little_endian( fp, (U16)(val & 0xffff) ) ;
   put_U16_little_endian( fp, (U16)((val & 0xffff0000) >> 16) ) ;
}


void put_U16_big_endian( FILE* fp, U16 val )
{
THIS_FUNC(put_U16_big_endian)
   put_U8( fp, (U8)((val & 0xff00) >> 8) ) ;
   put_U8( fp, (U8)(val & 0xff) ) ;
}


void put_U32_big_endian( FILE* fp, U32 val )
{
THIS_FUNC(put_U32_big_endian)
   /*U16 temp_U16 ;*/

   put_U16_little_endian( fp, (U16)((val & 0xffff0000) >> 16) ) ;
   put_U16_little_endian( fp, (U16)(val & 0xffff) ) ;
}

/*-------------------->   x   <------------------------------- 2004-Aug-22 --
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
