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
now.c

This program implements the 'now' command as well as a 'now' library func.
Compile with "-DMAIN" to generate command, without to generate Lib-Func.
The command writes the date & time to stdout.
The lib func returns this data in a structure.
*****************************************************************************
History: (latest change first)
2014-Oct-31: modifications for Unix compatibility
2014-Jul-04: modifications for Unix compatibility
2014-Jun-04: fixed bug in call to printf() -- was wrong arg type
2013-Oct-11: added 'COMPACT' format
2013-Oct-04: - testing unixtime_to_str()
             - raw output (seconds since Jan 1st, 1970)
2011-Apr-26: Change for GNU-C in "set_vts_by_secs_since_1970"
2010-May-27: added "set_vts_by_secs_since_1970"
2010-May-14: implementation of lib func
2007-Apr-01: Option '-i' fuer ISO-Datumsformat (YYYY-MM-DD)
* 16.07.00: Anpassung an geaendertes 'OPTION_GET'
* 30.08.97: Option '-n'
* 23.07.96: Ersterstellung
*****************************************************************************
Global objects:
- void main(int argc, char** argv)
- void now( VERSATILE_TIME_STRUCT* p, U8 format_flags )
- void set_vts_by_secs_since_1970( VERSATILE_TIME_STRUCT* p, U8 format_flags )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*for program 'now'*/



/*--  include files  ------------------------------------------------------*/

#include <now.h>
/*#include <misc.h>*/
#include <stdio.h>
#include <string.h>
#include <time.h>
/*#include <stdlib.h>*/
#include <getopts.h>

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/

#ifndef MAIN
/*static char* unixtime_to_str( U32 seconds_since_1970 ) ;*/
char* unixtime_to_str( U32 seconds_since_1970 ) ;
#endif /*MAIN*/

#ifdef MAIN
static void usage( void ) ;


/*--  macros  -------------------------------------------------------------*/


/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/

	/* Optionen */
static char flag_no_newline ;
static char flag_iso ;
static char flag_raw ;
static char flag_compact ;
/*static char* _arg ;*/
/*static char flag_count ;*/
/*static unsigned long count ;*/

OPTION_LIST(optlist)
OPTION_WO_ARG('n',flag_no_newline)
OPTION_WO_ARG('i',flag_iso)
OPTION_WO_ARG('s',flag_raw)
OPTION_WO_ARG('c',flag_compact)
/*OPTION_W_ARG('y',flag_y,y_arg)*/
/*OPTION_NUMBER(flag_count,count)*/
OPTION_LIST_END

/*-------------------->   main   <------------------------------- 2013-Oct-11
Purpose see file header
-----------------------------------------------------------------------------
Used functions: x
Globals/Internals: optlist
Parameters:	- argc, argv
Return value:	--
Exitcode:	EXITCODE_OK, EXITCODE_USAGE
---------------------------------------------------------------------------*/
void main( int argc, char** argv )
{
THIS_FUNC(main)
  VERSATILE_TIME_STRUCT vts ;
  U8 format ;

  OPTION_GET(optlist)
  if ( argc != 0 ) {
    usage() ;
  }

  format = NOW_STD_OUTPUT ; /*default*/
  if (flag_no_newline) {
    format |= NOW_NO_NEWLINE ;
  }
  if (flag_iso) {
    format |= NOW_ISO ;
  }
  else if (flag_compact) {
    format |= NOW_COMPACT ;
  }

  now( & vts, format ) ;

  if (flag_raw) {
    printf( "%lu", (unsigned long)( vts.secs_since_1970 ) ) ;
    if ( ! flag_no_newline) {
      printf( "\n" ) ;
    }
  }
  else {
    printf( "%s", vts.output_string ) ; /*local time*/
    /*printf( "%s", unixtime_to_str( vts.secs_since_1970 )) ;*/ /*GMT*/
    /*printf( "%s", unixtime_to_str( 0x7fffffff )) ;*/
  }

  exit( EXITCODE_OK ) ;
}

/*-------------------->   usage   <------------------------------ 2013-Oct-11
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
  fprintf( stderr,
"usage: now [ -n ] [ -i | -c | -s ]\n"
"Options:\n"
"-n - no newline\n"
"-i - ISO format (YYYY-MM-DD hh:mm:ss)\n"
"-c - compact format (YYYYMMDDhhmmss)\n"
"-s - seconds since Jan 1st, 1970 (GMT)\n"
         ) ;
  exit( EXITCODE_USAGE ) ;
}
#endif /* MAIN */

/*------------   lib functions   ------------------------------------------*/

/*-------------------->   now   <-------------------------------- 2014-Juy-04
This function returns the current date & time
in a struct VERSATILE_TIME_STRUCT.
The lib function provides an API independent of OS and compiler.
-----------------------------------------------------------------------------
Used functions: time, set_vts_by_secs_since_1970
Globals/Internals: Read access to hardware clock
Parameters:	- p pointer to current time
		- format_flags: OR'ed flags that influence the string output
Return value:	void
Exitcode:	EXITCODE_WRONG_PARAM
---------------------------------------------------------------------------*/
int now( VERSATILE_TIME_STRUCT* p, U8 format_flags )
{
THIS_FUNC(now)
  time_t now_now ;

     /*Step 1: get time*/
  now_now = time( (time_t*)NULL ) ;
  p->secs_since_1970 = now_now ;

     /*Step 2: set the other fields*/
  return set_vts_by_secs_since_1970( p, format_flags ) ;
}

/*-------------------->   set_vts_by_secs_since_1970   <--------- 2013-Oct-11
This function sets all other fields in a VERSATILE_TIME_STRUCT according
to the field "secs_since_1970".
-----------------------------------------------------------------------------
Used functions: ctime, localtime,
                fprintf, exit,
                sprintf, strncpy, strcat
Globals/Internals: --
Parameters:	- p: pointer to VERSATILE_TIME_STRUCT
                     (must represent a date 1980 or later)
                     (can only represent 1970 or later since it is an
                     unsigned 32 bit value and zero means Jan 1st, 1970)
		- format_flags: OR'ed flags that influence the string output
Return value:	void
Exitcode:	EXITCODE_WRONG_PARAM
---------------------------------------------------------------------------*/
int set_vts_by_secs_since_1970( VERSATILE_TIME_STRUCT* p, U8 format_flags )
{
THIS_FUNC(set_vts_by_secs_since_1970)
  char* static_buf ;
  struct tm* split_buf ;

  static_buf = ctime( (time_t*)( & (p->secs_since_1970)) ) ;
             /*ctime example: "Wed Jan 02 02:03:55 1980\n\0"*/

  if (static_buf == NULL) {
    /* Let's not have this fail in 2038, thank you very much */
    fprintf(stderr, "WARNING set_vts_by_secs_since_1970: date before 1980\n") ;
       /*if this should make a problem, replace "ctime" with
         an own function*/
    /*exit( EXITCODE_WRONG_PARAM ) ;*/
    return 0; /* Probably 2038 or later */
  }
  /*PRT_VAR(static_buf,s)*/

  split_buf = localtime( (time_t*)( & (p->secs_since_1970)) ) ;
  if (split_buf == NULL) {
    fprintf( stderr, "set_vts_by_secs_since_1970: localtime failed (%lu)\n",
                     (unsigned long)(p->secs_since_1970) ) ;
    exit( EXITCODE_WRONG_PARAM ) ;
  }

  p->year = (U16)(1900 + split_buf->tm_year) ;
  p->month = (U8)(1 + split_buf->tm_mon) ;
  p->day = (U8)(split_buf->tm_mday) ;
  p->hour = (U8)(split_buf->tm_hour) ;
  p->min = (U8)(split_buf->tm_min) ;
  p->sec = (U8)(split_buf->tm_sec) ;

  strncpy( p->WWW, static_buf, 3 ) ;
  (p->WWW)[3] = '\0' ;
  /*PRT_VAR(p->WWW,s)*/

  strncpy( p->MMM, & (static_buf [4]), 3 ) ;
  (p->MMM)[3] = '\0' ;
  /*PRT_VAR(p->MMM,s)*/


     /*compose & format time string*/
  if (format_flags & NOW_ISO) {
    sprintf( p->output_string, "%04d-%02d-%02d %02d:%02d:%02d",

      split_buf->tm_year + 1900,
      split_buf->tm_mon +1,
      split_buf->tm_mday,
      split_buf->tm_hour,
      split_buf->tm_min,
      split_buf->tm_sec ) ;
  }
  else if (format_flags & NOW_COMPACT) {
    sprintf( p->output_string, "%04d%02d%02d%02d%02d%02d",
      split_buf->tm_year + 1900,
      split_buf->tm_mon +1,
      split_buf->tm_mday,
      split_buf->tm_hour,
      split_buf->tm_min,
      split_buf->tm_sec ) ;
  }
  else {
    strncpy( p->output_string, static_buf, 24 ) ;
    p->output_string [24] = '\0' ;
  }

  if ((format_flags & NOW_NO_NEWLINE) == 0) {
    strcat( p->output_string, "\n" ) ;
  }
  return 1;
}

#ifndef MAIN
/*-------------------->   unixtime_to_str   <-------------------- 2014-Jun-04
This function converts the 32 bit value representing the seconds since
Jan 1st, 1970 (GMT) into an ISO date string.

The 32 bit UNIX time (signed) will be valid until 2038-Jan-19 03:14:07
-----------------------------------------------------------------------------
Used functions: sprintf
Globals:   --
Internals: --
Parameters:     - seconds_since_1970
Return value:   x
Exitcode:       x
---------------------------------------------------------------------------*/
/*static char* unixtime_to_str( U32 seconds_since_1970 )*/
char* unixtime_to_str( U32 seconds_since_1970 )
{
THIS_FUNC(unixtime_to_str)
static char ret_val [20] ;
static const U32 non_leap_year_table [] = {
/*31, 28, 31,  30,  31,  30,  31,  31,  30,  31,  30,  31*/
  31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
} ;
static const U32 leap_year_table [] = {
  31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366
} ;
  U32 days_since_1970 ;
  U32 seconds_in_day ; /*0..86399*/

  U32 four_year_blocks, day_in_four_year_block ; /*one leap year
                                                   + three regular years*/
  /*first block: 1970, 1971, #1972, 1973 (# marks leap year)*/

  U32 day_in_year /*0..365*/, day_in_month /*0..30*/ ;

  U32 year /*e. g. 1995*/, month /*0..11*/ ;
  U32 hours, minutes, seconds ;
  U32 temp ;
  U32* table_p ;
  BIT leap_year = FALSE ; /*default*/


  /*PRT_VAR((unsigned long)seconds_since_1970,lu)*/

  days_since_1970 = seconds_since_1970 / 86400 ;
  seconds_in_day =  seconds_since_1970 % 86400 ;
  /*PRT_VAR((unsigned long)seconds_in_day,lu)*/

  four_year_blocks =       days_since_1970 / 1461 ;
  day_in_four_year_block = days_since_1970 % 1461 ;

  year = 1970 + (4 * four_year_blocks) ;
  if (day_in_four_year_block >= 1096) {
    year += 3 ;
    day_in_year = day_in_four_year_block - 1096 ;
  }
  else if (day_in_four_year_block >= 730) {
    year += 2 ;
    day_in_year = day_in_four_year_block - 730 ;
    leap_year = TRUE ;
  }
  else if (day_in_four_year_block >= 365) {
    year += 1 ;
    day_in_year = day_in_four_year_block - 365 ;
  }
  else {
    day_in_year = day_in_four_year_block ;
  }
  /*PRT_VAR((unsigned long)year,lu)*/
  /*PRT_VAR((unsigned long)day_in_year,lu)*/

  table_p = (U32*)(leap_year ? leap_year_table : non_leap_year_table) ;
  day_in_month = day_in_year ; /*default*/
  for ( month = 0 ; month < 12 ; month++ ) {
    if (day_in_year < table_p [month]) {
      if (month > 0) {
        day_in_month = day_in_year - table_p [month -1] ;
      }
      break ;
    }
  }

  hours = seconds_in_day / 3600 ;
  /*PRT_VAR((unsigned long)hours,lu)*/
  temp = seconds_in_day - 3600 * hours ;
  /*PRT_VAR((unsigned long)temp,lu)*/
  minutes = temp / 60 ;
  /*PRT_VAR((unsigned long)minutes,lu)*/
  seconds = temp - 60 * minutes ;
  /*PRT_VAR((unsigned long)seconds,lu)*/
  sprintf( ret_val, "%04u-%02u-%02u %02u:%02u:%02u",
           (unsigned)year, (unsigned)(month +1), (unsigned)(day_in_month +1),
           (unsigned)hours, (unsigned)minutes, (unsigned)seconds ) ;

  return ret_val ;
}
#endif /*MAIN*/

/*-------------------->   x   <---------------------------------- 2014-Oct-31
This function x
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters: - x
            - x
Return value: x
Exitcode:     x
---------------------------------------------------------------------------*/
/*x*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/
/***************************************************************************/
