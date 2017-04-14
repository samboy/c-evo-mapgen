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
falloc.c

This lib contains 'forced' alloc/free routines
*****************************************************************************
History: (latest change first)
2013-Jul-06..07: added "fstrdup"
2012-Jul-07: - change to std filehead format
             - added ffree
2006-Sep-02: If alloc fails, give statistic info
2006-Jul-08: Debug-Help (PRT_VAR)
1996-Apr-02: last change in old header format
1989-Aug-15: initial version
*****************************************************************************
Global objects:
- void* falloc( size_t size )
- void ffree( void* )
- char* fstrdup( char* str )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*only for testing purposes*/

   /*makes fstrdup() slower, but gives additional info*/
#define  ADDITIONAL_STATISTICS


/*--  include files  ------------------------------------------------------*/

#include <misc.h>
#include <stdio.h>

#ifdef _TURBO_C_
# include <process.h>
# include <alloc.h>
#elif __GNUC__
# include <stdlib.h>
#else
# include <process.h>
# include <malloc.h>
#endif

#include <falloc.h>
#include <string.h> /*for strdup()*/

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/

static void no_mem( void ) ; /*reports statistics to stderr*/


/*--  macros  -------------------------------------------------------------*/


/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/

static unsigned long no_of_chunks = 0 ;
static unsigned long total_chunk_size = 0 ;

static unsigned long no_of_strings = 0 ;
#ifdef ADDITIONAL_STATISTICS
 static unsigned long total_string_size = 0 ;
#endif /*ADDITIONAL_STATISTICS*/


#ifdef MAIN
   /* Options */
/*static char flag_ ;*/
/*static char* _arg ;*/
/*static char flag_count ;*/
/*static unsigned long count ;*/

/*OPTION_LIST(optlist)*/
/*OPTION_WO_ARG('x',flag_x)*/
/*OPTION_W_ARG('y',flag_y,y_arg)*/
/*OPTION_NUMBER(flag_count,count)*/
/*OPTION_LIST_END*/

/*-------------------->   main   <------------------------------- 2013-Jul-07
Purpose see file header
-----------------------------------------------------------------------------
Used functions: usage
Globals/Internals: x
Parameters:     - argc, argv
Return value:   --
Exitcode:       EXITCODE_OK, EXITCODE_USAGE
---------------------------------------------------------------------------*/
void main( int argc, char** argv )
{
THIS_FUNC(main)
  char* temp ;
  unsigned long i ;
  char* test_str = "very longgggggggggggggggggggggggggggggg test string" ;


  /*OPTION_GET(optlist)*/
  /*if ( argc != 2 ) {*/
    /*usage() ;*/
  /*}*/

#ifdef __TINYC__
# define CHUNK_SIZE 1000000
#elif _MS_C_
# define CHUNK_SIZE 10000
#else
# error  falloc.c: main(test): no compiler specified
#endif

  i = 0 ;
  while(TRUE) {
    /*falloc( CHUNK_SIZE ) ;*/
    temp = fstrdup( test_str ) ;
    ASSERT(strcmp( temp, test_str ) == 0)
    i++ ;
    if (i % 10000 == 0) {
      PRT_VAR(i,lu)
    }
  }

  exit( EXITCODE_OK ) ;
}
#endif /* MAIN */



/*--  library functions  --------------------------------------------------*/

/*-------------------->   falloc   <----------------------------- 2013-Jul-06
This function returns a chunk of memory from heap.  Errors are checked.
-----------------------------------------------------------------------------
Used functions: malloc, no_mem
Globals/Internals: --
Parameters:     - size    of chunk in bytes
Return value:   pointer to chunk
Exitcode:       x
---------------------------------------------------------------------------*/
void* falloc( size_t size )
{
  THIS_FUNC(falloc)
  void* ret_val ;

  PRT_VAR(size,u) ;
  if ((ret_val = malloc( size )) == NULL) {
    fprintf( stderr, "falloc: out of memory\n" ) ;
    no_mem() ;
  }
  PRT_VAR(ret_val,08lx) ;
  no_of_chunks++ ;
  total_chunk_size += size ;
  return ret_val ;
}

/*-------------------->   ffree   <------------------------------ 2012-Jul-07
This function frees a memory block previously allocated with falloc.
-----------------------------------------------------------------------------
Used functions: free
Globals/Internals: --
Parameters:     - x
Return value:   x
Exitcode:       x
---------------------------------------------------------------------------*/
void ffree( void* memblock )
{
THIS_FUNC(ffree)
  free( memblock ) ;
         /*free does not generate any errors, so no errors can be checked*/
}

/*-------------------->   fstrdup   <---------------------------- 2013-Jul-07
This function calls the strdup() function and tests for its failure.
It exits if out of memory.
-----------------------------------------------------------------------------
Used functions: strdup, no_mem, strlen
Globals:   x
Internals: x
Parameters:     - str    string to duplicate
Return value:   x
Exitcode:       EXITCODE_NO_MEM
---------------------------------------------------------------------------*/
char* fstrdup( char* str )
{
THIS_FUNC(fstrdup)
  char* ret_val ;
  unsigned long len ;


  PRT_VAR(str,s)

     /*version with strdup*/
#ifdef NEVER
  ret_val = strdup( str ) ;
  if (ret_val == NULL) {
    fprintf( stderr, "fstrdup: out of memory\n" ) ;
    no_mem() ;
  }
  no_of_strings++ ;
#ifdef ADDITIONAL_STATISTICS
  total_string_size += 1 + strlen( str ) ;
#endif /*ADDITIONAL_STATISTICS*/
#endif /*NEVER*/


     /*version with malloc*/
  len = 1 + (unsigned long)strlen( str ) ; /*plus 1 for '\0'*/
  ret_val = (char*)malloc( (size_t)len ) ;
  if (ret_val == NULL) {
    fprintf( stderr, "fstrdup: out of memory\n" ) ;
    no_mem() ;
  }
  strcpy( ret_val, str ) ;
  no_of_strings++ ;
  total_string_size += len ;

  PRT_VAR(ret_val,08lx) ;
  return ret_val ;
}

/*-------------------->   no_mem   <----------------------------- 2013-Jul-06
This function prints an error message and exits.
-----------------------------------------------------------------------------
Used functions: fprintf, exit
Globals:   --
Internals: no_of_chunks, total_size
Parameters:     --
Return value:   does not return
Exitcode:   EXITCODE_NO_MEM    
---------------------------------------------------------------------------*/
static void no_mem( void )
{
THIS_FUNC(no_mem)
  fprintf( stderr, "allocated so far: %lu bytes in %lu chunks\n",
                   total_chunk_size, no_of_chunks ) ;

#ifdef ADDITIONAL_STATISTICS
  fprintf( stderr, "strdup'ed so far: %lu bytes in %lu strings\n",
                   total_string_size, no_of_strings ) ;
#else
  fprintf( stderr, "strdup'ed so far: %lu strings\n", no_of_strings ) ;
#endif /*ADDITIONAL_STATISTICS*/
  exit( EXITCODE_NO_MEM ) ;
}

/*-------------------->   x   <---------------------------------- 2013-Jul-06
This function x
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters:     - x
                - x
Return value:   x
Exitcode:       x
---------------------------------------------------------------------------*/
/*x()*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/
/***************************************************************************/
