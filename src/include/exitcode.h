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
*	exitcode.h
* Include-Datei fuer verschiedene Source-Files.
* Es werden uebliche Exit-Codes vereinbart.
*****************************************************************************
Aenderungen:
2013-Sep-05: ~_UNKNOWN_ITEM
2013-Apr-10: EXITCODE_BUFFER_FULL, EXITCODE_WRONG_DATA, EXITCODE_CANNOT_DELETE
2011-Aug-10: EXITCODE_PREMATURE_EOF
2011-Feb-28: included "stdlib.h"
2007-Sep-23: EXITCODE_CANNOT_SEEK
2007-Sep-08: EXITCODE_OUT_OF_RANGE
2006-May-26: EXITCODE_NO_SUCH_DEVICE, EXITCODE_DEVICE_NOT_AVAILABLE
2001-03-18: neu: EXITCODE_FILE_EMPTY, EXITCODE_DIR_NOT_EMPTY
2001-03-17: neu: EXITCODE_STANDARD_ERR
07.04.98: - neue Codes hinzugefuegt
05.04.98: - neue Codes hinzugefuegt
04.04.98: - neue Codes hinzugefuegt
22.03.98: - neue Codes hinzugefuegt
	  - bestehende Codes verschoben, um 1 bis 9 freizumachen
20.03.98: Korrektur
14.03.98-15.03.98: Ersterstellung
****************************************************************************/
#ifndef EXITCODE_H
#define EXITCODE_H
/***************************************************************************/

   /*for exit()*/
#include <stdlib.h>


/***  Function prototypes  *************************************************/

/***  Constants  ***********************************************************/
#define EXITCODE_OK		0
/*We don't want to use codes 1 thru 9 because of
  possible conflicts with standard applications*/

#define EXITCODE_USAGE		11
#define EXITCODE_USER_ABORT	12
#define EXITCODE_STANDARD_ERR	13

#define EXITCODE_UNEXPECTED_ERR	18
#define EXITCODE_UNSPECIFIED_ERR 19
/*#define EXITCODE_*/
#define EXITCODE_FILE_NOT_FOUND	20
#define EXITCODE_DIR_NOT_FOUND	21
#define EXITCODE_FILE_EXISTS	22
#define EXITCODE_DIR_EXISTS	23
#define EXITCODE_FILE_EMPTY	24
#define EXITCODE_DIR_NOT_EMPTY	25
#define EXITCODE_PREMATURE_EOF  26

#define EXITCODE_PERM_DENIED	29
/*#define EXITCODE_*/
#define EXITCODE_DISK_FULL	30
#define EXITCODE_NO_MEM		31
/*#define EXITCODE_CANNOT_STORE	32*/
#define EXITCODE_TABLE_FULL	32
#define EXITCODE_CANNOT_CREATE	33
#define EXITCODE_CANNOT_OPEN	34
#define EXITCODE_CANNOT_CLOSE	35
#define EXITCODE_ENTRY_NOT_FOUND 36
#define EXITCODE_OUT_OF_RANGE   37
#define EXITCODE_CANNOT_SEEK	38
#define EXITCODE_BUFFER_FULL    39
     /*Do not define BUFFER_OVERFLOW.  Check for overflow and avoid it.*/
     /*Exit with BUFFER_FULL when an overflow *would* occur.*/

#define EXITCODE_CANNOT_EXEC	40
#define EXITCODE_TIMEOUT	41
#define EXITCODE_WRONG_CALL	42
#define EXITCODE_WRONG_PARAM	43
#define EXITCODE_SYNTAX_ERR	44
#define EXITCODE_WRONG_DATA	45
#define EXITCODE_UNKNOWN_ITEM   46

#define EXITCODE_NO_SUCH_DEVICE 50
#define EXITCODE_DEVICE_NOT_AVAILABLE 51
/*#define EXITCODE_*/
#define EXITCODE_CANNOT_DELETE  60

/***  Makros  **************************************************************/

/***  Global variables  ****************************************************/
/*extern */

/***************************************************************************/
#endif	/* EXITCODE_H */
/***************************************************************************/
