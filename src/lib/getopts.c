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
getopts.c

This lib function scans the command line.
It puts options in global variables and modifies argc/argv.
*****************************************************************************
History: (latest change first)
2013-Feb-25: added "fprint_argv"
2013-Feb-22: extended test output
2013-Feb-21: - ported to new template
             - preserve single '-' as stdin symbol
1996-Apr-03..05: initial version
*****************************************************************************
Global objects:
- int getopts( int optc, char** optv )
- void fprint_argv( FILE* fp, int argc, char** argv )
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define TEST*/ /*generates main for test purposes*/



/*--  include files  ------------------------------------------------------*/

#include <misc.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopts.h>
#include <ctype.h>

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/

/*static void search_opt_in_list() ;*/
static struct optdef* search_opt_in_list( char chr, struct optdef* list ) ;


/*--  macros  -------------------------------------------------------------*/


/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/

static int optc ;
static char** optv ;
static char*  optstr ; /*Zeiger auf (Teil-)string eines optv*/


/*--  library functions  --------------------------------------------------*/

/*-------------------->   getopts   <---------------------------- 2013-Feb-22
This function interprets argc/argv according to 'definition_list'.
Get_opts hides the 'used' arguments from the calling function by modifying
argc and argv.
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters:     - argc_poi, argv_poi
                - definition_list    array of struct optdef
Return value:   TRUE if error, else FALSE
Exitcode:       --
---------------------------------------------------------------------------*/
int getopts( int* argc_poi, char*** argv_poi, struct optdef* definition_list)
{
THIS_FUNC(get_opts)
  struct optdef* p = definition_list ;
  char optchr ;


  optc = *argc_poi ;
  optv = *argv_poi ;

  while (p->optchar) { /* initialize all flags */
    *(p->optflag) = FALSE ;
    p++ ;
  }

  while (optv++, --optc) {
    optstr = *optv ;
    if (    (*optstr != '-')
         || (optstr [1] == '\0') /*stdin argument*/
       ) {
      break ; /*end of options*/
    }

    while ( (optchr = (*++optstr)) ) {
                    /*many options may queue after one single '-'*/
      if (search_opt_in_list( optchr, definition_list ) == NULL) {
        return TRUE ; /*invalid option*/
      }
    }
  }
  *argc_poi = optc ;
  *argv_poi = optv ;
  return FALSE ; /* ok */
}

/*-------------------->   search_opt_in_list   <----------------- 1996-Apr-05
This function searches an option letter in the definition list
and sets the corresponding flag to TRUE.
-----------------------------------------------------------------------------
Used functions: isdigit, strtoul
Globals:   x
Internals: anchor   r/o
           p        r/w  set to NULL upon error, else points to optdef
           optchr   r/o
Parameters:     --
Return value:   NULL upon error, else points into list
Exitcode:       x
---------------------------------------------------------------------------*/
static struct optdef* search_opt_in_list( char chr, struct optdef* list )
{
THIS_FUNC(search_opt_in_list)
  struct optdef* p = list ;
  /*struct optdef* ret_val ;*/

  while (p->optchar) {
    if (chr == p->optchar) { /*gefunden*/
      if (*(p->optflag)) {/*Option zweimal angegeben!*/
        return NULL ;
      }
      *(p->optflag) = TRUE ;
      if (p->optarg != NULL) { /*Arg holen*/
        if (*++optstr) {
          *(p->optarg) = optstr ;
        }
        else if (optv++, --optc) {
          *(p->optarg) = *optv ;
        }
        else { /*OptArg fehlt*/
          return NULL ;
        }
        optstr = " " ; /*simuliertes Ende*/
      }
      return p ;
    }

    else if (isdigit(chr) && (p->optchar == OPT_NUMBER)) {
      if (*(p->optflag)) {/*Option zweimal angegeben!*/
        return NULL ;
      }
      *(p->optflag) = TRUE ;
      *((unsigned long*)(p->optarg)) =
        strtoul( optstr, &optstr, 0 ) ;
      optstr-- ;
      return p ;
    }
    p++ ;
  } /*end while*/

  /*Option ist nicht in der Liste -- ungueltig*/
  return NULL ;
}

/*-------------------->   fprint_argv   <------------------------ 2013-Feb-25
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
void fprint_argv( FILE* fp, int argc, char** argv )
{
THIS_FUNC(fprint_argv)
  int i ; /*loop control*/


  for ( i = 0 ; i < argc ; i++ ) {
    fprintf( fp, "<%s>", argv [i] ) ;
  }
  fprintf( fp, "\n" ) ;
}


/*--  test functions  -----------------------------------------------------*/

#ifdef TEST
/* compile with "-DTEST" to get an executable file for testing "getopts.c" */
/* run with (e.g.):
	getopts -v
	getopts
	getopts abc par
	getopts -123 -o file -v abc par
*/

#include <process.h>
static void print_args( int argc, char** argv) ;

static char flag_outfile, flag_count, flag_verbose ;
static char *outfile_name ;
static unsigned long count ;

/*static struct optdef optlist [] = {
	{ 'o', &flag_outfile, &outfile_name },
	{ OPT_NUMBER, &flag_count, (char**)&count },
	{ 'v', &flag_verbose, NO_ARG },
	{ '\0', NULL, NO_ARG }
} ;*/
OPTION_LIST(optlist)
 OPTION_W_ARG('o',flag_outfile,outfile_name)
 OPTION_NUMBER(flag_count,count)
 OPTION_WO_ARG('v',flag_verbose)
OPTION_LIST_END

void main( int argc, char** argv)
{
  print_args( argc, argv ) ;
  if (getopts( &argc,
               &argv,
               optlist )) {
    fprintf( stderr, "usage: ...\n" ) ;
    exit( 1 ) ;
  }
  if (flag_verbose)
    printf( "Option: verbose\n" ) ;
  if (flag_outfile)
    printf( "Option: outfile=%s\n", outfile_name ) ;
  if (flag_count)
    printf( "Option: count=%lu\n", count ) ;
  printf( "Arguments after 'get_opts': " ) ;
  print_args( argc, argv ) ;
}

static void print_args( int argc, char** argv)
{
  while (argc--) {
    printf( "<%s> ", *argv++) ;
  }
  printf( "\n" ) ;
}
#endif /* TEST */

/*-------------------->   x   <---------------------------------- 2013-Feb-25
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
