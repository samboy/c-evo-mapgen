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
random.c

This library implements a random number generator.
*****************************************************************************
History: (latest change first)
2017-Mar-22: added comments & tests
2011-Feb-28: - included "time.h"
             - improved "usage" text
2010-Jun-10: the random sequence is now independent of compiler/DLL
2010-May-02..09: initial version (with compiler dependent libraries)
                 (even worse: with DLL-dependent behavior)
*****************************************************************************
Global objects:
- U16 random_init( U16 seed )
- U16 random_draw( void )
- U16 random_draw_range( U16 min, U16 max )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define TEST*/


/*--  include files  ------------------------------------------------------*/

#include <stdio.h>
#include <time.h>

#ifdef __TINYC__
# include <stdlib.h>
#endif

#include <random.h>

#ifdef TEST
# include <math.h>
# include <getopts.h>
# include <openfile.h>
#endif

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

#define WARM_UP_CYCLES 100


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/

#ifdef TEST
 static void usage( void ) ;
#endif


/*--  macros  -------------------------------------------------------------*/


/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/

/*static U32 state ;*/ /*internal memory*/
static uint32_t rg_belt[39], rg_mill[19], rg_phase = 2;

// GPLv3 note: This is *not* obfuscated. This is how I wrote this code
// by hand.

/* Tiny RadioGatun[32] implementation */
#include <stdint.h> 
#define rgp uint32_t
#define rgf(a) for(c=0;c<a;c++)
#define rgn w[c*13]^=s;u[16+c]^=s;
/* This is the Belt + Mill */
void rg(rgp*a,rgp*b){rgp m=19,A[19],x,o=13,c,y,r=0,q[3];rgf(12)b[c+
c%3*o]^=a[c+1];rgf(3){for(q[c]=b[y=12+c*o];y>c*o;b[y+1]=b[y])y--;b[
y]=q[c];}rgf(m){r=(c+r)&31;y=c*7;x=a[y++%m];x^=a[y%m]|~a[(y+1)%m];A
[c]=x>>r|x<<(32-r);}rgf(m)a[c]=A[c]^A[(c+1)%m]^A[(c+4)%m];rgf(3)a[c
+o]^=q[c];*a^=1;}
/* This converts a null-terminated string in to a rg32 state */
void rgl(rgp*u,rgp*w,char*v){rgp s,q,c,x;rgf(39)w[c]=u[c%19]=0;for(
;;rg(u,w)){rgf(3){for(s=q=0;q<4;){x=*v++;s|=(x?255&x:1)<<8*q++;if(!
x){rgn;rgf(17)rg(u,w);return;}}rgn;}}}
/* This generates a random 32-bit unsigned integer */
rgp rgi(rgp*m,rgp*b,rgp*a){if(*a&2)rg(m,b);return m[*a^=3];}

/*-----------   Library functions   ---------------------------------------*/

/*-------------------->   random_init   <------------------------ 2010-Jun-10
This function initializes the basic random generator.
-----------------------------------------------------------------------------
Used functions: time, random_draw
Globals/Internals: state
Parameters:	- seed: U16 seed value
		- use_seed: If FALSE, a random value is used as seed
Return value:	The effective seed value
Exitcode:	--
---------------------------------------------------------------------------*/
U16 random_init( BIT use_seed, U16 seed )
{
THIS_FUNC(random_init)
  U16 effective_seed ;
  U8 i ; /*loop control*/
  char seed_string[16];

  if (use_seed) {
    effective_seed = seed ;
  }
  else {
    effective_seed = (U16)(time( NULL ) & 0xffff) ;
  }
  sprintf(seed_string,"%d",effective_seed);
  DEB((stderr, "Seed value %s\n",seed_string))
  /*state = (U32)effective_seed ;*/ /*now seeding*/
  rgl(rg_mill,rg_belt,seed_string); // Init rg32 RNG

     /*warm up random generator*/
  for ( i = 0 ; i < WARM_UP_CYCLES ; i++ ) {
    random_draw() ;
  }
  return effective_seed ;
}

/*-------------------->   random_draw   <------------------------ 2017-Mar-22
This function uses RadioGatun[32], which does not have the issues
a LCG has
-----------------------------------------------------------------------------
Used functions: --
Globals/Internals: state
Parameters:	--
Return value:	pseudo random value [0..32767], uniform distribution
Exitcode:	--
---------------------------------------------------------------------------*/
U16 random_draw( void )
{
THIS_FUNC(random_draw)

     /*parameters similar to ISO/IEC 9899*/

  uint32_t state;
  state = rgi(rg_mill,rg_belt,&rg_phase);
  /*printf("%x\n",(U16)((state >> 16) & 0x7fff));*/
  return (U16)((state >> 16) & 0x7fff) ;
}

/*-------------------->   random_draw_range   <------------------ 2010-May-02
This function draws an unsigned random number in the range [min .. max].
(uniform distribution)

Important:  Double to Int conversion:
0.8 -> 0
0.9 -> 0
1.0 -> 1
1.1 -> 1
1.2 -> 1

    0 * m + b = min   => b = min
32767 * m + b = max +1            => m = (max +1 - min) / 32767
-----------------------------------------------------------------------------
Used functions: fprintf, exit, random_draw
Globals/Internals: --
Parameters:	- min
		- max
Return value:	pseudo random value [min..max], uniform distribution
Exitcode:	EXITCODE_WRONG_PARAM
---------------------------------------------------------------------------*/
U16 random_draw_range( U16 min, U16 max )
{
THIS_FUNC(random_draw_range)
  double m ;
  double number_d ;
  U16 ret_val ;

  if (max < min) {
    fprintf( stderr, "random_draw_range: max < min\n" ) ;
    exit( EXITCODE_WRONG_PARAM ) ;
  }

  m = (double)(max - min +1) / 32767.0 ;
  number_d = m * ((double)random_draw()) ; /*random_draw() = 0..32767*/
  ret_val = (U16)number_d ;
  /*PRT_VAR(m,lf)*/
  /*PRT_VAR(number_d,lf)*/
  /*PRT_VAR(ret_val,u)*/

  ret_val += min ; /*add offset*/

  ASSERT(ret_val >= min)
  if (ret_val > max) { /*rounding error*/
    ret_val = max ;
  }
  return ret_val ;
}


/*--  test main  ----------------------------------------------------------*/

#ifdef TEST
	/* Options */
/*static char flag_ ;*/
/*static char* _arg ;*/
static char flag_count ;
static unsigned long count ;

OPTION_LIST(optlist)
/*OPTION_WO_ARG('x',flag_x)*/
/*OPTION_W_ARG('y',flag_y,y_arg)*/
OPTION_NUMBER(flag_count,count)
OPTION_LIST_END

/*static U8 cnt [32768] ;*/
static U32 cnt [32768] ;

static U32 lcnt [7] ;


/*-------------------->   main   <------------------------------- 2017-Mar-22
Test program to debug the lib functions.
It can also serve as an example for how to use the lib functions.
-----------------------------------------------------------------------------
Used functions: random_init, random_draw, random_draw_range,
                printf, exit, sqrt,
                forced_fopen, forced_fclose, fprintf
Globals/Internals: all
Parameters:	- argc, argv
Return value:	- 0: ok
		- 1: error
Exitcode:	EXITCODE_OK, EXITCODE_USAGE
---------------------------------------------------------------------------*/
void main( int argc, char** argv )
{
THIS_FUNC(main)
  U32 i ; /*loop control*/
  /*U32 n, sum, sum2 ;*/
  U32 n, sum_lo, sum_hi ; /*hi&lo form a 64 bit value*/
  FILE* fp ;
  U16 dice ;
  U16 min, max ;
  /*U8 min_cnt, max_cnt ;*/
  U32 min_cnt, max_cnt ;
  double n_d, sum_d, sum2_d ; /*floating representation of n, sum, sum2*/
  double variance ;


  OPTION_GET(optlist)
  if ( argc != 0 ) {
    usage() ;
  }
  random_init( flag_count, (U16)count ) ;


/*---------  mean, variance, min, max  ------------------------------------*/
  for ( i = 0 ; i < 32768 ; i++ ) {
    cnt [i] = 0 ;
  }
  n = 0 ;
  sum_lo = sum_hi = 0 ;
  sum2_d = 0 ;
  max = 0 ;
  min = 65535 ;
  for ( i = 0 ; i < 1000000 ; i++ ) {
    dice = random_draw() ;
    n++ ;
    sum_lo += dice ;
    if (sum_lo > 0xffff0000) {
      sum_lo -= 0xffff0000 ;
      sum_hi++ ;
    }
    sum2_d += (double)dice * (double)dice ;
    if (dice > max) {
      max = dice ;
    }
    if (dice < min) {
      min = dice ;
    }
    (cnt [dice])++ ;
  }
  n_d    = (double)n ;
  sum_d  = ((double)sum_hi * (double)0xffff0000) + (double)sum_lo ;
  variance = (sum2_d - (sum_d * sum_d) / n_d) / (n_d -1) ;
  ASSERT(variance > 0)
  printf( "n=%lu  mean=%.1lf   stddev=%.1lf  min=%u  max=%u\n"
          "expected:       16383.5          9459.0\n",
          (unsigned long)n,
          sum_d / n_d,
          sqrt( variance ),
          (unsigned)min,
          (unsigned)max
        ) ;

  max_cnt = 0 ;
  min_cnt = 255 ;
  for ( i = 0 ; i < 32768 ; i++ ) {
    if (cnt [i] > max_cnt) {
      max_cnt = cnt [i] ;
    }
    if (cnt [i] < min_cnt) {
      min_cnt = cnt [i] ;
    }
  }
  printf( "min_cnt=%u  max_cnt=%u\n",
          (unsigned)min_cnt,
          (unsigned)max_cnt
        ) ;

/*---------  X/Y graphical test  ------------------------------------------*/
  fp = forced_fopen_wt( "random.csv" ) ; /*import this into Excel (X-Y Test)*/
  for ( i = 0 ; i < 4000 ; i++ ) {
    dice = random_draw() ; /*why do we skip one value?*/
    /*fprintf( fp, "%u;%u\n",*/ /*for german excel: ';'*/
    fprintf( fp, "%u,%u\n",
             (unsigned)random_draw(),
             (unsigned)random_draw()
           ) ;
  }
  forced_fclose( fp ) ;

/*---------  1..6 dice simulation (test for random_draw_range())  ---------*/
  printf( "Test 1..6\n" ) ;
  for ( i = 1 ; i < 6 ; i++ ) {
    lcnt [i] = 0 ;
  }
  for ( i = 0 ; i < 100 ; i++ ) {
    dice = random_draw_range( 1, 6 ) ;
    ASSERT(dice > 0 && dice < 7)
    (lcnt [dice])++ ;
  }
  for ( i = 1 ; i < 7 ; i++ ) {
    printf( "%u: %7lu\n", (unsigned)i, (unsigned long)lcnt [i] ) ;
  }

/*---------  write random numbers to a file  ------------------------------*/
  fp = forced_fopen_wt( "random.txt" ) ;
  for ( i = 0 ; i < 10000 ; i++ ) {
    fprintf( fp, "%u\n", (unsigned)random_draw() ) ;
  }
  forced_fclose( fp ) ;


  exit( EXITCODE_OK ) ;
}

/*-------------------->   usage   <------------------------------ 2011-Feb-28
This function displays an ultra-short usage instruction.
-----------------------------------------------------------------------------
Used functions: fprintf, exit
Globals/Internals: --
Parameters:	--
Return value:	-- (doesn't return)
Exitcode:	EXITCODE_USAGE
---------------------------------------------------------------------------*/
static void usage( void )
{
  fprintf( stderr, "usage: random [-<dec. seed value>]\n" ) ;
  exit( EXITCODE_USAGE ) ;
}
#endif /* TEST */

/*-------------------->   x   <---------------------------------- 2010-Jun-10
This function x
-----------------------------------------------------------------------------
Used functions: x
Globals/Internals: x
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
