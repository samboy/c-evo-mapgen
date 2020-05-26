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
now.h

Include file for now.c
*****************************************************************************
History: (latest change first)
2017-Mar-05: Backport to DOS
2014-Jul-04: changed "secs_since_1970" to 'time_t' (=standard)
2013-Oct-11: added 'COMPACT' format
2010-May-27: added "set_vts_by_secs_since_1970"
2010-May-14: initial version
****************************************************************************/
#ifndef NOW_H
#define NOW_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

#include <time.h> /*for time_t*/

#include <misc.h>


/*--  constants  ----------------------------------------------------------*/

   /*format flags (2nd param for "now")*/
   /*influence the string format, can be OR'ed (except formats)*/
   /*COMPACT is ISO w/o dashes, colons and blanks*/
#define  NOW_STD_OUTPUT 0x00
#define  NOW_NO_NEWLINE 0x01
#define  NOW_ISO        0x02
#define  NOW_COMPACT    0x04

   /*for convenience*/
#define  VTS_STD_OUTPUT  NOW_STD_OUTPUT
#define  VTS_NO_NEWLINE  NOW_NO_NEWLINE
#define  VTS_ISO         NOW_ISO


/*--  typedefs & enums  ---------------------------------------------------*/

typedef struct {
  time_t secs_since_1970 ; /*Jan 1st, 00:00:00 local time (TZ)*/
                           /*With some compilers 32 bit, others 64*/
  U16 year ; /*20xx*/
  U8  month ; /*1..12*/
  U8  day ; /*1..31*/
  U8 hour ; /*0..23*/
  U8 min ; /*0..59*/
  U8 sec ; /*0..59*/
  U8 weekday ; /*Mon=1, Tue=2, ...*/
  char MMM [4] ; /*null terminated 3 letter month abbrevation*/
  char WWW [4] ; /*null terminated 3 letter weekday abbrevation*/
  char output_string [ 26 ] ; /*null terminated date & time string*/
} VERSATILE_TIME_STRUCT ;


/*--  function prototypes  ------------------------------------------------*/

int now( VERSATILE_TIME_STRUCT* p, U8 format_flags ) ;
int set_vts_by_secs_since_1970( VERSATILE_TIME_STRUCT* p, U8 format_flags ) ;

/*--  macros  -------------------------------------------------------------*/

/*--  global variables  ---------------------------------------------------*/

/*extern */


/***************************************************************************/
#endif	/* NOW_H */
/***************************************************************************/
