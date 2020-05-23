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
cpp_main.c

This program implements a C preprocessor.
*****************************************************************************
History: (latest change first)
2016-Mar-31: clean up, added docu
2013-Feb-25: changed local includes from <...> to "..."
2003-Jul-13..Nov-07: initial version
*****************************************************************************
Global objects:
- void main( int argc, char** argv )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*only for testing purposes*/



/*--  include files  ------------------------------------------------------*/

#include <misc.h>
#include <stdio.h>
#include <string.h>
#include <openfile.h>
#include <stdlib.h>
#include <getopt.h>
#include <getopts.h>

#include "cpp_getc.h"
#include "inc_stk.h"
#include "mac_tab.h"

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

#define MAX_ID_LENGTH 255
#define MAX_ARGS 9
#define MAX_ARG_LEN 80


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/

static void usage( void ) ;

static void skip_whites( void ) ;
static void skip_whites_to_eol( void ) ;
static void remove_trailing_whites( char* str ) ;
static char* scan_identifier( void ) ;
static char* scan_string( void ) ;
static char* scan_chr( void ) ;
static char* scan_to_chr( char c ) ;
static char* scan_to_eol( void ) ;

static void  scan_macro_args( void ) ; /*scans all args*/
static char* scan_macro_arg( void ) ; /*scans one arg*/
static char* scan_macro_body( void ) ;


/*--  macros  -------------------------------------------------------------*/


/*--  global variables  ---------------------------------------------------*/

enum {
  FIRST_COLUMN,
  NOT_FIRST_COLUMN,
  LOOK_FOR_COMMENT, /*used whenever a '/' is found*/
  COMMENT_KR, /*K&R style comment*/
  LOOK_FOR_KR_COMMENT_END, /*used when a '*' is found in K&R comment*/
  COMMENT_CPP, /*C++ style comment*/
} state ;


/*--  internal variables  -------------------------------------------------*/

	/* Optionen */
static char flag_keep_comments ;
static char flag_include_path ;
static char* I_arg = NULL ;
static char flag_define ;
static char* D_arg ;
/*static char flag_count ;*/
/*static unsigned long count ;*/

OPTION_LIST(optlist)
OPTION_WO_ARG('C',flag_keep_comments)
OPTION_W_ARG('I',flag_include_path,I_arg) /*Achtung: kann nicht multiple*/
OPTION_W_ARG('D',flag_define,D_arg)
/*OPTION_NUMBER(flag_count,count)*/
OPTION_LIST_END


static char* arg_list [ MAX_ARGS +1 ] ; /*plus one for the last NULL pointer*/
static U8    arg_count ;

static FILE* outfile ;

static int chr ; /*must be int, otherwise not all compilers recognize EOF*/
static BIT unexpected_error = FALSE ;
static U16 exit_code = EXITCODE_OK ;


/*-------------------->   main   <-------------------------------- 2016-Mar-31
Purpose see file header
-----------------------------------------------------------------------------
Used functions:
Parameters:	- argc, argv
Return value:	- 0: ok
		- 1: error
Exitcode:	x
---------------------------------------------------------------------------*/
void main( int argc, char** argv )
{
THIS_FUNC(main)
   /*U16 temp_u16 ;*/
   U16 infile_name_len ;
   char* suffix ;
   char* id ;
   char* string ;
   char* chr_string ;
   char* macro_body ;
   char* inc_file ;
   BIT flag_found ;


   /*REDIRECT("cppdebug.log")*/
   OPTION_GET(optlist)
   PRT_VAR(argv [0],s)
   PRT_VAR(argv [1],s)
   /*PRT_VAR(argv [2],s)*/

   mac_tab_init() ;
   inc_stk_init( I_arg ) ;
   /*line_no = 1 ;*/

   switch (argc) {
   /*case 0:*/
      /*infile = stdin ;*/
      /*outfile = stdout ;*/
      /*break ;*/
   case 1:
      /*infile = forced_fopen_rt( argv [0] ) ;*/
      inc_stk_open( argv [0] ) ;

      infile_name_len = strlen( argv [0] ) ;
      suffix = (argv [0]) + infile_name_len -2 ;
      PRT_VAR(suffix,s)
      if (strcmp( suffix, ".c" ) != 0) {
         fprintf( stderr, "input file name must end in \".c\"\n" ) ;
         exit( EXITCODE_WRONG_PARAM ) ;
      }
      (argv [0]) [ infile_name_len -1 ] = 'i' ;
      PRT_VAR(argv [0],s)
      outfile = forced_fopen_wt( argv [0] ) ;
      break ;
   /*case 2:*/
      /*infile = forced_fopen_rt( argv [0] ) ;*/
      /*outfile = forced_fopen_wt( argv [1] ) ;*/
      /*break ;*/
   default:
      usage() ;
   }


   state = FIRST_COLUMN ;
   while ((chr = cpp_get_chr() ) != EOF) {
      if (unexpected_error) {
         break ;
      }

      /*PRT_VAR(state,u)*/
      switch (state) {
      case FIRST_COLUMN:
         if (chr == '#') { /*preprocessor command*/
            skip_whites() ;
            id = scan_identifier() ;
            if (id == NULL) {
               fprintf( stderr, "preprocessor command missing\n" ) ;
               exit( EXITCODE_SYNTAX_ERR ) ;
            }

            if (strcmp( id, "define" ) == 0) {
               skip_whites() ;
               id = scan_identifier() ;
               id = strdup( id ) ; /*because of "scan_macro_body*/
               PRT_VAR(id,s)

               chr = cpp_get_chr() ;
               cpp_put_back_chr( chr ) ;
               arg_list [0] = NULL ; /*default*/
               arg_count = 0 ; /*default*/
               if (chr == '(') { /*macro with args*/
                  scan_macro_args() ;
               }
               skip_whites() ;
               /*macro_body = scan_to_eol() ;*/
               macro_body = scan_macro_body() ;
               PRT_VAR(macro_body,s)
               mac_tab_add( id, arg_list, macro_body ) ;

            }
            else if (strcmp( id, "undef" ) == 0) {
               skip_whites() ;
               id = scan_identifier() ;
               skip_whites_to_eol() ; /*prints warning if non-white found*/
               mac_tab_del( id ) ; /*prints warning if not in mac_tab*/
            }
            else if (strcmp( id, "include" ) == 0) {
               skip_whites() ;
               chr = cpp_get_chr() ;
               cpp_put_back_chr( chr ) ;

               flag_found = FALSE ; /*default*/
               if (chr == '\"') {
                  inc_file = scan_string() ;
                  if (inc_file != NULL) {
                     flag_found = TRUE ;
                  }
               }
               else if (chr == '<') {
                  inc_file = scan_to_chr( '>' ) ;
                  if (inc_file != NULL) {
                     flag_found = TRUE ;
                  }
               }
               if (flag_found) {
                  PRT_VAR(inc_file,s)
                  skip_whites_to_eol() ;
                  chr = cpp_get_chr() ; /*read the '\n'*/

                  /*fputc( chr, outfile ) ;*/ /*to keep track of line numbers*/
                  /*NO !!  *Replace* the '#include' line with the*/
                  /*include file content rather than appending it!*/

                  state = FIRST_COLUMN ;

                     /*remove the double qoutes resp. the angle brackets*/
                     /*NO! Pass them to "inc_stk_open"*/
                  /*inc_file++ ;*/
                  /*inc_file [ strlen(inc_file) -1 ] = '\0' ;*/
                  inc_stk_open( inc_file ) ;
               }
               else { /*no valid file spec found*/
                  fprintf( stderr, "include file specification missing\n" ) ;
                  exit( EXITCODE_SYNTAX_ERR ) ;
               }
            }
            else {
               fprintf( stderr, "not a preprocessor command: %s\n", id ) ;
               exit( EXITCODE_SYNTAX_ERR ) ;
            }

            break ;
         }

         state = NOT_FIRST_COLUMN ;
            /*fall thru to "NOT_FIRST_COLUMN"*/


      case NOT_FIRST_COLUMN:
         switch (chr) {

         case '\n':
            state = FIRST_COLUMN ;
            fputc( chr, outfile ) ;
            break ;

         case '/': /*maybe start of comment*/
            state = LOOK_FOR_COMMENT ;
              /*don't output the '/' until we know for sure it is a comment*/
            break ;

         case '"': /*start of string constant*/
            cpp_put_back_chr( chr ) ;
            string = scan_string() ;
            if (string == NULL) {
               unexpected_error = TRUE ;
            }
            else {
               fputs( string, outfile ) ;
            }
            break ;

         case '\'': /*start of character constant*/
            cpp_put_back_chr( chr ) ;
            chr_string = scan_chr() ;
            if (chr_string == NULL) {
               unexpected_error = TRUE ;
            }
            else {
               fputs( chr_string, outfile ) ;
            }
            break ;

         default:
            if (flag_is_letter_or_underscore) {
               cpp_put_back_chr( chr ) ;
               id = scan_identifier() ;
               PRT_VAR(id,s)

                  /*check for macro here*/
               if (mac_tab_is_defined( id )) {
                  chr = cpp_get_chr() ;
                  cpp_put_back_chr( chr ) ;
                  arg_list [0] = NULL ; /*default*/
                  arg_count = 0 ; /*default*/
                  if (chr == '(') {
                     scan_macro_args() ;
                  }
                  mac_tab_expand( id, arg_list ) ;
               }
               else {
                  fputs( id, outfile ) ;
               }
            }
            else { /*copy anything else to output*/
               fputc( chr, outfile ) ;
            }
         } /*end switch chr in state "NOT_FIRST_COLUMN"*/
         break ; /*end state NOT_FIRST_COLUMN*/


      case LOOK_FOR_COMMENT:
         switch (chr) {
         case '*':
            state = COMMENT_KR ;
            break ;

         case '/':
            state = COMMENT_CPP ;
            break ;

         default: /*wasn't a comment*/
            state = NOT_FIRST_COLUMN ;
            if (chr == '\n') {
               state = FIRST_COLUMN ;
            }
            fputc( '/', outfile ) ;
            fputc( chr, outfile ) ;
         } /*end switch chr in state "LOOK_FOR_COMMENT"*/
         break ; /*end state LOOK_FOR_COMMENT*/


      case COMMENT_KR:
         switch (chr) {
         case '*':
            state = LOOK_FOR_KR_COMMENT_END ;
            break ;

         case '\n':
            fputc( chr, outfile ) ; /*needed to keep track of line numbers
                                      until #line directive is implemented*/
            break ;

         default: /*continue comment*/
            ;
         }
         break ; /*end state COMMENT_KR*/


      case LOOK_FOR_KR_COMMENT_END:
         switch (chr) {
         case '/': /*end of K&R comment*/
            state = NOT_FIRST_COLUMN ;
            break ;

         case '*':
            /*state = LOOK_FOR_KR_COMMENT_END ;*/ /*keep state!*/
            break ;

         default: /*continue comment*/
            state = COMMENT_KR ;
         }
         break ; /*end state LOOK_FOR_KR_COMMENT_END*/


      case COMMENT_CPP:
         switch (chr) {
         case '\n':
            fputc( chr, outfile ) ;
            state = FIRST_COLUMN ;
            break ;

         default: /*continue comment*/
            ;
         }
         break ; /*end state COMMENT_CPP*/


      default:
         fprintf( stderr,
            "Internal error.  State machine in unknown state (0x%x).\n",
            state ) ;
         unexpected_error = TRUE ;
         exit_code = EXITCODE_UNEXPECTED_ERR ;
      } /*end switch state*/
   } /*end while*/
   DEB((stderr,"end while\n"))



   switch (state) {
   case FIRST_COLUMN:
   case NOT_FIRST_COLUMN:
      break ;

   default:
      fprintf( stderr, "premature EOF (state machine in state 0x%x)\n",
            state ) ;
      exit_code = EXITCODE_SYNTAX_ERR ;
      unexpected_error = TRUE ;
   }



   /*DEB_STATEMENT(mac_tab_print() ;)*/

   /*if (exit_code != 0) {*/
      /*unlink( OUTPUT_FILE ) ;*/
   /*}*/
   exit( exit_code ) ;
}

/*-------------------->   usage   <--------------------------- 2003-Jul-13 --
This function displays an ultra-short usage instruction.
-----------------------------------------------------------------------------
Used functions:
Parameters:	--
Return value:	-- (doesn't return)
Exitcode:	EXITCODE_USAGE
---------------------------------------------------------------------------*/
static void usage( void )
{
   fprintf( stderr, "usage: cpp [options] [infile [outfile]]\n" ) ;
   fprintf( stderr, "options:\n" ) ;
   fprintf( stderr, "-D def    additional define\n" ) ;
   fprintf( stderr, "-I path   additional include search path\n" ) ;
   fprintf( stderr, "-C path   keep comments\n" ) ;
   exit( EXITCODE_USAGE ) ;
}

/*-------------------->   skip_whites   <--------------------- 2003-Aug-19 --
This function skips blanks and tabs.
-----------------------------------------------------------------------------
Used functions: cpp_get_chr, cpp_put_back_chr
Parameters:	--
Return value:	--
Exitcode:	--
---------------------------------------------------------------------------*/
static void skip_whites( void )
{
THIS_FUNC(skip_whites)
   int c ;

   do {
      c = cpp_get_chr() ;
   } while (flag_is_white) ;
   cpp_put_back_chr( c ) ;
}

/*-------------------->   scan_identifier   <----------------- 2003-Aug-15 --
This function scans an ID.
-----------------------------------------------------------------------------
Used functions: cpp_get_chr, cpp_put_back_chr
Parameters:	--
Return value:	scanned identifier, null-terminated, static
                or NULL if no identifier was found
Exitcode:	--
---------------------------------------------------------------------------*/
static char* scan_identifier( void )
{
   static char scan_id [ MAX_ID_LENGTH +1 ] ; /*plus one for the trailing \0*/
   char* id_poi ;
   int c ;

   c = cpp_get_chr() ;
   if ( ! flag_is_letter_or_underscore) {
      cpp_put_back_chr( c ) ;
      return NULL ;
   }
   else {
      id_poi = scan_id ;
      *id_poi++ = c ;
      while (1) { /*xxx  CHECK  FOR  OVERFLOW !!!!*/
         c = cpp_get_chr() ;
         if (flag_is_alphanum_or_underscore) {
            *id_poi++ = c ;
         }
         else {
            *id_poi = '\0' ;
            cpp_put_back_chr( c ) ;
            break ;
         }
      }
   }
   return scan_id ;
}

/*-------------------->   scan_string   <--------------------- 2003-Aug-17 --
This function scans a string.
-----------------------------------------------------------------------------
Used functions: cpp_get_chr, cpp_put_back_chr, fprintf, exit
Parameters:	--
Return value:	scanned string, null-terminated, static
                or NULL if string is too long
Exitcode:	EXITCODE_WRONG_CALL
---------------------------------------------------------------------------*/
static char* scan_string( void )
{
THIS_FUNC(scan_string)
   static char scan_string [ 1000 +1 ] ; /*plus one for the trailing \0*/
   char* poi ;
   int c ;
   enum {
      INIT,
      BACKSLASH1,
      BACKSLASH2,
      BACKSLASH3
   } string_state ;

   c = cpp_get_chr() ;
   if (c != '\"') {
      fprintf( stderr, "wrong call to \"scan_string\"\n" ) ;
      exit( EXITCODE_WRONG_CALL ) ;
   }
   scan_string [0] = '\"' ;
   poi = scan_string +1 ;
   string_state = INIT ;

   while (1) {
      c = cpp_get_chr() ;
      *poi++ = c ;
      if (c == EOF) {
         cpp_put_back_chr( c ) ;
         return NULL ;
      }
      if (poi >= scan_string + 1000) {
         return NULL ;
      }

      switch (string_state) {
      case INIT:
         switch ( c ) {
         case '"':
            *poi = '\0' ;
            return scan_string ;

         case '\\':
            string_state = BACKSLASH1 ;
            break ;

         default:
            ;
         }
         break ; /*end state INIT*/

      case BACKSLASH1:
         if (flag_is_digit) {
            string_state = BACKSLASH2 ;
         }
         else {
            string_state = INIT ;
         }
         break ; /*end state BACKSLASH1*/

      case BACKSLASH2:
         if (flag_is_digit) {
            string_state = BACKSLASH3 ;
         }
         else {
            fprintf( stderr, "invalid octal constant in string\n" ) ;
            exit( EXITCODE_SYNTAX_ERR ) ;
         }
         break ; /*end state BACKSLASH2*/

      case BACKSLASH3:
         if (flag_is_digit) {
            string_state = INIT ;
         }
         else {
            fprintf( stderr, "invalid octal constant in string\n" ) ;
            exit( EXITCODE_SYNTAX_ERR ) ;
         }
         break ; /*end state BACKSLASH3*/

      default:
         fprintf( stderr,
            "Internal error."
            "  String scan state machine in unknown state (0x%x).\n",
            state ) ;
         exit( EXITCODE_UNEXPECTED_ERR ) ;
      } /*end of switch (string_state)*/
   } /*end of while (1)*/
}

/*-------------------->   scan_chr   <------------------------ 2003-Aug-17 --
This function scans a character constant.
-----------------------------------------------------------------------------
Used functions: cpp_get_chr, fprintf, exit
Parameters:	--
Return value:	scanned character constant, null-terminated, static
                or NULL on syntax error or EOF
Exitcode:	EXITCODE_WRONG_CALL
---------------------------------------------------------------------------*/
static char* scan_chr( void )
{
THIS_FUNC(scan_chr)
   static char scan_chr [ 4 +1 ] ; /*plus one for the trailing \0*/
   char* poi ;
   int c ;

   c = cpp_get_chr() ;
   if (c != '\'') {
      fprintf( stderr, "wrong call to \"scan_chr\"\n" ) ;
      exit( EXITCODE_WRONG_CALL ) ;
   }
   scan_chr [0] = '\'' ;
   poi = scan_chr +1 ;
   /*string_state = INIT ;*/

   c = cpp_get_chr() ;
   if (c == EOF) {
      return NULL ;
   }
   if (c == '\\') {
      *poi++ = c ;
      c = cpp_get_chr() ;
      if (c == EOF) {
         return NULL ;
      }
   }
   *poi++ = c ;

   c = cpp_get_chr() ;
   if (c != '\'') {
      fprintf( stderr, "invalid character constant\n" ) ;
      exit( EXITCODE_SYNTAX_ERR ) ;
   }
   *poi++ = c ;
   *poi   = '\0' ;
   return scan_chr ;
}

/*-------------------->   scan_to_chr   <--------------------- 2003-Aug-15 --
This function scans to the character 'c' (including 'c') and returns
all preceeding characters and 'c'.
It returns NULL if 'c' wasn't found before the internal buffer overflows or
an EOF is encountered.
-----------------------------------------------------------------------------
Used functions: cpp_get_chr
Parameters:	--
Return value:	scanned string, null-terminated, static
                or NULL if 'c' wasn't found
Exitcode:	--
---------------------------------------------------------------------------*/
static char* scan_to_chr( char c )
{
   static char ret_val [ 1000 +1 ] ; /*plus one for the trailing \0*/
   char* poi ;
   int cc ;

   poi = ret_val ;
   while ((cc = cpp_get_chr()) != c) {
      if (cc == EOF) {
         cpp_put_back_chr( cc ) ;
         return NULL ;
      }
      *poi++ = cc ;
      if (poi >= ret_val + 1000) {
         return NULL ;
      }
   }
   *poi++ = cc ;
   *poi = '\0' ;
   return ret_val ;
}

/*-------------------->   scan_to_eol   <--------------------- 2003-Aug-20 --
This function scans to the end of line, excluding the '\n' character.
A backslash at the end of line, however, masks the following '\n' and does
*not* stop the function, i. e. it will continue with the next line.
Both the backslash and the eol character are omitted and *not* copied to
the returned string.  That means the returned string will never contain
a newline character.  In case of a backslash/newline pair, however, a single
newline is copied to the output file to keep track of the line numbers!

The final \n is put back into the put_back queue for evaluation by the
main loop.

The Function returns NULL if an eol wasn't found before the internal buffer
overflows or if an EOF is encountered.
-----------------------------------------------------------------------------
Used functions: cpp_get_chr, cpp_put_back_chr
Parameters:	--
Return value:	scanned string, null-terminated, static
                or NULL if an eol wasn't found
Exitcode:	--
---------------------------------------------------------------------------*/
static char* scan_to_eol( void )
{
   static char ret_val [ 1000 +1 ] ; /*plus one for the trailing \0*/
   char* poi ;
   int cc ;
   BIT flag_backslash ;

   poi = ret_val ;
   flag_backslash = FALSE ;
   while (1) {
      cc = cpp_get_chr() ;
      if (cc == EOF) {
         cpp_put_back_chr( cc ) ;
         return NULL ;
      }

      if (flag_backslash) {
         if (cc == '\n') {
            fputc( cc, outfile ) ; /*keep track of line numbers!*/
         }
         else {
            *poi++ = '\\' ;
            *poi++ = cc ;
         }
         flag_backslash = FALSE ;
      }
      else {
         if (cc == '\\') {
            flag_backslash = TRUE ;
         }
         else {
            if (cc == '\n') {
               cpp_put_back_chr( cc ) ;
               *poi = '\0' ;
               return ret_val ;
            }
            else {
               *poi++ = cc ;
            }
         }
      }

      if (poi >= ret_val + 999) {
         return NULL ;
      }
   }
}

/*-------------------->   scan_macro_args   <----------------- 2003-Aug-23 --
This function scans all arguments of a macro.
It 'eats up' the whole list including the parentheses ( ... , ... , ... ).
-----------------------------------------------------------------------------
Used functions: cpp_get_chr, cpp_put_back_chr
Parameters:	--
Return value:	--
Exitcode:	EXITCODE_WRONG_CALL
---------------------------------------------------------------------------*/
static void scan_macro_args( void )
{
THIS_FUNC(scan_macro_args)
   int c ;
   char* arg ;
   char** args_poi ;

   args_poi = arg_list ;
   arg_list [0] = NULL ; /*scope: internal*/
   arg_count = 0 ;       /*scope: internal*/

   c = cpp_get_chr() ;
   if (c != '(') {
      fprintf( stderr, "wrong call to \"scan_macro_args\"\n" ) ;
      exit( EXITCODE_WRONG_CALL ) ;
   }

   do {
      arg = scan_macro_arg() ;
      *args_poi++ = strdup( arg ) ;
      *args_poi = NULL ;
      arg_count++ ;
      PRT_VAR(arg,s)
      c = cpp_get_chr() ;
      if (c == ')') {
         break ;
      }
      if (c != ',') {
         fprintf( stderr, "unexpected error\n" ) ;
         exit( EXITCODE_UNEXPECTED_ERR ) ;
      }
      if (arg_count == MAX_ARGS) {
         fprintf( stderr, "too many arguments\n" ) ;
         exit( EXITCODE_TABLE_FULL ) ;
      }
   } while (1) ;
   PRT_VAR(arg_count,u)
}

/*-------------------->   scan_macro_arg   <------------------ 2003-Aug-22 --
This function scans one argument of a macro.
-----------------------------------------------------------------------------
Used functions: cpp_get_chr, cpp_put_back_chr, remove_trailing_whites
Parameters:	--
Return value:	scanned arg, null-terminated, static
Exitcode:	--
---------------------------------------------------------------------------*/
static char* scan_macro_arg( void )
{
THIS_FUNC(scan_macro_arg)
static char arg_scan [ MAX_ARG_LEN +1 ] ; /*plus one for the trailing \0*/
   int c ;
   char* poi ;
   char* temp ;
   BIT not_ready ;
   BIT dec_par_lev ;
   S8 parenthesis_level ;

   poi = arg_scan ;
   parenthesis_level = 0 ;
   dec_par_lev = FALSE ;
   skip_whites() ;

   not_ready = TRUE ;
   do {
      c = cpp_get_chr() ;
      switch (c) {
      case '\"':
         cpp_put_back_chr( c ) ;
         temp = scan_string() ;
         strcpy( poi, temp ) ;
         poi += strlen( temp ) ;
         break ;

      case '\'':
         cpp_put_back_chr( c ) ;
         temp = scan_chr() ;
         strcpy( poi, temp ) ;
         poi += strlen( temp ) ;
         break ;

      case '\\':
         *poi++ = c ;
         *poi++ = cpp_get_chr() ;
         break ;

      case '(':
         parenthesis_level++ ; /*CHECK  FOR  OVERFLOW*/
         *poi++ = c ;
         break ;


      case ')':
         dec_par_lev = TRUE ;
      case ',':
         if (parenthesis_level == 0) { /*end of parameter*/
            cpp_put_back_chr( c ) ;
            *poi = '\0' ;
            not_ready = FALSE ;
         }
         else { /*',' or ')' not on lowest parenthesis_level => cont. arg*/
            *poi++ = c ;
         }

         if (dec_par_lev) {
            parenthesis_level-- ;
            dec_par_lev = FALSE ;
         }
         break ;

      default:
         *poi++ = c ;
      }
   } while (not_ready) ;

   remove_trailing_whites( arg_scan ) ;
   return arg_scan ;
}

/*-------------------->   scan_macro_body   <----------------- 2003-Oct-31 --
The function returns a precompiled macro body with all arguments
from "args" translated to the form $(nnn).
And all '$' translated to '$$'.

This function scans to the end of line, excluding the '\n' character.
A backslash at the end of line, however, masks the following '\n' and does
*not* stop the function, i. e. it will continue with the next line.
Both the backslash and the eol character are omitted and *not* copied to
the returned string.  That means the returned string will never contain
a newline character.  In case of a backslash/newline pair, however, a single
newline is copied to the output file to keep track of the line numbers!

The Function returns NULL if an eol wasn't found before the internal buffer
overflows or if an EOF is encountered.
-----------------------------------------------------------------------------
Used functions: cpp_get_chr, cpp_put_back_chr
Parameters:	--
Return value:	scanned string, null-terminated, static
                or NULL if an eol wasn't found
Exitcode:	--
---------------------------------------------------------------------------*/
static char* scan_macro_body( void )
{
THIS_FUNC(scan_macro_body)
static char precompiled_body [ 1000 +1 ] ; /*plus one for the trailing \0*/
   char* poi ;
   int cc ;
   BIT flag_backslash ;
   char* temp ;
   U8 i ; /*loop counter thru the args*/
   char arg_placeholder [7] ; /*$(nnn)\0*/

   ENTRY
   poi = precompiled_body ;
   flag_backslash = FALSE ;
   while (1) {
      cc = cpp_get_chr() ;
      if (cc == EOF) {
         cpp_put_back_chr( cc ) ;
         EXIT
         return NULL ;
      }

      if (flag_backslash) {
         if (cc == '\n') {
            fputc( cc, outfile ) ; /*keep track of line numbers!*/
         }
         else {
            *poi++ = '\\' ;
            *poi++ = cc ;
         }
         flag_backslash = FALSE ;
      }
      else {
         if (cc == '\\') {
            flag_backslash = TRUE ;
         }
         else {
            switch (cc) {
            case '\n':
               cpp_put_back_chr( cc ) ;
               *poi = '\0' ;
               EXIT
               return precompiled_body ;

            case '\"':
               cpp_put_back_chr( cc ) ;
               temp = scan_string() ;
               strcpy( poi, temp ) ;
               poi += strlen( temp ) ;
               break ;

            case '\'':
               cpp_put_back_chr( cc ) ;
               temp = scan_chr() ;
               strcpy( poi, temp ) ;
               poi += strlen( temp ) ;
               break ;

            default:
               if (flag_is_letter_or_underscore) {
                  cpp_put_back_chr( cc ) ;
                  temp = scan_identifier() ;

                     /*search id in arg_list*/
                  for ( i = 0 ; i < arg_count ; i++ ) {
                     if (strcmp( temp, arg_list [i] ) == 0) {
                        sprintf( arg_placeholder, "$(%u)", (int)i ) ;
                        PRT_VAR(arg_placeholder,s)
                        temp = arg_placeholder ;
                        break ;
                     }
                  }
                  strcpy( poi, temp ) ;
                  poi += strlen( temp ) ;
               }
               else { /*just copy everything that isn't an identifier*/
                  *poi++ = cc ;
               }
            } /*switch (cc)*/
         } /*wasn't a backslash*/
      } /*wasn't a backslash prefix*/

      if (cc == '$') {
         *poi++ = cc ; /*output '$' twice, 2nd time here, 1st time above*/
      }

      if (poi >= precompiled_body + 999) {
         EXIT
         return NULL ;
      }
   } /*outer while loop (scans body)*/
}

/*-------------------->   remove_trailing_whites   <---------- 2003-Aug-22 --
This function does what its name tells.
-----------------------------------------------------------------------------
Used functions: strlen
Parameters:	--
Return value:	--
Exitcode:	--
---------------------------------------------------------------------------*/
static void remove_trailing_whites( char* str )
{
THIS_FUNC(remove_trailing_whites)
   char* poi ;

   poi = str + strlen( str ) -1 ;
   while (((*poi == ' ') || (*poi == '\t')) && (poi >= str)) {
      *poi-- = '\0' ;
   }
}

/*-------------------->   skip_whites_to_eol   <-------------- 2003-Aug-24 --
This function skips blanks and tabs.  It expects nothing but whites until
the end of line.  In case it finds a non-white character, it prints a warning
to stderr but skips everything until eol nevertheless.

The final \n is put back into the put_back queue for evaluation by the
main loop.
-----------------------------------------------------------------------------
Used functions: skip_whites, cpp_get_chr, cpp_put_back_chr, scan_to_eol
Parameters:	--
Return value:	--
Exitcode:	--
---------------------------------------------------------------------------*/
static void skip_whites_to_eol( void )
{
THIS_FUNC(skip_whites_to_eol)
   int c ;
   char* the_rest ;

   skip_whites() ;
   c = cpp_get_chr() ;
   cpp_put_back_chr( c ) ;
   if (c != '\n') {
      the_rest = scan_to_eol() ;
      fprintf( stderr,
         "Warning: expected whites to end of line.  Found: >%s<\n",
         the_rest ) ;
   }
}
/***************************************************************************/
