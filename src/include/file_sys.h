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
file_sys.h

Implements a file system API which is independent of the underlying file
system as well as of the compiler.
*****************************************************************************
History: (latest change first)
2014-Jun-08: added ~_IGNORE_CASE (does not work, but is not required under FAT)
2013-Aug-22: added ~_mkdir()
2013-Aug-21: added ~_set_attr()
2013-Apr-12: added ~file_size_zero
2013-Feb-26: added ~file_is_plain and ~search_plain
2012-Jun-03: comments added
2011-Apr-24: comments added
2011-Mar-01..03: extensions for Unix
2010-May-27: added ~get_extension
2010-Apr-11: ~get_abs_path renamed to ~make_abs_path
2010-Apr-07..08: added "~opendir", "~dir_get_next" and "~closedir"
2010-Apr-04: Conceptually absorbed parts of "cwd.h"
2010-Apr-02..03: added "file_sys_get_fileinfo"
2005-Jul-11: funcs added: "file_sys_get_cwd" , "file_sys_get_abs_path"
2004-Apr-05: funcs added: "file_sys_is_abs_path" and "file_sys_file_exists"
10.04.98: Ersterstellung
****************************************************************************/
#ifndef FILE_SYS_H
#define FILE_SYS_H
/***************************************************************************/

/*--  nested include files  -----------------------------------------------*/

#ifndef MISC_H
# include <misc.h>
#endif
#include <stdlib.h>


/*--  constants  ----------------------------------------------------------*/

#ifdef _TURBO_C_
#define FILE_SYS_MAX_PATH   MAXPATH

#elif _MS_C_
#define FILE_SYS_MAX_PATH   _MAX_PATH

#elif __TINYC__
# include <stdio.h>
#define FILE_SYS_MAX_PATH   FILENAME_MAX

#elif __GNUC__
# include <limits.h>
#define FILE_SYS_MAX_PATH   PATH_MAX

#else
#  error  file_sys.h: no compiler specified
#endif

#ifdef __unix__
#  define PATH_SEPARATOR '/'
#  define PATH_SEPARATOR_STR "/"
#else
#  define PATH_SEPARATOR '\\'
#  define PATH_SEPARATOR_STR "\\"
#endif

         /*alias*/
#define FILE_SYS_PATH_SEPARATOR PATH_SEPARATOR


/*--  typedefs & enums  ---------------------------------------------------*/

typedef struct {
  char* name ; /*always on heap when filled in by "file_sys_*"*/
  U32 size ;
  U32 mod_time ; /*seconds since 1900-Jan-01 00:00:00*/ /*will fail in 2036*/
  U16 mod_time_year ; /*2010 = 2010*/
  U8  mod_time_month ; /*1..12*/
  U8  mod_time_day ; /*1..31*/
  U8  mod_time_hour ; /*0..23*/
  U8  mod_time_min ; /*0..59*/
  U8  mod_time_sec ; /*0..59*/
  U8  attr ;
} FILE_SYS_FILEINFO ;

   /*Attributes in FILEINFO*/
#  define FILE_SYS_A_NORMAL 0x00
#  define FILE_SYS_A_RDONLY 0x01
#  define FILE_SYS_A_HIDDEN 0x02
#  define FILE_SYS_A_SYSTEM 0x04
#  define FILE_SYS_A_VOLID  0x08
#  define FILE_SYS_A_SUBDIR 0x10
#  define FILE_SYS_A_ARCH   0x20
#  define FILE_SYS_A_ALL    0x3f


/*--  function prototypes  ------------------------------------------------*/

BIT file_sys_file_exists( char* filename ) ; /*... and it may be a directory*/
BIT file_sys_file_is_plain( char* filename ) ; /*exists and is plain file*/
BIT file_sys_file_size_zero( char* filename ) ; /*is plain file with size 0*/
BIT file_sys_is_abs_path( char* filename ) ;

   /*No check is made if the path exists.
     Dest = NULL selects an internal static buffer*/
char* file_sys_make_abs_path( char* rel_path, char* dest ) ;

   /*Filename must be a basename.  Search_path_list is ';' separated*/
   /*Returns an absolute path in a static var or NULL if not found*/
char* file_sys_search_plain( char* filename, char* search_path_list ) ;

      /*these functions simply return pointers into the path arg*/
char* basename( char* path ) ;
char* file_sys_get_extension( char* path ) ;

char* file_sys_get_cwd( void ) ; /*the LAST ret_val is stored internally*/
BIT   file_sys_set_cwd( char* path ) ; /*TRUE if command failed*/

FILE_SYS_FILEINFO* file_sys_get_fileinfo( char* filename,
                        FILE_SYS_FILEINFO* dest, U8 flags ) ;
   /*returns NULL if file does not exist*/
        /*U8 flags*/
#define FILE_SYS_NULL_NAME     0
#define FILE_SYS_LINK_NAME     1
#define FILE_SYS_DUP_NAME      2
#define FILE_SYS_IGNORE_CASE 128


   /*Same format as attr byte in FILEINFO*/
   /*Only a, r, h and s bits can be manipulated*/
BIT file_sys_set_attr( char* path, U8 attr ) ; /*TRUE if command failed*/


   /*these functions read the content of a directory*/
FILE_SYS_FILEINFO* file_sys_opendir( char* dirname, FILE_SYS_FILEINFO* dest ) ;
FILE_SYS_FILEINFO* file_sys_dir_get_next( FILE_SYS_FILEINFO* dest ) ;
void file_sys_closedir( void ) ;


/*--  macros  -------------------------------------------------------------*/


/*--  global variables  ---------------------------------------------------*/

/*extern */

/***************************************************************************/
#endif	/* FILE_SYS_H */
/***************************************************************************/
