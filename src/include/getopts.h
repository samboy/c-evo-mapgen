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
*	getopts.h
* Include-Datei fuer getopts.c
*****************************************************************************
* Aenderungen:
2014-Jun-04: changed order of definitions (struct optdef first)
2013-Feb-25: added "fprintf_argv"
2013-Feb-22: function prototype *with* correct parameters
* 15.03.98: Das Makro OPTION_GET nimmt keinen Exitcode mehr an, da im Fehler-
*	    fall usage() aufgerufen wird, welches selbst terminiert.
*	    ALTE PROGRAMME MUESSEN VOR NEUCOMPILATION ANGEPASST WERDEN!!
* 03.04.96-05.04.96: Ersterstellung
****************************************************************************/
#ifndef GETOPTS_H
#define GETOPTS_H
/***************************************************************************/
#include <misc.h>
#include <stdio.h>

#define OPT_NUMBER '\001'
#define NO_ARG NULL

struct optdef {
	char optchar ;
	char *optflag ;
	char **optarg ;
} ;

/*int getopts( int optc, char** optv, struct optdef *defi_list ) ;*/
BIT getopts( int* optc, char*** optv, struct optdef* defi_list ) ;
void fprint_argv( FILE* fp, int argc, char** argv ) ;

#define OPTION_LIST(list_name) static struct optdef list_name [] = {
#define OPTION_WO_ARG(chr,flag) { chr, &flag, NO_ARG },
#define OPTION_W_ARG(chr,flag,arg) { chr, &flag, &arg },
#define OPTION_NUMBER(flag,arg) { OPT_NUMBER, &flag, (char**)&arg },
#define OPTION_LIST_END { '\0', NULL, NO_ARG } } ;

#define OPTION_GET(list_name)  \
	/*if (getopts( &argc, &argv, &list_name)) {*/  \
	if (getopts( &argc, &argv, list_name)) {  \
		usage() ;  \
	}

/***************************************************************************/
#endif /* GETOPTS_H */
/***************************************************************************/
