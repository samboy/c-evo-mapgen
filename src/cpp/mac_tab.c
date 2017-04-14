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
mac_tab.c

This library implements a macro table for CPP.
*****************************************************************************
History: (latest change first)
2013-Feb-26: predefined macro ___MAKE_STR
2003-Aug-14..30: initial version
*****************************************************************************
Global objects:
- x
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*only for testing purposes*/



/*--  include files  ------------------------------------------------------*/

#include "mac_tab.h"

#include <stdio.h>
/*#include <stdlib.h>*/
#include <string.h>

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

#define MAX_ENTRIES  500
#define MAX_NESTING_LEVEL 20


/*--  type declarations & enums  ------------------------------------------*/

typedef struct {
   char* name ;
   char* body ;
   U8    no_of_params ;
   BIT   in_use ; /*to prevent recursion*/
} MAC_TAB_ENTRY ;


/*--  local function prototypes  ------------------------------------------*/

static S16 get_index_by_name( char* name ) ;
static U8 calc_argc_from_arglist( char** arglist ) ;


/*--  macros  -------------------------------------------------------------*/


/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/

static MAC_TAB_ENTRY table [ MAX_ENTRIES ] = {
  { "___MAKE_STR", "\"$(0)\"", 1, 0 }           /*predefined macro*/
} ;
static U16 no_of_entries = 1 ;

static U8 nesting_level ;
static char  expansion_str [ 1000 ] ;
static char* expansion_pointer [ MAX_NESTING_LEVEL +1 ] ;
static char*      free_pointer [ MAX_NESTING_LEVEL +1 ] ;


/*-------------------->   mac_tab_init   <----------------------- 2013-Feb-26
This function initializes the macro table.
-----------------------------------------------------------------------------
Used functions:
Parameters:	none
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
void mac_tab_init( void )
{
THIS_FUNC(mac_tab_init)
   /*no_of_entries = 0 ;*/ /*CAUTION: predefined macro !!!*/
   for ( nesting_level = 0 ;
         nesting_level < MAX_NESTING_LEVEL ;
         nesting_level++ ) {
      expansion_pointer [ nesting_level ] = NULL ;
   }
   free_pointer [ 0 ] = expansion_str ; /*it has  *always*  this value*/
   nesting_level = 0 ;
}

/*-------------------->   mac_tab_add   <--------------------- 2003-Aug-30 --
This function adds a new macro to the table.
-----------------------------------------------------------------------------
Used functions: fprintf, exit,
                mac_tab_is_defined, calc_argc_from_arglist, strdup
Globals:
Internals: table, no_of_entries
Parameters:     - name
                - arg_list  only needed to calculate argc
                - body      precompiled body with args replaced by $(nnn)
Return value:   void
Exitcode:       EXITCODE_TABLE_FULL, EXITCODE_SYNTAX_ERR
---------------------------------------------------------------------------*/
void mac_tab_add( char* name, char** arg_list, char* body )
{
THIS_FUNC(mac_tab_add)
   U8 arg_cnt ;

   PRT_VAR(name,s)

   if (no_of_entries == MAX_ENTRIES -1) {
      fprintf( stderr, "macro table full\n" ) ;
      exit( EXITCODE_TABLE_FULL ) ;
   }

   if (mac_tab_is_defined( name )) {
      fprintf( stderr, "macro already defined: %s\n", name ) ;
      exit( EXITCODE_SYNTAX_ERR ) ;
   }

   (table [ no_of_entries ]).name         = strdup( name ) ;
   (table [ no_of_entries ]).in_use       = 0 ;

   arg_cnt = calc_argc_from_arglist( arg_list ) ;
   (table [ no_of_entries ]).no_of_params = arg_cnt ;
   (table [ no_of_entries ]).body         = strdup( body ) ;

   no_of_entries++ ;
}

/*-------------------->   mac_tab_del   <------------------------ 2013-Feb-26
This function deletes one entry from mac_tab.
Strategy: Overwrite the entry to delete with the last entry in mac_tab.
Decrement "no_of_entries" then.  If the entry to delete  *is*  the last
entry, simply copy it to itself.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- name  to delete
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
void mac_tab_del( char* name )
{
THIS_FUNC(mac_tab_del)
   S16 index ;

   PRT_VAR(name,s)

   index = get_index_by_name( name ) ;
   PRT_VAR(index,d)
   if (index == -1) {
      fprintf( stderr, "mac_tab_del: Warning: No such macro: \"%s\"\n",
                       name ) ;
      return ;
   }

   table [ index ] = table [ no_of_entries -1 ] ;
   DEB(( stderr, "copying %s from %u to %u\n",
         (table [index]).name, no_of_entries -1, index ))
   no_of_entries-- ;
}

/*-------------------->   mac_tab_is_defined   <-------------- 2003-Aug-15 --
This function tells if a given macro is defined.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
Return value:	TRUE if name is defined, else FALSE
Exitcode:	--
---------------------------------------------------------------------------*/
BIT mac_tab_is_defined( char* name )
{
THIS_FUNC(mac_tab_is_defined)
   if (get_index_by_name( name ) == -1) {
      return FALSE ;
   }
   return TRUE ;
}

/*-------------------->   mac_tab_expand   <--------------------- 2013-Feb-26
This function x
-----------------------------------------------------------------------------
Used functions:
Parameters:	- name
                - arglist
Return value:	TRUE if name is defined, else FALSE
Exitcode:	x
---------------------------------------------------------------------------*/
void mac_tab_expand( char* name, char** arg_list )
{
THIS_FUNC(mac_tab_expand)
   S16 index ;
   U8 no_of_args ;
   char* pbody_poi ;
   char* exp_poi ;
   char c ;
   char* arg ;
   U8 arg_number ;


   PRT_VAR(name,s)

   index = get_index_by_name( name ) ;
   if (index == -1) {
      fprintf( stderr, "No such macro: \"%s\"\n", name ) ;
      exit( EXITCODE_WRONG_CALL ) ;
   }

   if (arg_list == NULL) {
      no_of_args = 0 ;
   }
   else {
      no_of_args = calc_argc_from_arglist( arg_list ) ;
   }
   PRT_VAR(no_of_args,u)
   DEB_STATEMENT(if (no_of_args > 0) PRT_VAR(arg_list[0],s))
   DEB_STATEMENT(if (no_of_args > 1) PRT_VAR(arg_list[1],s))
   DEB_STATEMENT(if (no_of_args > 2) PRT_VAR(arg_list[2],s))
   if (no_of_args != (table [index]).no_of_params) {
      fprintf( stderr,
         "macro %s has been called with wrong # of params\n", name ) ;
      exit( EXITCODE_SYNTAX_ERR ) ;
   }

   if (nesting_level == MAX_NESTING_LEVEL) {
      fprintf( stderr, "max. macro nesting level = %u\n", MAX_NESTING_LEVEL ) ;
      exit( EXITCODE_TABLE_FULL ) ;
   }

   pbody_poi = (table [ index ]).body ;
   PRT_VAR(pbody_poi,s)
   exp_poi = free_pointer [ nesting_level ] ;
   expansion_pointer [ nesting_level +1 ] = exp_poi ;
   while ((c = *pbody_poi++) != '\0') {
      if ((exp_poi - expansion_str) > 990) {
         fprintf( stderr, "macro expansion buffer overflow\n" ) ;
         exit( EXITCODE_TABLE_FULL ) ;
      }
      if (c == '$') { /*escape character found*/
         c = *pbody_poi++ ; /*read character after escape character*/
         if (c == '$') { /*it was "$$"*/
            *exp_poi++ = c ; /*$$ -> $*/
         }
         else if (c == '(' ) { /*arg# follows*/
            arg_number = 0 ;
            while (1) {
               c = *pbody_poi++ ;
               if ((c < '0') || (c > '9')) { /*not a digit*/
                  break ;
               }
               arg_number = 10 * arg_number + (c - '0') ;
            }
            PRT_VAR(arg_number,u)
            if (c != ')' ) {
               fprintf( stderr,
                  "error in precompiled macro body (expected ')')\n" ) ;
               exit( EXITCODE_UNEXPECTED_ERR ) ;
            }
            arg = arg_list [ arg_number ] ;
            ASSERT(arg != NULL)
            PRT_VAR(arg,s)
            strcpy ( exp_poi, arg ) ;
            exp_poi += strlen( arg ) ;
         }
         else {
            fprintf( stderr,
               "error in precompiled macro body (expected '(')\n" ) ;
            exit( EXITCODE_UNEXPECTED_ERR ) ;
         }
      }
      else { /*not a '$'*/
         *exp_poi++ = c ;
      }
   }
   *exp_poi++ = '\0' ;
   free_pointer [ nesting_level+1 ] = exp_poi ;

   nesting_level++ ;
   PRT_VAR(nesting_level,u)
   PRT_VAR(expansion_pointer[nesting_level],s)
}

/*-------------------->   mac_tab_get_chr   <----------------- 2003-Aug-27 --
This function x
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
int mac_tab_get_chr()
{
THIS_FUNC(mac_tab_get_chr)

   while (1) {
      if (nesting_level == 0) {
         return EOF ;
      }
      if (*expansion_pointer [ nesting_level ] == '\0') {
         nesting_level-- ;
      }
      else {
         return *(expansion_pointer [ nesting_level ])++ ;
      }
   }
}

/*-------------------->   mac_tab_put_back_chr   <------------ 2003-Aug-29 --
This function x
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
void mac_tab_put_back_chr( int chr )
{
THIS_FUNC(mac_tab_put_back_chr)
   DEB((stderr,"to put_back queue: >%c<\n", chr))

   if (chr != *(--expansion_pointer [ nesting_level ])) {
      fprintf( stderr, "put_back character mismatch\n" ) ;
      exit( EXITCODE_UNEXPECTED_ERR ) ;
   }
}

/*-------------------->   get_index_by_name   <------------------ 2013-Feb-26
This function x
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
Return value:	TRUE if name is defined, else FALSE
Exitcode:	x
---------------------------------------------------------------------------*/
static S16 get_index_by_name( char* name )
{
THIS_FUNC(get_index_by_name)
   S16 index ;

   for ( index = 0 ; index < no_of_entries ; index++ ) {
      if (strcmp( name, (table [ index ]).name ) == 0) {
         return index ;
      }
   }
   return -1 ;
}

/*-------------------->   calc_argc_from_arglist   <---------- 2003-Aug-21 --
This function returns argc for a given NULL terminated argv list
-----------------------------------------------------------------------------
Used functions:
Parameters:	- arglist  is an array of char*
                           its end is marked by a NULL pointer
Return value:	number of args, not including the NULL pointer
Exitcode:	--
---------------------------------------------------------------------------*/
static U8 calc_argc_from_arglist( char** arglist )
{
THIS_FUNC(calc_argc_from_arglist)
   U8 arg_cnt ;
   char** arg_poi ;

   arg_cnt = 0 ;
   arg_poi = arglist ;
   while ( *arg_poi != NULL) {
      arg_cnt++ ;
      arg_poi++ ;
   }
   PRT_VAR(arg_cnt,u)
   return arg_cnt ;
}

#ifdef DEBUG
/*-------------------->   mac_tab_print   <------------------- 2003-Aug-17 --
This function x
-----------------------------------------------------------------------------
Used functions:
Parameters:	- x
Return value:	TRUE if name is defined, else FALSE
Exitcode:	x
---------------------------------------------------------------------------*/
void mac_tab_print( void )
{
THIS_FUNC(mac_tab_print)
   U16 index ;

   printf( "++++++++++  macro table  +++++++++++++++++++++\n" ) ;
   for ( index = 0 ; index < no_of_entries ; index++ ) {
      printf( "%s, # of params = %u, body = >%s<\n",
         (table [ index ]).name,
         (unsigned)((table [ index ]).no_of_params),
         (table [ index ]).body
            ) ;
   }
   printf( "++++++++++  end of macro table  ++++++++++++++\n" ) ;
}
#endif /*DEBUG*/
/***************************************************************************/
