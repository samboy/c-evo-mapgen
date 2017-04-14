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
scan.c

This library contains functions which scan thru callback functions.
*****************************************************************************
History: (latest change first)
2014-Oct-04..Nov-20: changes for Linux compatibility
2014-Jan-25: added scan_double()
2013-Nov-08..09: - added scan_while_charset(), scan_until_charset()
                 - implemented SCAN_STORE_TOKEN
2012-Nov-25..2013-Sep-30: initial version
*****************************************************************************
Global objects:
- 

Common parameters: SCAN_PARAMS* par
Return value: TRUE if token_len == 0
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define TEST*/ /*generates main for test*/



/*--  include files  ------------------------------------------------------*/

#include <scan.h>
#include <falloc.h>
/*#include <stdlib.h>*/
#include <string.h> /*for strchr()*/
#include <ctype.h> /*for is*() functions*/
#include <math.h> /*for pow()*/

/*#define DEBUG*/
#include <debug.h>


/*--  constants  ----------------------------------------------------------*/

/*#define*/


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/

static void  pre_scan( SCAN_PARAMS* par ) ;
static  BIT post_scan( SCAN_PARAMS* par ) ;
static void put_dest_chr( SCAN_PARAMS* this, char c ) ;


/*--  macros  -------------------------------------------------------------*/

#define GET_NEXT_CHR  \
  if (par->putback_in_use) {  \
    chr = par->putback_chr ;  \
    par->putback_in_use = FALSE ;  \
  }  \
  else {  \
    chr = (*(par->get))( par ) ;  \
  }  \
  par->scan_len++ ;

#define PUT_BACK_CHR  \
  ASSERT( ! par->putback_in_use)  \
  par->putback_chr = chr ;  \
  par->putback_in_use = TRUE ;  \
  par->scan_len-- ;

#define PUT_DEST_CHR     put_dest_chr( par, chr ) ;


/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/

#define COMMON_BUF_LEN  100
static char buf [ COMMON_BUF_LEN ] ; /*common buffer if no heap wanted*/


/*--  library functions  --------------------------------------------------*/

/*-------------------->   scan_new   <--------------------------- 2013-Sep-29
This function allocates and initializes a new SCAN_PARAMS object for use
with scan_* functions.
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters:	- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
SCAN_PARAMS* scan_new( SCAN_GET_FUNC_P func )
{
THIS_FUNC(scan_new)
  SCAN_PARAMS* ret_val ;


  /*ENTRY*/
  ret_val = (SCAN_PARAMS*)falloc( sizeof( SCAN_PARAMS ) ) ;
  scan_init( ret_val, func ) ;
  return ret_val ;
}

/*-------------------->   scan_init   <-------------------------- 2013-Nov-09
This function (re-)initializes an existing SCAN_PARAMS object.
-----------------------------------------------------------------------------
Used functions: --
Globals:   --
Internals: --
Parameters:  - this   pointer to SCAN_PARAMS
             - func   pointer to function which returns the next character
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
void scan_init( SCAN_PARAMS* this, SCAN_GET_FUNC_P func )
{
THIS_FUNC(scan_init)
  this->get = func ;
  this->putback_in_use = FALSE ;
  this->flags = SCAN_EXIT_ON_ERROR | SCAN_STORE_TOKEN ; /*default*/
}

/*-------------------->   scan_from_mem   <---------------------- 2013-Sep-28
This function serves as a callback function for scan_* functions
in case characters come from memory.
The "p" field of SCAN_PARAMS must hold an character pointer into mem.
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
char scan_from_mem( SCAN_PARAMS* par )
{
THIS_FUNC(scan_from_mem)
  /*PRT_VAR(*(par->p),c)*/
  return *((par->p)++) ;
}

/*-------------------->   scan_from_file   <--------------------- 2013-Sep-28
This function serves as a callback function for scan_* functions
in case characters come from a file.
The "fp" field of SCAN_PARAMS must hold an open FILE pointer.
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
char scan_from_file( SCAN_PARAMS* par )
{
THIS_FUNC(scan_from_file)
  int c ;


  /*ENTRY*/
  c = fgetc( par->fp )  ;
  return (c == EOF) ? '\0' : (char)c ;
}

/*-------------------->   scan_peep_next_chr   <----------------- 2013-Sep-29
This function returns the next character and puts it back immediately.
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters:	- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
char scan_peep_next_chr( SCAN_PARAMS* par )
{
THIS_FUNC(scan_peep_next_chr)
  char chr ; /*scratchpad for one character*/


  GET_NEXT_CHR
  PUT_BACK_CHR
  return chr ;
}

/*-------------------->   scan_get_next_chr   <------------------ 2013-Sep-29
This function returns the next character from the input stream.
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters:	- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
char scan_get_next_chr( SCAN_PARAMS* par )
{
THIS_FUNC(scan_get_next_chr)
  char chr ; /*scratchpad for one character*/


  GET_NEXT_CHR
  return chr ;
}

/*-------------------->   pre_scan   <--------------------------- 2013-Sep-30
This function is called internally before tokens are scanned.
-----------------------------------------------------------------------------
Used functions: --
Globals:   --
Internals: --
Parameters:	- par     pointer to SCAN_PARAMS
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
static void pre_scan( SCAN_PARAMS* par )
{
THIS_FUNC(pre_scan)
  char chr ; /*scratchpad for one character*/


  /*PRT_VAR(par->p,s)*/
  par->scan_stat = 0x0000 ; /*default: nothing to report*/
  par->scan_len = 0 ;
  par->token_len = 0 ;
  par->flags &= ~_SCAN_ALLOW_EMPTY_STRING ; /*default*/

  if (par->dest == NULL) {
    par->dest = buf ;
    par->dest_size = COMMON_BUF_LEN ;
  }
  par->dest_invalid = par->dest + par->dest_size ;
  par->remember_dest = par->dest ;
  /*PRT_VAR((unsigned long)(par->dest),08lx)*/
  /*PRT_VAR((unsigned long)(par->dest_invalid),08lx)*/

  if (par->flags & SCAN_SKIP_LEADING_WHITES) {
    /*PRT_VAR((unsigned long)(par->p),08lx)*/
    do {
      GET_NEXT_CHR
      /*fprintf( stderr, "%s: c=>%c< (0x%02x)\n", _this_func, chr, chr ) ;*/
    } while ((chr == ' ') || (chr == '\t')) ;
    /*fprintf( stderr, "%s: putting back >%c<\n", _this_func, chr ) ;*/
    PUT_BACK_CHR
  }
  /*EXIT*/
}

/*-------------------->   post_scan   <-------------------------- 2013-Sep-30
This function is called internally after tokens are scanned.
-----------------------------------------------------------------------------
Used functions: --
Globals:   --
Internals: --
Parameters:	- par     pointer to SCAN_PARAMS
Return value:	TRUE if scan failed (nothing scanned, except whites)
Exitcode:	--
---------------------------------------------------------------------------*/
static BIT post_scan( SCAN_PARAMS* par )
{
THIS_FUNC(post_scan)
  if (    (par->token_len == 0)
       && ((par->flags & _SCAN_ALLOW_EMPTY_STRING) == 0)
     ) {
    par->scan_stat |= SCANSTAT_NOTHING_SCANNED ;
    if (par->flags & SCAN_EXIT_ON_ERROR) {
      fprintf( stderr, "scan: no token found\n" ) ;
      exit( EXITCODE_SYNTAX_ERR ) ;
    }
    return TRUE ; /*nothing scanned*/
  }
  par->dest = par->remember_dest ; /*restore for next scan*/
  return FALSE ; /*scan ok*/
}

/*-------------------->   put_dest_chr   <----------------------- 2013-Nov-09
This function puts one character to the destination field (somewhere in
memory).
"Token_len" is incremented.
-----------------------------------------------------------------------------
Used functions: fprintf, exit
Globals:   --
Internals: --
Parameters:	- SCAN_PARAMS* this
		- c    character to put
Return value:	void
Exitcode:	EXITCODE_TABLE_FULL
---------------------------------------------------------------------------*/
static void put_dest_chr( SCAN_PARAMS* this, char c )
{
THIS_FUNC(put_dest_chr)
  /*PRT_VAR((unsigned long)(this->dest),08lx)*/
  /*PRT_VAR(c,c)*/
  if (this->flags & SCAN_STORE_TOKEN) {
    if (this->dest == this->dest_invalid) {
      *(this->dest -1) = '\0' ; /*prepare to output*/
      fprintf( stderr, "scan: buffer too short\n"
                       "scanned so far: %s\n", this->remember_dest ) ;
      exit( EXITCODE_TABLE_FULL ) ;
    }
    *(this->dest)++ = c ;
  }
  this->token_len++ ;
}

/*-------------------->   scan_check_str   <--------------------- 2013-Sep-30
This function checks the next character sequence from input.
It returns TRUE if the character sequence does not correspond to "str".
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters:	- x
		- x
Return value:	FALSE if sequence is as expected
Exitcode:	EXITCODE_SYNTAX_ERR
---------------------------------------------------------------------------*/
BIT scan_check_str( SCAN_PARAMS* par, char* str )
{
THIS_FUNC(scan_check_str)
  char* p = str ;
  char chr ; /*scratchpad for one character*/


  pre_scan( par ) ;

  while (*p != '\0') {
    GET_NEXT_CHR
    PUT_DEST_CHR
    if (chr != *p) {
      if (par->flags & SCAN_EXIT_ON_ERROR) {
        *(par->dest) = '\0' ; /*prepare buf for output*/
        fprintf( stderr,
"scan_check_str: expected: \"%s\"\n"
"                   found: \"%s\"\n", str, par->remember_dest ) ;
        exit( EXITCODE_SYNTAX_ERR ) ;
      }
      par->dest = par->remember_dest ;
      return TRUE ; /*strings different*/
    }
    p++ ;
  }
  par->dest = par->remember_dest ;
  return FALSE ; /*ok*/
}

/*-------------------->   scan_while_charset   <----------------- 2013-Nov-09
This function scans while the characters are in "charset".
-----------------------------------------------------------------------------
Used functions: x
Globals:   --
Internals: --
Parameters:	- SCAN_PARAMS*
                - charset  null-terminated string specifying the set of
		           characters to scan
Return value:	TRUE if no characters scanned
Exitcode:	--
---------------------------------------------------------------------------*/
BIT scan_while_charset( SCAN_PARAMS* par, char* charset )
{
THIS_FUNC(scan_while_charset)
  char chr ; /*scratchpad for one character*/
  BIT ongoing = TRUE ; /*default*/


  pre_scan( par ) ;

  while (ongoing) {
    GET_NEXT_CHR
    if (strchr( charset, (int)chr ) == NULL) { /*not in charset*/
      ongoing = FALSE ;
      PUT_BACK_CHR
      put_dest_chr( par, '\0' ) ; /*increments "token_len"*/
      par->token_len-- ;
    }
    else {
      PUT_DEST_CHR /*increments "token_len"*/
    }
  }
  return post_scan( par ) ;
}

/*-------------------->   scan_until_charset   <----------------- 2013-Nov-09
This function scans while the characters are  *not*  in "charset".
-----------------------------------------------------------------------------
Used functions: x
Globals:   --
Internals: --
Parameters:	- SCAN_PARAMS*
                - charset  null-terminated string specifying the set of
		           characters to skip
Return value:	TRUE if no characters skipped
Exitcode:	--
---------------------------------------------------------------------------*/
BIT scan_until_charset( SCAN_PARAMS* par, char* charset )
{
THIS_FUNC(scan_until_charset)
  char chr ; /*scratchpad for one character*/
  BIT ongoing = TRUE ; /*default*/


  pre_scan( par ) ;

  while (ongoing) {
    GET_NEXT_CHR
    if (strchr( charset, (int)chr ) != NULL) { /*in charset*/
      ongoing = FALSE ;
      PUT_BACK_CHR
      put_dest_chr( par, '\0' ) ; /*increments "token_len"*/
      par->token_len-- ;
    }
    else {
      PUT_DEST_CHR /*increments "token_len"*/
    }
  }
  return post_scan( par ) ;
}

/*-------------------->   scan_identifier   <-------------------- 2013-Sep-29
This function scans an identifier.
Valid examples: abc, def23, gh_i, jkl_, _0mno
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters:	- par  SCAN_PARAMS*
Return value:	TRUE if scan failed
Exitcode:	x
---------------------------------------------------------------------------*/
BIT scan_identifier( SCAN_PARAMS* par )
{
THIS_FUNC(scan_identifier)
  char chr ; /*scratchpad for one character*/
  BIT ongoing = TRUE ; /*default*/


  /*ENTRY*/
  PRT_VAR((unsigned)(par->flags),04x)
  pre_scan( par ) ;

     /*read first character*/
  GET_NEXT_CHR
  if (isalpha( chr ) || (chr == '_')) {
    put_dest_chr( par, chr ) ;

       /*read other characters*/
    while (ongoing) {
      GET_NEXT_CHR
      if (isalnum( chr ) || (chr == '_')) {
        put_dest_chr( par, chr ) ;
      }
      else {
        PUT_BACK_CHR
        ongoing = FALSE ;
        put_dest_chr( par, '\0' ) ;
        par->token_len-- ;
      }
    } /*end while*/
  }

  else { /*first char was not alpha*/
    PUT_BACK_CHR
  }

  return post_scan( par ) ;
}

/*-------------------->   scan_U32   <--------------------------- 2013-Sep-30
This function scans an unsigned number.
Hex numbers must  *not*  start with "0x".  Upper or lower case letters are
allowed.

Problem: Should "011014" be a legal input for a binary scan (result: 13d)
or should an error be signalled (illegal '4') ?
Similarly, should "faw" be a legal input for a hex scan (result: 250d) ?
Currently, these examples are valid!
-----------------------------------------------------------------------------
Used functions:
Globals:   --
Internals: --
Parameters:	- par     pointer to SCAN_PARAMS
Return value:	TRUE if scan failed
Exitcode:	EXITCODE_SYNTAX_ERR
---------------------------------------------------------------------------*/
BIT scan_U32( SCAN_PARAMS* par )
{
THIS_FUNC(scan_U32)
  char chr ; /*scratchpad for one character*/
  char lower_case ; /*modified chr*/
  U32 digit_value ; /*U8 would be sufficient, but U32 needs no type cast*/
  U32 radix = (U32)(par->radix) ; /*avoid type casts inside loop*/
  BIT ongoing = TRUE ; /*loop control*/


  /*printf( "scan_U32: >%s<\n", par->p ) ;*/
     /*check parameter range*/
  if (    (radix > 36) /*the A..Z extension would fail*/
       || (radix < 2)) {
    fprintf( stderr, "scan_U32: radix = %u\n", (unsigned)(radix) ) ;
    exit( EXITCODE_WRONG_PARAM ) ;
  }

  pre_scan( par ) ;


  par->number = 0 ; /*initial value*/
  while (ongoing) {
    GET_NEXT_CHR
    par->token_len++ ;
    /*PRT_VAR(chr,c)*/
    if ((chr >= '0') && (chr <= '9')) {
      digit_value = chr - '0' ;
    }
    else {
      lower_case = (chr | 0x20) ; /*upper case -> lower case*/
      if ((lower_case >= 'a') && (lower_case <= 'z')) {
        digit_value = lower_case - 'a' + 10 ;
      }
      else {
        ongoing = FALSE ;
        /*printf( "putting back '%c'\n", chr ) ;*/
        PUT_BACK_CHR
        par->token_len-- ;
      }
    }

    if (ongoing) {
      par->number = par->radix * par->number + digit_value ;
    }
  } /*end digit while loop*/

  return post_scan( par ) ;
}

/*-------------------->   scan_double   <------------------------ 2014-Nov-20
This function scans a floating point number number.
Radix for floating point numbers is always 10.

Valid examples:
1   1.   .1  1.23  12.34  001.23   1.23e3  1.23e+3  1.23e-3   1.23E3  123E5

Problem: Should "1.23." or "1.23e3e" be a legal input?
Currently, these examples are valid!
-----------------------------------------------------------------------------
Used functions:
Globals:   --
Internals: --
Parameters:	- par     pointer to SCAN_PARAMS
Return value:	TRUE if scan failed
Exitcode:	EXITCODE_SYNTAX_ERR
---------------------------------------------------------------------------*/
BIT scan_double( SCAN_PARAMS* par )
{
THIS_FUNC(scan_double)
  double multiplier ;
  char chr ; /*scratchpad for one character*/
  //char lower_case ; /*modified chr*/
  U32 digit_value ; /*U8 would be sufficient, but U32 needs no type cast*/
  //BIT ongoing = TRUE ; /*loop control*/
  //BIT found_decimal_point = FALSE ; /*default*/
  //BIT found_E = FALSE ; /*default*/
  BIT exp_sign ; /*FALSE means exponent is positive*/
  enum phase { PRE_DOT, POST_DOT, EXPONENT, FINISHED } ;
  enum phase phase ;


  /*printf( "scan_double: >%s<\n", par->p ) ;*/
  pre_scan( par ) ;


  par->real = 0.0 ; /*initial value*/
  par->number = 0 ; /*initial value, integer copy*/
  phase = PRE_DOT ;
  while (phase != FINISHED) {
    GET_NEXT_CHR
    par->token_len++ ;
    /*PRT_VAR(chr,c)*/
    if ((chr >= '0') && (chr <= '9')) {
      digit_value = chr - '0' ;
      switch (phase) {
      case PRE_DOT:
        par->number = 10 * par->number + digit_value ; /*integer copy*/
        par->real = 10.0 * par->real   + (double)digit_value ;
        break ;

      case POST_DOT:
        par->real += multiplier * (double)digit_value ;
        multiplier = 0.1 * multiplier ; /*for next digit*/
        break ;

      case EXPONENT:
        par->number = 10 * par->number + digit_value ;
        break ;

      default:
        fprintf( stderr, "scan_double: internal error\n" ) ;
        exit( EXITCODE_UNEXPECTED_ERR ) ;
      }
    }
    else if (chr == '.') {
      if (phase == PRE_DOT) {
        phase = POST_DOT ;
        multiplier = 0.1 ; /*for next digit*/
        continue ; /*read mantissa digits*/
      }
      else {
        /*phase = FINISHED ;*/
        break ;
      }
    }
    else if ((chr == 'e') || (chr == 'E')) {
      if (phase != EXPONENT) {
        phase = EXPONENT ;
        exp_sign = FALSE ; /*default: positive*/

           /*read optional sign*/
        GET_NEXT_CHR
        par->token_len++ ;
        if (chr == '+') {
          /*continue ;*/
        }
        else if (chr == '-') {
          exp_sign = TRUE ; /*negative exponent*/
        }
        else { /*1st exponent digit  or  end of floating point number*/
          PUT_BACK_CHR
          par->token_len-- ;
        }

        par->number = 0 ; /*use "number" for exponent value*/
        continue ; /*read exponent digits*/
      }
      else {
        /*phase = FINISHED ;*/
        break ;
      }
    }
    else {
      if (phase == EXPONENT) {
        if (exp_sign) { /*neg*/
          par->real /= pow( 10.0, (double)(par->number) ) ;
        }
        else { /*pos*/
          par->real *= pow( 10.0, (double)(par->number) ) ;
        }
      }
      /*phase = FINISHED ;*/
      break ;
    }
  } /*end digit while loop*/

  /*printf( "putting back '%c'\n", chr ) ;*/
  PUT_BACK_CHR
  par->token_len-- ;

  return post_scan( par ) ;
}

/*-------------------->   scan_string   <------------------------ 2013-Sep-30
This function scans a string and copies it to "dest".
If the string begins with a double quote ("), a scan is done behind the
trailing double quote.
If it begins with another character, a scan is done until
a white character or until a '\0'.
If SCAN_NL_TERMINATES is given, the scan stops at a newline, too.
Leading and trailing double quotes (if any) are not copied to "dest".
Backslash sequences are treated in 'C style', except when
SCAN_BACKSLASH_VERBATIM is given.
-----------------------------------------------------------------------------
Used functions: falloc, strncpy, fprintf, exit
Globals:   --
Internals: --
Parameters:
- flags
  SCAN_EXIT_ON_ERROR -- exit on any error
  SCAN_BACKSLASH_VERBATIM
  SCAN_NL_TERMINATES -- a newline stops the string
  SCAN_ACCEPT_INCOMPLETE -- don't complain about missing " at end
Return value:	TRUE if scan failed
Exitcode:	SCAN_EXIT_ON_ERROR
---------------------------------------------------------------------------*/
BIT scan_string( SCAN_PARAMS* par )
{
THIS_FUNC(scan_string)
  char chr ; /*scratchpad for one character*/
  BIT ongoing = TRUE ; /*while loop control*/


  /*ENTRY*/
  /*PRT_VAR(par->p,s)*/
  pre_scan( par ) ;

  GET_NEXT_CHR
  /*PRT_VAR(chr,c)*/
  if (chr == '"') { /*string is enclosed by double quotes*/
    par->flags |= _SCAN_ALLOW_EMPTY_STRING ;
    while (ongoing) {
      GET_NEXT_CHR
      /*PRT_VAR(chr,c)*/

      switch (chr) {
      case '"':
        ongoing = FALSE ;
        break ;

      case '\n':
        ongoing = FALSE ;
        if (par->flags & SCAN_NL_TERMINATES) { /*newline allowed*/
          break ;
        }
        /*newline not allowed, fall thru to case \0*/

      case '\0':
        ongoing = FALSE ;
        par->scan_stat |= SCANSTAT_INCOMPLETE ;
        if (par->flags & SCAN_EXIT_ON_ERROR) {
          fprintf( stderr, "scan_string: missing double quote\n" ) ;
          exit( EXITCODE_SYNTAX_ERR ) ;
        }
        break ;

      case '\\':
        GET_NEXT_CHR
        /*PRT_VAR(chr,c)*/
        /*fall thru to default*/

      default:
        put_dest_chr( par, chr ) ;
      } /*end switch*/
    } /*end while*/
  } /*end: string is enclosed by double quotes*/


  else { /*string is not enclosed by double quotes*/
    put_dest_chr( par, chr ) ;

    while (ongoing) {
      GET_NEXT_CHR
      /*PRT_VAR(chr,c)*/
      switch (chr) {
      case ' ':
      case '\t':
      case '\n':
      case '\0':
        ongoing = FALSE ;
        PUT_BACK_CHR
        break ;

      case '\\':
        GET_NEXT_CHR
        /*PRT_VAR(chr,c)*/
        /*fall thru to default*/

      default:
        put_dest_chr( par, chr ) ;
      }
    } /*end while*/
  } /*end: string is not enclosed by double quotes*/

  put_dest_chr( par, '\0' ) ;
  par->token_len-- ;

  /*EXIT*/
  return post_scan( par ) ;
}

/*--  test functions  -----------------------------------------------------*/

#ifdef TEST

#include <string.h> /*for strcmp()*/
#include <openfile.h>
#ifdef __unix__
# include <unistd.h> /*for unlink()*/
#else
# include <io.h> /*for unlink()*/
#endif

void main( int argc, char** argv ) /*2015-Sep-22*/
{
THIS_FUNC(main)
  SCAN_PARAMS* p ;
  char mem [] = "  ab_c 23 xx" ; /*scan_identifier should return "ab_c"*/
  char string1 [] = " \"Hello world\" " ;
  char string2 [] = " \"string \\\" with double quote\"" ;
  char floating [] = "1 1. .1 1.23 12.34 001.23 1.23e3 1.23e+3 1.23E-3 123E5" ;
  char floating2 [] = "12.34" ;
  char dest [50] ;


  p = scan_new( scan_from_mem ) ;


/*----------  test reading identifier from mem  ----------------------------*/
  printf( "Test reading identifier\n" ) ;
  p->p = mem ;
  p->flags |= SCAN_SKIP_LEADING_WHITES ;
  p->dest = dest ;
  p->dest_size = 50 ;

  scan_identifier( p ) ;
  if (strcmp( dest, "ab_c" ) != 0) {
    printf( "unexpected: dest = \"%s\"\n", dest ) ;
    exit( 1 ) ;
  }
  if ( p->scan_stat != 0) {
    printf( "unexpected: scan_stat = 0x%04x\n", (unsigned)(p->scan_stat) ) ;
    exit( 1 ) ;
  }
  if ( p->scan_len != 6) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 4) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }


  scan_init( p, scan_from_mem ) ; /*clears putback buffer*/
  p->p = mem ;
  p->flags &= ~(SCAN_SKIP_LEADING_WHITES | SCAN_EXIT_ON_ERROR ) ;
  p->dest = dest ;

  scan_identifier( p ) ;
  if ( p->scan_stat != SCANSTAT_NOTHING_SCANNED) {
    printf( "unexpected: scan_stat = 0x%04x\n", (unsigned)(p->scan_stat) ) ;
    exit( 1 ) ;
  }

/*----------  test reading from file  --------------------------------------*/
  printf( "Test reading from file\n" ) ;
  p->fp = forced_fopen_w( "scan.$$$", OPENFILE_TEXT ) ;
  fprintf( p->fp, "_0mno" ) ;
  forced_fclose( p->fp ) ;

  scan_init( p, scan_from_file ) ; /*clears putback buffer*/
  p->fp = forced_fopen_rt( "scan.$$$" ) ;
  /*p->flags = SCAN_EXIT_ON_ERROR ;*/
  p->flags &= ~SCAN_SKIP_LEADING_WHITES ;
  p->dest = dest ;

  scan_identifier( p ) ;
  if (strcmp( dest, "_0mno" ) != 0) {
    printf( "unexpected: dest = \"%s\"\n", dest ) ;
    exit( 1 ) ;
  }

  forced_fclose( p->fp ) ;
  unlink( "scan.$$$" ) ;

/*----------  test reading number  -----------------------------------------*/
  printf( "Test reading number\n" ) ;
  scan_init( p, scan_from_mem ) ; /*clears putback buffer*/
  p->p =  & (mem [7]) ;
  /*p->flags = SCAN_EXIT_ON_ERROR | SCAN_SKIP_LEADING_WHITES ;*/
  /*p->flags = SCAN_SKIP_LEADING_WHITES ;*/
  p->flags |= SCAN_SKIP_LEADING_WHITES ;
  p->radix = 10 ;

  scan_U32( p ) ;
  if ( p->number != 23) {
    printf( "unexpected: number = %lu\n", (unsigned long)(p->number) ) ;
    exit( 1 ) ;
  }
  if ( p->scan_len != 2) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 2) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  p->flags &= ~SCAN_SKIP_LEADING_WHITES ;
  if (scan_check_str( p, " xx" )) {
    printf( "unexpected remaining string (expected: > xx<)\n" ) ;
    exit( 1 ) ;
  }

/*----------  test reading strings  ----------------------------------------*/
  printf( "Test reading strings\n" ) ;
  scan_init( p, scan_from_mem ) ; /*clears putback buffer*/
  p->p = mem ; /*string w/o double quotes*/
  p->dest = dest ;
  /*p->flags = SCAN_EXIT_ON_ERROR | SCAN_SKIP_LEADING_WHITES ;*/
  p->flags |= SCAN_SKIP_LEADING_WHITES ;

  scan_string( p ) ;
  if (strcmp( dest, "ab_c" ) != 0) {
    printf( "unexpected: dest = \"%s\"\n", dest ) ;
    exit( 1 ) ;
  }
  if ( p->scan_stat != 0) {
    printf( "unexpected: scan_stat = 0x%04x\n", (unsigned)(p->scan_stat) ) ;
    exit( 1 ) ;
  }
  if ( p->scan_len != 6) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 4) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }


  scan_init( p, scan_from_mem ) ; /*clears putback buffer AND FLAGS*/
  p->p = string1 ;
  p->dest = dest ;
  /*p->flags = SCAN_EXIT_ON_ERROR | SCAN_SKIP_LEADING_WHITES ;*/
  p->flags |= SCAN_SKIP_LEADING_WHITES ;

  scan_string( p ) ;
  if (strcmp( dest, "Hello world" ) != 0) {
    printf( "unexpected: dest = \"%s\"\n", dest ) ;
    exit( 1 ) ;
  }
  if ( p->scan_stat != 0) {
    printf( "unexpected: scan_stat = 0x%04x\n", (unsigned)(p->scan_stat) ) ;
    exit( 1 ) ;
  }
  if ( p->scan_len != 14) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 11) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }


  scan_init( p, scan_from_mem ) ; /*clears putback buffer AND FLAGS*/
  p->p = string2 ;
  p->dest = dest ;
  /*p->flags = SCAN_EXIT_ON_ERROR | SCAN_SKIP_LEADING_WHITES ;*/
  p->flags |= SCAN_SKIP_LEADING_WHITES ;

  scan_string( p ) ;
  if (strcmp( dest, "string \" with double quote" ) != 0) {
    printf( "unexpected: dest = \"%s\"\n", dest ) ;
    exit( 1 ) ;
  }
  if ( p->scan_stat != 0) {
    printf( "unexpected: scan_stat = 0x%04x\n", (unsigned)(p->scan_stat) ) ;
    exit( 1 ) ;
  }
  if ( p->scan_len != 30) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 26) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }


  scan_init( p, scan_from_mem ) ; /*clears putback buffer AND FLAGS*/
  p->p = mem ;
  p->dest = dest ;
  /*p->flags = SCAN_EXIT_ON_ERROR | SCAN_SKIP_LEADING_WHITES ;*/
  p->flags |= SCAN_SKIP_LEADING_WHITES ;

  scan_check_str( p, "ab" ) ;

  scan_init( p, scan_from_mem ) ; /*clears putback buffer AND FLAGS*/
  p->p = mem ;
  p->dest = dest ;
  p->flags = SCAN_SKIP_LEADING_WHITES ;

  if ( ! scan_check_str( p, "abc" )) {
    printf( "unexpected: abc != ab_c\n" ) ;
    exit( 1 ) ;
  }

/*----------  test charset routines  ---------------------------------------*/
  printf( "Test charset routines (1)\n" ) ;
  scan_init( p, scan_from_mem ) ; /*clears putback buffer*/
  p->p = string1 +4 ;
  p->dest = dest ;
  /*p->flags = SCAN_EXIT_ON_ERROR | SCAN_SKIP_LEADING_WHITES ;*/
  p->flags |= SCAN_SKIP_LEADING_WHITES ;

  scan_while_charset( p, "ow l" ) ;
  if (strcmp( dest, "llo wo" ) != 0) {
    printf( "unexpected: dest = \"%s\"\n", dest ) ;
    exit( 1 ) ;
  }
  if ( p->scan_stat != 0) {
    printf( "unexpected: scan_stat = 0x%04x\n", (unsigned)(p->scan_stat) ) ;
    exit( 1 ) ;
  }
  if ( p->scan_len != 6) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 6) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }


  printf( "Test charset routines (2)\n" ) ;
  scan_init( p, scan_from_mem ) ; /*clears putback buffer*/
  p->p = string1 ;
  p->dest = dest ;
  /*p->flags = SCAN_EXIT_ON_ERROR | SCAN_SKIP_LEADING_WHITES ;*/
  /*p->flags |= SCAN_SKIP_LEADING_WHITES ;*/

  scan_until_charset( p, "owl" ) ;
  if (strcmp( dest, " \"He" ) != 0) {
    printf( "unexpected: dest = \"%s\"\n", dest ) ;
    exit( 1 ) ;
  }
  if ( p->scan_stat != 0) {
    printf( "unexpected: scan_stat = 0x%04x\n", (unsigned)(p->scan_stat) ) ;
    exit( 1 ) ;
  }
  if ( p->scan_len != 4) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 4) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }


/*----------  test reading floating point numbers  -------------------------*/
  printf( "Test reading floating point numbers\n" ) ;
  scan_init( p, scan_from_mem ) ; /*clears putback buffer*/
  p->p = floating ;
  p->flags |= SCAN_SKIP_LEADING_WHITES ;

  scan_double( p ) ; /*"1"*/
  if ( p->scan_len != 1) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 1) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  if ( p->real != 1.0) {
    printf( "unexpected: double = %lf (expected 1.0)\n", p->real ) ;
    exit( 1 ) ;
  }

  scan_double( p ) ; /*" 1."*/
  if ( p->scan_len != 3) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 2) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  if ( p->real != 1.0) {
    printf( "unexpected: double = %lf (expected 1.0)\n", p->real ) ;
    exit( 1 ) ;
  }

  scan_double( p ) ; /*" .1"*/
  if ( p->scan_len != 3) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 2) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  if ( p->real != 0.1) {
    printf( "unexpected: double = %lf (expected 0.1)\n", p->real ) ;
    exit( 1 ) ;
  }

  scan_double( p ) ; /*" 1.23"*/
  if ( p->scan_len != 5) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 4) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  if ( p->real != 1.23) {
    printf( "unexpected: double = %lf (expected 1.23)\n", p->real ) ;
    exit( 1 ) ;
  }

  scan_double( p ) ; /*" 12.34"*/
  if ( p->scan_len != 6) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 5) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  if ( p->real != 12.34) {
    printf( "unexpected: double = %lf (expected 12.34)\n", p->real ) ;
    exit( 1 ) ;
  }

  scan_double( p ) ; /*" 001.23"*/
  if ( p->scan_len != 7) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 6) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  if ( p->real != 1.23) {
    printf( "unexpected: double = %lf (expected 1.23)\n", p->real ) ;
    exit( 1 ) ;
  }

  scan_double( p ) ; /*" 1.23e3"*/
  if ( p->scan_len != 7) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 6) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  if ( p->real != 1230.0) {
    printf( "unexpected: double = %lf (expected 1.23e+3)\n", p->real ) ;
    exit( 1 ) ;
  }

  scan_double( p ) ; /*" 1.23e+3"*/
  if ( p->scan_len != 8) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 7) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  if ( p->real != 1230.0) {
    printf( "unexpected: double = %lf (expected 1.23e+3)\n", p->real ) ;
    exit( 1 ) ;
  }

  scan_double( p ) ; /*" 1.23E-3"*/
  if ( p->scan_len != 8) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 7) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  if ( p->real != 1.23e-3) {
    printf( "unexpected: double = %lf (expected 1.23e-3)\n", p->real ) ;
    exit( 1 ) ;
  }

  scan_double( p ) ; /*" 123E5"*/
  if ( p->scan_len != 6) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 5) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  if ( p->real != 12300000.0) {
    printf( "unexpected: double = %lf (expected 1.23e7)\n", p->real ) ;
    exit( 1 ) ;
  }

  scan_init( p, scan_from_mem ) ; /*clears putback buffer*/
  p->p = floating2 ;
  scan_double( p ) ; /*"12.34"*/
  if ( p->scan_len != 5) {
    printf( "unexpected: scan_len = %u\n", (unsigned)(p->scan_len) ) ;
    exit( 1 ) ;
  }
  if ( p->token_len != 5) {
    printf( "unexpected: token_len = %u\n", (unsigned)(p->token_len) ) ;
    exit( 1 ) ;
  }
  if ( p->real != 12.34) {
    printf( "unexpected: double = %lf (expected 1.23e7)\n", p->real ) ;
    exit( 1 ) ;
  }


  printf( "All tests passed successfully\n" ) ;
}
#endif /*TEST*/

/*-------------------->   x   <---------------------------------- 2013-Nov-09
This function x
-----------------------------------------------------------------------------
Used functions: x
Globals:   x
Internals: x
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
/*x()*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/
/***************************************************************************/
