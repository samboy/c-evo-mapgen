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
*	openfile.h
* Include-Datei fuer openfile.c
*****************************************************************************
Aenderungen:
#2016-Mar-31: no open in text mode under unix
2014-Jun-20: implemented flag ~_APPEND
2012-Dec-27: included <stdio.h> for struct FILE
2011-Aug-11: changed "forced_open_w", introduced flags
2002-Jul-09: neue Funktionen: _rb, _wt, _wb
2001-03-28: Ersterstellung
****************************************************************************/
#ifndef OPENFILE_H
#define OPENFILE_H
/***************************************************************************/

#include <stdio.h>


/***  Constants  ***********************************************************/

   /*flags*/
#define   OPENFILE_NO_FLAGS        0x00
#ifdef __unix__
#define   OPENFILE_TEXT            0x00
#else
#define   OPENFILE_TEXT            0x01
#endif
#define   OPENFILE_BIN             0x02
#define   OPENFILE_DONT_OVERWRITE  0x04
#define   OPENFILE_APPEND          0x08


/***  Type declarations ****************************************************/
/*typedef struct {*/
/*} NEW_TYPE ;*/

/***  Function prototypes  *************************************************/

FILE* forced_fopen_r( char* filename ) ; /*not for new_development*/
FILE* forced_fopen_rt( char* filename ) ;
FILE* forced_fopen_rb( char* filename ) ;
/*FILE* forced_fopen_w( char* filename ) ;*/
FILE* forced_fopen_w( char* filename, U8 flags ) ;
/*#ifndef __unix__*/
  FILE* forced_fopen_wt( char* filename ) ; /*not for new_development*/
/*#endif*/
FILE* forced_fopen_wb( char* filename ) ; /*not for new_development*/
void  forced_fclose( FILE* fp ) ;


/***  Makros  **************************************************************/

/***  Global variables  ****************************************************/
/*extern */

/***************************************************************************/
#endif	/* OPENFILE_H */
/***************************************************************************/
