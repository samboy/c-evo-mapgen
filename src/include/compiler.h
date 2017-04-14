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
*	compiler.h
*
* This include file has typedefs to get rid of the compiler dependent
* native type sizes (e. g. int = 16/32 bit?  and so on)
*
* You'll have to add your compiler if it is not yet in the list
*****************************************************************************
History: (latest change first)
2016-Jul-15: Updates for gcc 4.8.2 (32 vs. 64 bit version)
2016-Mar-29: Updates for gcc 4.8.2 (64-bit types)    Conflicts with U64-lib
2014-Sep-24: Updates for gcc 4.8.2
2014-Jun-04: Quick'n'dirty workaround for DOUBLE_MAX for gcc
2012-May-13: added floating constants for TinyC and MS-C
2011-Feb-24: added func "access" for GNU-C
2010-May-22: Changed _GNU_CC_ to predefined macro __GNUC__
2010-Apr-04: added TinyCC
2007-Sep-29: TRUE redefined   -1 -> 0xff
2003-Jun-30: added macro CONCAT2
2003-Jun-29: - added type "bit"
             - added IAR C compiler
2002-Oct-26: initial version
****************************************************************************/
#ifndef COMPILER_H
#define COMPILER_H
/***************************************************************************/


/*---------  Microsoft-C 6.0 (for DOS)  -----------------------------------*/

#if defined _MS_C_
   typedef           char S8 ;
   typedef  unsigned char U8 ;
   typedef            int S16 ;
   typedef   unsigned int U16 ;
   typedef           long S32 ;
   typedef  unsigned long U32 ;
   typedef             U8 BIT ; /*best implementation on PC*/

#include <float.h>
#define DOUBLE_MAX      DBL_MAX
#define DOUBLE_MIN      DBL_MIN
#define DOUBLE_EPSILON  DBL_EPSILON


#  ifndef FALSE
#     define FALSE 0
#  endif
#  ifndef TRUE
/*#     define TRUE -1*/
             /* -1 doesn't work properly in "xxx_bit == TRUE"*/
             /*since BIT is UNsigned*/
#     define TRUE (0xff)
#  endif

#  define CONCAT_AUX(x) x
#  define CONCAT2(a,b) CONCAT_AUX(a)b


/*---------  gcc  ---------------------------------------------------------*/
/* predefined macros of gcc can be found with:
 *      cpp -dM foo.h > predefined.txt
 * where foo.h is an empty file (zero byte) */

#elif defined __GNUC__
   /*typedef            char S8 ;*/
   /*typedef   unsigned char U8 ;*/
   /*typedef           short S16 ;*/
   /*typedef  unsigned short U16 ;*/
   /*typedef             int S32 ;*/
   /*typedef    unsigned int U32 ;*/

      /*gcc nicely does the job for us*/
   typedef     __INT8_TYPE__ S8 ;
   typedef    __UINT8_TYPE__ U8 ;
   typedef    __INT16_TYPE__ S16 ;
   typedef   __UINT16_TYPE__ U16 ;
   typedef    __INT32_TYPE__ S32 ;
   typedef   __UINT32_TYPE__ U32 ;
   /*typedef  __INT64_TYPE__ S64 ;*/ /*conflict with my old 64-bit lib*/
   /*typedef __UINT64_TYPE__ U64 ;*/

   typedef              U8 BIT ; /*best (memory) implementation on PC*/

#  ifndef FALSE
#     define FALSE 0
#  endif
#  ifndef TRUE
#     define TRUE -1
#  endif

#include <values.h>
#define DOUBLE_MAX      DBL_MAX
#define DOUBLE_MIN      DBL_MIN

    //int access( char* path, int cmd ) ; /*TODO: find a better place*/
    /*where is it defined?  --> unistd.h file*/


/*---------  IAR C-Compiler for microcontrollers  -------------------------*/

#elif defined __IAR_SYSTEMS_ICC__
   typedef   signed char S8 ;
   typedef unsigned char U8 ;
   typedef           int S16 ;
   typedef  unsigned int U16 ;
   typedef          long S32 ;
   typedef unsigned long U32 ;
   /*bit is a native type for the IAR C compiler*/
   typedef           bit BIT ;

#  ifndef FALSE
#     define FALSE 0
#  endif
#  ifndef TRUE
#     define TRUE -1
#  endif

/*IAR compiler cannot concatenate strings -- run MS-C preprocessor*/
/*with -D__IAR_SYSTEMS_ICC__*/
#  define CONCAT_AUX(x) x
#  define CONCAT2(a,b) CONCAT_AUX(a)b


/*---------  Tiny-C compiler  ---------------------------------------------*/

#elif defined __TINYC__
   typedef           char S8 ;
   typedef  unsigned char U8 ;
   typedef           short S16 ;
   typedef  unsigned short U16 ;
   typedef            int S32 ;
   typedef   unsigned int U32 ;
   typedef             U8 BIT ; /*best implementation on PC*/

#include <float.h>
#define DOUBLE_MAX      DBL_MAX
#define DOUBLE_MIN      DBL_MIN
#define DOUBLE_EPSILON  DBL_EPSILON


/*-------------------------------------------------------------------------*/

#else
#  error  compiler.h: no compiler specified
#endif


/***************************************************************************/
#endif	/* COMPILER_H */
/***************************************************************************/
