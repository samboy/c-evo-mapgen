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
file_sys.c

This file implements some functions which help the programmer to write
applications independent of the underlying file system.
*****************************************************************************
History: (latest change first)
2016-Jul-22: Debugging (opendir/readdir/closedir)
2016-Apr-05: function find_file() undefined under __unix__
2016-Apr-01: fixed gcc complaints when compiling with DEBUG
2014-Sep-24: corrected typos
2014-Jun-08: new flag ~_IGNORE_CASE for ~_get_fileinfo()
             (neither does it work nor is it required -- it is inherent!!)
2013-Aug-22: added ~file_size_mkdir
2013-Apr-12: added ~file_size_zero
2013-Feb-26: added ~file_is_plain and ~search_plain
2011-Oct-01: DIR_NESTING 10->30
2011-Apr-24..26: implemented "~get_fileinfo" for non-unix (attributes)
2011-Mar-02..03: Adopted to GNU-C under Linux
2010-Sep-08: Debugging "~make_abs_path"
2010-May-27: added ~get_extension
2010-Apr-11..12: added ~make_abs_path
2010-Apr-10..11: Up to NESTING_DIR recursive calls to "~opendir" can be made
2010-Apr-07..08: added "~opendir", "~dir_get_next" and "~close_dir"
2010-Apr-05: added "file_sys_set_cwd"
2010-Apr-02: added "file_sys_get_fileinfo"
2007-Sep-15: added "basename"
2005-Jul-11: New funcs: file_sys_get_cwd, file_sys_get_abs_path
2004-Apr-05: initial version
*****************************************************************************
Global objects:
- FILE_SYS_FILEINFO* file_sys_get_fileinfo( char* filename,
                              FILE_SYS_FILEINFO* dest, U8 flags )
- BIT file_sys_file_exists
- BIT file_sys_is_abs_path
- char* basename
- char* file_sys_get_cwd
- char* file_sys_set_cwd
- char* file_sys_get_abs_path
- FILE_SYS* file_sys_opendir
- FILE_SYS* file_sys_dir_get_next
- void      file_sys_closedir
****************************************************************************/

/*--  switches  -----------------------------------------------------------*/

/*#define MAIN*/ /*generates program "file_sys_get_fileinfo"*/
/*#define TEST*/ /*generates main for self-test*/


/*--  include files  ------------------------------------------------------*/

/*#include <misc.h>*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MAIN
#include <getopts.h>
#endif /* MAIN */

/*#define DEBUG*/
#include <debug.h>

#ifdef _MS_C_
#  include <io.h>
#  include <direct.h>
#  include <time.h>
#  include <sys\types.h>
#  include <sys\stat.h>
#  include <dos.h>
#elif __TINYC__
#  include <io.h> /*for "finddata_t"*/
#  include <direct.h>
#  include <time.h>
   /*for "toupper"*/
#  include <ctype.h>
#  include <sys\types.h>
#  include <sys\stat.h>
#  include <dos.h>
#elif __GNUC__
#  include <unistd.h>
#  include <time.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <dirent.h>
#else
#  error  file_sys.c: no compiler specified (includes)
#endif


#include <file_sys.h>


/*--  constants  ----------------------------------------------------------*/

   /*for recursive opendir calls*/
#define DIR_NESTING 30


#ifdef _TURBO_C_
#define FA_ALL (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_LABEL | FA_DIREC \
              | FA_ARCH )
#define FA_NO_DIRS (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_ARCH )


#elif _MS_C_ || __TINYC__
#define _A_ALL (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_VOLID | _A_SUBDIR \
              | _A_ARCH )
#define _A_NO_DIRS (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH )

#elif __GNUC__
  /*left blank, not needed under Unix*/

#else
#  error  file_sys.c: no compiler specified (constants)
#endif


/*--  type declarations & enums  ------------------------------------------*/

/*typedef struct {*/
/*} NEW_TYPE ;*/


/*--  local function prototypes  ------------------------------------------*/

#ifndef __unix__
static FILE_SYS_FILEINFO* find_file( char* filename, FILE_SYS_FILEINFO* dest,
                                     U8 flags ) ;
#endif

#ifdef _TURBO_C_
  static void findtype2fileinfo( FILE_SYS_FILEINFO* dest,
                                 struct ffblk* src ) ;
#elif _MS_C_
  static void findtype2fileinfo( FILE_SYS_FILEINFO* dest,
                                 struct find_t* src ) ;
#elif __TINYC__
  static void findtype2fileinfo( FILE_SYS_FILEINFO* dest,
                                 struct _finddata_t* src ) ;
#elif __GNUC__
  static void findtype2fileinfo( FILE_SYS_FILEINFO* dest,
                                 struct _finddata_t* src ) ;

#else
#  error  file_sys.c: no compiler specified (prototypes)
#endif

#ifdef MAIN
static void usage( void ) ;
#endif /* MAIN */


/*--  macros  -------------------------------------------------------------*/

#define   STR_I_CMP(flags,a,b)  \
((((flags) & FILE_SYS_IGNORE_CASE) ? stricmp((a),(b)) : strcmp((a),(b)) ) == 0)


/*--  global variables  ---------------------------------------------------*/


/*--  internal variables  -------------------------------------------------*/

   /*common variables for findfirst/findnext*/
static FILE_SYS_FILEINFO fileinfo ;
/*static S8 stack_idx = -1 ;*/ /*index for findtype stack*/
static int stack_idx = -1 ; /*index for findtype stack*/

/*CAUTION: Cwd must not change between calls to readdir().*/
         /*The cwd which was effective when opendir() was called is*/
         /*remembered  here.*/
static char remember_dir [ DIR_NESTING ] [ FILE_SYS_MAX_PATH ] ;

#ifdef _TURBO_C_
     /*Turbo-C version not tested yet !!! */
  static unsigned find_status ;
  static struct ffblk find_result [ DIR_NESTING ] ;
  static struct ffblk find_result_temp ;

#elif _MS_C_
  static unsigned find_status ;
  static struct find_t find_result [ DIR_NESTING ] ;
  static struct find_t find_result_temp ;

#elif __TINYC__
  static int handle [ DIR_NESTING ] ;
  static int handle_temp ;
  static int findnext_status ;
  static struct _finddata_t find_result [ DIR_NESTING ] ;
  static struct _finddata_t find_result_temp ;

#elif __GNUC__
  static int handle [ DIR_NESTING ] ;
  static int handle_temp ;
  static int findnext_status ;
  static struct _finddata_t find_result [ DIR_NESTING ] ;
  static struct _finddata_t find_result_temp ;
  static DIR* dirp [ DIR_NESTING ] ;
  static struct dirent* dir_entry ;

#else
#  error  file_sys.c: no compiler specified (statics)
#endif

#ifdef MAIN
   /* Options */
static char flag_ignore_case ;
static char* c_arg ;
/*static char flag_count ;*/
/*static unsigned long count ;*/

OPTION_LIST(optlist)
OPTION_WO_ARG('c',flag_ignore_case)
/*OPTION_W_ARG('y',flag_y,y_arg)*/
/*OPTION_NUMBER(flag_count,count)*/
OPTION_LIST_END


/*-------------------->   main   <------------------------------- 2016-Apr-05
Small test and demo program
-----------------------------------------------------------------------------
Used functions: usage
Globals:   x
Internals: x
Parameters: - argc, argv
Return value: --
Exitcode:     EXITCODE_OK, EXITCODE_USAGE
---------------------------------------------------------------------------*/
void main( int argc, char** argv )
{
THIS_FUNC(main)
  FILE_SYS_FILEINFO* fileinfo ;
  U8 flags ;


  OPTION_GET(optlist)
  if ( argc != 1 ) {
    usage() ;
  }
  PRT_VAR(argv [0],s)
  flags = flag_ignore_case
        ? FILE_SYS_LINK_NAME | FILE_SYS_IGNORE_CASE : FILE_SYS_LINK_NAME ;

  fileinfo = file_sys_get_fileinfo( argv [0], NULL, flags ) ;
  if (fileinfo == NULL) {
    printf( "file not found\n" ) ;
  }
  else {
    printf( "%s: %lu bytes\n", fileinfo->name,
            (unsigned long)(fileinfo->size) ) ;
  }

  exit( EXITCODE_OK ) ;
}

/*-------------------->   usage   <------------------------------ 2014-Jun-08
This function displays an ultra-short usage instruction.
-----------------------------------------------------------------------------
Used functions: fprintf, exit
Globals:   --
Internals: --
Parameters: --
Return value: -- (doesn't return)
Exitcode:     EXITCODE_USAGE
---------------------------------------------------------------------------*/
static void usage( void )
{
  fprintf( stderr,
"usage: file_sys [options] pathname\n"
"       This test program prints out some info about the argument file\n"
"Options:\n"
"-c        ignore case in file name\n"
/*"\n"*/
         ) ;
  exit( EXITCODE_USAGE ) ;
}
#endif /* MAIN */


/*--  library functions  --------------------------------------------------*/

/*-------------------->   file_sys_file_exists   <------------ 2004-Apr-05 --
This function returns TRUE if the given path exists.
It can be a plain file, a directory, or even a volume ID.
-----------------------------------------------------------------------------
Used functions: access
Parameters:	- char* filename
Return value:	TRUE if the file exists
Exitcode:	--
---------------------------------------------------------------------------*/
BIT file_sys_file_exists( char* filename )
{
THIS_FUNC(file_sys_file_exists)

   if (access( filename, 0 )) {
      return FALSE ;
   }
   return TRUE ;
}

/*-------------------->   file_sys_file_is_plain   <------------- 2013-Feb-26
This function returns TRUE if the given path exists and is a plain file.
It still might be r/o, system, or hidden.
-----------------------------------------------------------------------------
Used functions: file_sys_get_fileinfo
Globals:    --
Internals:  --
Parameters:	- char* filename
Return value:   TRUE if the file exists and is a plain file
Exitcode:       --
---------------------------------------------------------------------------*/
BIT file_sys_file_is_plain( char* filename )
{
THIS_FUNC(file_sys_file_is_plain)
  FILE_SYS_FILEINFO* fileinfo ;

  fileinfo = file_sys_get_fileinfo( filename, NULL, FILE_SYS_NULL_NAME ) ;
  if (fileinfo == NULL) {
    return FALSE ;
  }
  if ((fileinfo->attr & (FILE_SYS_A_VOLID | FILE_SYS_A_SUBDIR)) != 0) {
    return FALSE ;
  }
  return TRUE ;
}

/*-------------------->   file_sys_file_size_zero   <------------ 2013-Apr-12
This function returns TRUE if the given path exists and is a plain file
with size zero.
It still might be r/o, system, or hidden.
-----------------------------------------------------------------------------
Used functions: file_sys_get_fileinfo
Globals:    --
Internals:  --
Parameters:	- char* filename
Return value:   TRUE if file is plain and has size zero, else FALSE
Exitcode:       --
---------------------------------------------------------------------------*/
BIT file_sys_file_size_zero( char* filename )
{
THIS_FUNC(file_sys_file_size_zero)
  FILE_SYS_FILEINFO* fileinfo ;

  fileinfo = file_sys_get_fileinfo( filename, NULL, FILE_SYS_NULL_NAME ) ;
  if (fileinfo == NULL) {
    return FALSE ;
  }
  if ((fileinfo->attr & (FILE_SYS_A_VOLID | FILE_SYS_A_SUBDIR)) != 0) {
    return FALSE ;
  }
  if (fileinfo->size != 0) {
    return FALSE ;
  }
  return TRUE ;
}

/*-------------------->   file_sys_is_abs_path   <------------ 2004-Apr-05 --
This function returns TRUE if the given filename is an absolute pathname.
No check is made wether the file exists or not.
-----------------------------------------------------------------------------
Used functions: strncmp
Parameters:	- filename:  the path to examine
Return value:	TRUE if the filename is an absolute pathname
Exitcode:	--
---------------------------------------------------------------------------*/
BIT file_sys_is_abs_path( char* filename )
{
THIS_FUNC(file_sys_is_abs_path)

   PRT_VAR(filename,s)
   if ( *filename == PATH_SEPARATOR) {
      return TRUE ;
   }
   if ( strncmp( & (filename [ 1 ]), ":\\", 2 ) == 0 ) {
      return TRUE ;
   }
   return FALSE ;
}

/*-------------------->   file_sys_search_plain   <-------------- 2013-Feb-26
This function searches for a plain file given by "filename".
It looks into several directories, specified by "search_path_list".
If you want to search the current dir too, include "." into the search list.
-----------------------------------------------------------------------------
Used functions: file_sys_get_cwd
Globals:    --
Internals:  --
Parameters:     - filename           must be a basename
                - search_path_list   must be a ';' separated list of dirs. 
                                     Abs and rel dirs may be mixed.
Return value:   The complete, absolute path of the file or NULL if not found
Exitcode:       --
---------------------------------------------------------------------------*/
char* file_sys_search_plain( char* filename, char* search_path_list )
{
THIS_FUNC(file_sys_search_plain)
static char composed_path [ FILE_SYS_MAX_PATH +1 ] ;
  char  search_path [ FILE_SYS_MAX_PATH +1 ] ; /*abs or rel, copied from arg*/
  char* search_path_poi ;
  char* search_list_poi ;


  ASSERT(filename != NULL)
  PRT_VAR(filename,s)
  ASSERT(search_path_list != NULL)
  PRT_VAR(search_path_list,s)

  search_list_poi = search_path_list ;
  while (*search_list_poi) { /*not yet encountered '\0'*/
    if (*search_list_poi == ';') { /*needed from 2nd loop cycle on*/
      search_list_poi++ ;
    }
    search_path_poi = search_path ;
    while((*search_list_poi != ';') && (*search_list_poi != '\0')) {
      *search_path_poi++ = *search_list_poi++ ;
    }
    *search_path_poi++ = '\0' ;
    PRT_VAR(search_path,s)
    file_sys_make_abs_path( search_path, composed_path ) ;
    strcat( composed_path, PATH_SEPARATOR_STR ) ;
    strcat( composed_path, filename ) ;
    PRT_VAR(composed_path,s)
    if (file_sys_file_is_plain( composed_path )) {
      return composed_path ; /*file found*/
    }
    /*search_list_poi points either to ';' or to '\0' here*/
  }
  return NULL ; /*file not found*/
}

/*-------------------->   file_sys_get_fileinfo   <-------------- 2014-Jun-08
This function obtains misc info about a file, like size, attr, date.

Under Unix:
- the readonly attribute is set according to the user id.
- the hidden attribute is set when the filename starts with '.'
ToDo: What if a DOS partition is mounted under unix?
-----------------------------------------------------------------------------
Used functions:
Parameters:	- filename
                - dest: NULL or pointer to FILE_INFO
                - flags: ~NULL_NAME, ~LINK_NAME, ~DUP_NAME, ~_IGNORE_CASE
Return value:	NULL if info cannot be obtained
Exitcode:	--
---------------------------------------------------------------------------*/
FILE_SYS_FILEINFO* file_sys_get_fileinfo( char* filename,
                 FILE_SYS_FILEINFO* dest, U8 flags )
{
THIS_FUNC(file_sys_get_fileinfo)
  FILE_SYS_FILEINFO* ret_val ;
#ifdef __unix__
  struct stat stat_buf ;
  struct tm* mtime ;
#endif /*__unix__*/

  ret_val = dest ;
  if ( dest == NULL ) {
    ret_val = & fileinfo ; /*use internal variable*/
  }

#ifdef __unix__
  if (stat( filename, & stat_buf ) != 0) {
    return NULL ;
  }
#endif /*__unix__*/

  switch (flags & 0x03) {
  case FILE_SYS_NULL_NAME:
    ret_val->name = NULL ;
    break ;

  case FILE_SYS_LINK_NAME:
    ret_val->name = filename ;
    break ;

  case FILE_SYS_DUP_NAME:
    ret_val->name = strdup( filename ) ;
    break ;

  default:
    fprintf( stderr, "file_sys_get_fileinfo: wrong flags (0x%02x)\n",
                     (int)flags ) ;
    exit( EXITCODE_WRONG_PARAM ) ;
  }

#ifdef __unix__
  ret_val->size = (U32)(stat_buf.st_size) ;
  ret_val->mod_time = (U32)(stat_buf.st_mtime) ;

  mtime = localtime( & (stat_buf.st_mtime) ) ;
  ret_val->mod_time_year = (U16)(mtime->tm_year + 1900) ;
  ret_val->mod_time_month = (U8)(mtime->tm_mon + 1) ;
  ret_val->mod_time_day = (U8)(mtime->tm_mday) ;
  ret_val->mod_time_hour = (U8)(mtime->tm_hour) ;
  ret_val->mod_time_min = (U8)(mtime->tm_min) ;
  ret_val->mod_time_sec = (U8)(mtime->tm_sec) ;


  ret_val->attr = 0x00 ; /*default*/

  if (*filename == '.') {
    ret_val->attr = FILE_SYS_A_HIDDEN ;
  }
  if (S_ISDIR( stat_buf.st_mode )) {
    ret_val->attr |= FILE_SYS_A_SUBDIR ;
  }
  if (getuid() == stat_buf.st_uid) { /*file belongs to this user*/
    if ((stat_buf.st_mode & S_IWUSR) == 0) {
      ret_val->attr |= FILE_SYS_A_RDONLY ;
    }
  }
  else if (getgid() == stat_buf.st_gid) { /*file belongs to this group*/
    if ((stat_buf.st_mode & S_IWGRP) == 0) {
      ret_val->attr |= FILE_SYS_A_RDONLY ;
    }
  }
  else { /*file belongs to others*/
    if ((stat_buf.st_mode & S_IWOTH) == 0) {
      ret_val->attr |= FILE_SYS_A_RDONLY ;
    }
  }

#else /*DOS, Windows*/

# ifdef _TURBO_C_
  find_status = findfirst( filename, & find_result_temp, FA_ALL ) ;
  if (find_status == 0) { /*matching file is found*/
    findtype2fileinfo( ret_val, & find_result_temp ) ;
    return ret_val ;
  }
  return NULL ;

# elif _MS_C_
  if ((flags & FILE_SYS_IGNORE_CASE) == 0) {
    find_status = _dos_findfirst( filename, _A_ALL, & find_result_temp ) ;
    if (find_status == 0) { /*matching file is found*/
      findtype2fileinfo( ret_val, & find_result_temp ) ;
      return ret_val ;
    }
    return NULL ;
  }
  else {
    return find_file( filename, ret_val, flags ) ;
  }

# else
  if ((flags & FILE_SYS_IGNORE_CASE) == 0) {
    handle_temp = _findfirst( filename, & find_result_temp ) ;
    if (handle_temp == -1) { /*no match found*/
      RETURN( NULL,08x) ;
    }
    _findclose( handle_temp ) ; /*returns int: meaning?  values??*/
    findtype2fileinfo( ret_val, & find_result_temp ) ;
    RETURN( ret_val,08x) ;
  }
  else {
    return find_file( filename, ret_val, flags ) ;
  }

# endif

#endif /*unix/DOS*/
  return ret_val ;
}

/*-------------------->   basename   <--------------------------- 2007-Sep-15
This function returns a pointer to the basename (incl. ext) of a pathname.
-----------------------------------------------------------------------------
Used functions: --
Parameters:	- path: the path from which the basename is to be extracted
Return value:	pointer into parameter "path", where basename starts
Exitcode:	--
---------------------------------------------------------------------------*/
char* basename( char* path )
{
THIS_FUNC(basename)
  char* p = path ;

  PRT_VAR(path,s)
  while (*p != '\0') {
    p++ ;
  }
  /*p points now to the trailing \0 of path*/
  PRT_VAR((unsigned long)(p-path),lu)

  while (1) {
    if (*p == PATH_SEPARATOR) {
      /*PRT_VAR(p-path,u)*/
      /*PRT_VAR(p,s)*/
      /*return ( p +1 ) ;*/ /*doesn't work properly.  Compiler bug?*/
      p++ ;
      return p ;
    }
    if (p == path) {
      PRT_VAR((unsigned long)(p-path),lu)
      return p ;
    }
    p-- ;
  }
}

/*-------------------->   file_sys_get_extension   <------------- 2010-May-27
This function returns a pointer to the extension (including the dot) of
a filename or path.
If the file has no extension, a pointer to an empty string is returned.
The return value points in either case into the filename supplied as
argument, so don't move it away.
-----------------------------------------------------------------------------
Used functions:
Globals/Internals:
Parameters:	- filename
Return value:	pointer into parameter "filename"
Exitcode:	--
---------------------------------------------------------------------------*/
char* file_sys_get_extension( char* filename )
{
THIS_FUNC(file_sys_get_extension)
  char* ret_val ;
  char* remember_end ; /*points to the terminating '\0'*/

  ret_val = filename ;
  while (*ret_val != '\0') { /*search end of filename*/
    ret_val++ ;
  }
  remember_end = ret_val ;

  while (ret_val != filename) { /*now scan backwards for '.' or '\'*/
    ret_val-- ;
    if (*ret_val == '.') { /*done*/
      return ret_val ;
    }
    if (*ret_val == FILE_SYS_PATH_SEPARATOR) { /*done, too*/
      return remember_end ;
    }
  }
  return remember_end ; /*neither '.' nor (back)slash in whole path*/
}

/*-------------------->   file_sys_get_cwd   <------------------- 2010-Apr-04
This function returns the current working directory as a string.
-----------------------------------------------------------------------------
Used functions: getcwd
Parameters:	--
Return value:	pointer to an internally stored string (static)
Exitcode:	EXITCODE_UNEXPECTED_ERR
---------------------------------------------------------------------------*/
char* file_sys_get_cwd( void )
{
THIS_FUNC(file_sys_get_cwd)
static char ret_val [ FILE_SYS_MAX_PATH ] ;

  if (getcwd( ret_val, FILE_SYS_MAX_PATH ) != ret_val) {
    fprintf( stderr, "file_sys_get_cwd: getcwd() failed\n" ) ;
    exit( EXITCODE_UNEXPECTED_ERR ) ;
  }
  return ret_val ;
}

/*-------------------->   file_sys_set_cwd   <------------------- 2011-Mar-02
This function changes the current working directory.
-----------------------------------------------------------------------------
Used functions: strchr, setdisk, toupper, _chdrive, chdir
Parameters:	- path: Drive and directory (w/o trailing backslash!)
Return value:	- FALSE: ok
		- TRUE: cannot change for some reason
Exitcode:	EXITCODE_WRONG_PARAM (drive letter under Unix)
---------------------------------------------------------------------------*/
BIT file_sys_set_cwd( char *path )
{
THIS_FUNC(file_sys_set_cwd)

  /*PRT_VAR(path,s)*/
  if( strchr( path, ':' ) == path +1 ) { /*path contains drive letter*/
    PRT_VAR(*path,c)
    /*printf( "chdrv->%c:\n", *path ) ;*/

#ifdef _TURBO_C_
    setdisk( toupper(*path) -'A' ) ;
#elif _MS_C_ || __TINYC__
    if (_chdrive( toupper(*path) -'@' )) {
      return TRUE ;
    }
#elif __GNUC__
    fprintf( stderr, "file_sys_set_cwd: No drive letters under Unix\n" ) ;
    exit( EXITCODE_WRONG_PARAM ) ;

#else
#  error  file_sys.c: no compiler specified (cwd)
#endif

    chdir( "\\" ) ; /*change to DOS root dir*/
    path += 2 ;
  }
  if (*path) {
    /*printf( "chdir->%s\n", path ) ;*/
    if (chdir( path )) {
      return TRUE ;
    }
  }
  return FALSE ;
}

/*-------------------->   file_sys_make_abs_path   <------------- 2010-Sep-08
This function returns the absolute path for a relative path string.
The input string may contain any number of "." and ".." references.
It may even be an absolute path (though the param is called "rel_path").
No check is made whether the path exists or not.
-----------------------------------------------------------------------------
Used functions: file_sys_is_abs_path, file_sys_get_cwd,
                fprintf, exit,
                strlen, strcpy, strcat
Parameters:	- rel_path: pointer to rel_path string
                - dest: put the return value here
Return value:	pointer to an internally stored string (static)
Exitcode:	EXITCODE_WRONG_PARAM
---------------------------------------------------------------------------*/
char* file_sys_make_abs_path( char* rel_path, char* dest )
{
THIS_FUNC(file_sys_make_abs_path)
static char ret_val_buf [ FILE_SYS_MAX_PATH ] ;
  char* ret_val ;
  char* read_p ;
  char* root_point ;
  char* this_dir ;

  /*PRT_VAR(rel_path,s)*/
  ret_val = dest ;
  if (dest == NULL) {
    ret_val = ret_val_buf ;
  }

     /*Step 0: One simple check of input parameter*/
  if (strlen( rel_path ) > FILE_SYS_MAX_PATH) {
    fprintf( stderr, "file_sys_make_abs_path: arg too long\n" ) ;
    exit( EXITCODE_WRONG_PARAM ) ;
  }

     /*Step 1: Test if it is already an absolute path*/
  if (file_sys_is_abs_path( rel_path )) {
    strcpy( ret_val , rel_path ) ;
  }
  else { /*Step 1b: make an absolute path*/
    sprintf( ret_val, "%s%c%s",
             file_sys_get_cwd(), PATH_SEPARATOR, rel_path
           ) ;
  }
  /*PRT_VAR(ret_val,s)*/

     /*Step 2: Remove all . and .. references
       as well as double PATH_SEPARATORs*/
  if (*ret_val == PATH_SEPARATOR) { /*Unix style file system*/
    root_point = ret_val ;
  }
  else if (strncmp( ret_val +1, ":\\", 2 ) == 0) { /*DOS style file system*/
    root_point = ret_val +2 ;
  }
  else {
    fprintf( stderr, "file_sys_make_abs_path: unexpected abs path: >%s<\n",
             ret_val ) ;
    exit( EXITCODE_UNEXPECTED_ERR ) ;
  }
     /*"root_point" points to the root (back)slash here*/
  /*PRT_VAR(root_point,s)*/
  ASSERT(*root_point == PATH_SEPARATOR)

  read_p = root_point +1 ;
  while (*read_p != '\0') { /*another dir to process*/
    /*PRT_VAR(read_p,s)*/
    this_dir = read_p ;
    while ((*read_p != PATH_SEPARATOR) && (*read_p != '\0')) {
      read_p++ ;
    }
    /*PRT_VAR(read_p,s)*/

    switch (read_p - this_dir) { /* = len of dir name*/
    case 0: /*double PATH_SEPARATOR => omit 2nd PATH_SEPARATOR*/
      /*DEB((stderr,"case 0\n"))*/
      strcpy( this_dir, this_dir +1 ) ;
      break ; /*read_p points after the PATH_SEPARATOR*/

    case 1:
      /*DEB((stderr,"case 1\n"))*/
      if (*this_dir == '.') { /*omit this . and preceeding PATH_SEPARATOR*/
        strcpy( this_dir -1, read_p ) ;
        read_p = this_dir -1 ;
      }
      if (*read_p != '\0') {
        read_p++ ;
      }
      break ; /*read_p points to beginning of next dir or to '\0'*/

    case 2:
      /*DEB((stderr,"case 2\n"))*/
      if (strncmp( this_dir, "..", 2 ) == 0) {
           /*omit .., the preceeding PATH_SEPARATOR
             and the dir before (if any !!) plus the preceeding PATH_S...*/
        read_p = this_dir -2 ; /*point to end of previous dir*/
        if (read_p <= root_point) {
          fprintf( stderr,
            "file_sys_make_abs_path: dotdot directory in top level: %s\n",
            ret_val ) ;
          exit( EXITCODE_WRONG_PARAM ) ;
        }
        while (*read_p != PATH_SEPARATOR) {
          read_p-- ;
        }
        /*read_p points to PATH_SEPARATOR before previous dir now*/
        /*PRT_VAR(read_p,s)*/
        strcpy( read_p, this_dir +2 ) ;
        /*PRT_VAR(ret_val,s)*/
      }
      if (*read_p != '\0') {
        read_p++ ;
      }
      break ; /*read_p points to beginning of next dir or to '\0'*/

    default:
      /*DEB((stderr,"default\n"))*/
      if (*read_p != '\0') {
        read_p++ ;
      }
      break ; /*read_p points to beginning of next dir or to '\0'*/
    } /*end switch*/
  } /*end while*/

  /*PRT_VAR(ret_val,s) ;*/
  if(*--read_p == PATH_SEPARATOR) {
    *read_p = '\0' ; /*no PATH_SEPARATOR at end ...*/
  }
  if (read_p == root_point) {
    *read_p = PATH_SEPARATOR ; /*... unless end == root*/
  }
  RETURN(ret_val,s) ;
}

/*-------------------->   file_sys_mkdir   <--------------------- 2013-Aug-22
This function creates a new directory under the cwd.
-----------------------------------------------------------------------------
Used functions: mkdir, _mkdir
Globals:    --
Internals:  --
Parameters:     - dirname      the name of the new directory
Return value:   TRUE if failed
Exitcode:       --
---------------------------------------------------------------------------*/
BIT file_sys_mkdir( char* dirname )
{
THIS_FUNC(file_sys_mkdir)
#ifdef _MS_C_
  if (mkdir( dirname ) != 0) {
    return TRUE ; /*failed*/
  }
#elif __TINYC__
  if (_mkdir( dirname ) != 0) {
    return TRUE ; /*failed*/
  }
#endif
  return FALSE ; /*ok*/
}

/*-------------------->   file_sys_opendir   <------------------- 2016-Jul-22
This function "opens" a directory for subsequent read commands and issues
the first read command.  It returns the first file in the directory.
The function usually changes the working directory.  Use "cwd_save/restore"
functions to restore your current working directory.
-----------------------------------------------------------------------------
Used functions:
Parameters:	- dirname:  absolute or relative directory path
		- dest:  where to save file info
Return value:	Pointer to FILE_SYS_INFO for 1st file or NULL upon error
Exitcode:	EXITCODE_TABLE_FULL if nesting is exhausted
---------------------------------------------------------------------------*/
FILE_SYS_FILEINFO* file_sys_opendir( char* dirname, FILE_SYS_FILEINFO* dest )
{
THIS_FUNC(file_sys_opendir)
  FILE_SYS_FILEINFO* ret_val ;

  PRT_VAR(dirname,s)
  /*PAUSE*/
  stack_idx++ ; /*value changes from -1 to 0 at first call*/
  if (stack_idx == DIR_NESTING) {
    fprintf( stderr, "file_sys_opendir: nesting too deep -- aborting\n" ) ;
    exit( EXITCODE_TABLE_FULL ) ;
  }
  DEB((stderr,"now opening dir >%s<(handle #%d)\n",dirname,(int)stack_idx))

  ret_val = dest ;
  if ( dest == NULL ) {
    ret_val = & fileinfo ;
  }
  if (file_sys_set_cwd( dirname ) ) { /*TRUE if we cannot goto dir*/
    /*RETURN( NULL,08lx) ;*/ /*gcc complains*/
    return NULL ;
  }
  PRT_VAR(file_sys_get_cwd(),s)
  strcpy( & (remember_dir [ stack_idx ] [0]), file_sys_get_cwd() ) ;

#ifdef _TURBO_C_
  find_status = findfirst( "*.*", & (find_result [ stack_idx ]), FA_ALL ) ;
  if (find_status == 0) { /*matching file is found*/
    findtype2fileinfo( ret_val, & (find_result [ stack_idx ]) ) ;
    return ret_val ;
  }
  return NULL ;

#elif _MS_C_
  find_status = _dos_findfirst( "*.*", _A_ALL, & (find_result [ stack_idx ]) ) ;
  if (find_status == 0) { /*matching file is found*/
    findtype2fileinfo( ret_val, & (find_result [ stack_idx ]) ) ;
    return ret_val ;
  }
  return NULL ;

#elif __TINYC__
  handle [ stack_idx ] = _findfirst( "*.*", & (find_result [ stack_idx ]) ) ;
  if (handle [ stack_idx ] == -1) { /*no match found*/
    RETURN( NULL,08x) ;
  }
  findtype2fileinfo( ret_val, & (find_result [ stack_idx ]) ) ;
  RETURN( ret_val,08x) ;

#elif __GNUC__
  dirp [ (int)stack_idx ] = opendir( dirname ) ;
  if (dirp [ (int)stack_idx ] == NULL) {
    return NULL ;
  }
  dir_entry = readdir( dirp [ (int)stack_idx ] ) ; /*1st read*/
  if (dir_entry == NULL) {
    DEB((stderr,"NOTHING READ -> EMPTY DIR -> NO DOT/DOTDOT ???\n"))
    return NULL ;
  }
  DEB((stderr,"now returning FILEINFO for >%s<(1st read)\n",dir_entry->d_name))
  return file_sys_get_fileinfo( dir_entry->d_name,
                                ret_val, FILE_SYS_DUP_NAME ) ;


#else
#  error  file_sys.c: no compiler specified (opendir)
#endif
}

/*-------------------->   file_sys_dir_get_next   <-------------- 2016-Jul-22
This function performs a subsequent directory read command. It returns the
next matching file.
-----------------------------------------------------------------------------
Used functions:
Internals/Globals: find_status, findnext_status
Parameters:	- x
		- x
Return value:	x
Exitcode:	x
---------------------------------------------------------------------------*/
FILE_SYS_FILEINFO* file_sys_dir_get_next( FILE_SYS_FILEINFO* dest )
{
THIS_FUNC(file_sys_dir_get_next)
  FILE_SYS_FILEINFO* ret_val ;

  ret_val = dest ;
  if ( dest == NULL ) {
    ret_val = & fileinfo ;
  }
  DEB((stderr,"getting next item for handle #%d\n", stack_idx))
  PRT_VAR(& (remember_dir [ stack_idx ] [0]),s)
  file_sys_set_cwd( & (remember_dir [ stack_idx ] [0]) ) ;
  PRT_VAR(file_sys_get_cwd(),s)

#ifdef _TURBO_C_
  find_status = findnext( & (find_result [ stack_idx ]) ) ;
  if (find_status == 0) { /*matching file is found*/
    findtype2fileinfo( ret_val, & (find_result [ stack_idx ]) ) ;
    return ret_val ;
  }
  return NULL ;

#elif _MS_C_
  find_status = _dos_findnext( & (find_result [ stack_idx ]) ) ;
  if (find_status == 0) { /*matching file is found*/
    findtype2fileinfo( ret_val, & (find_result [ stack_idx ]) ) ;
    return ret_val ;
  }
  return NULL ;

#elif __TINYC__
  findnext_status = _findnext( handle [ stack_idx ],
                       & (find_result [ stack_idx ]) ) ;
  if (findnext_status == -1) { /*no match found*/
    return NULL ;
  }
  if (findnext_status != 0) {
    fprintf( stderr, "file_sys_dir_get_next: _findnext returns %d\n",
             findnext_status ) ;
    exit( EXITCODE_UNEXPECTED_ERR ) ;
  }
     /*match found*/
  findtype2fileinfo( ret_val, & (find_result [ stack_idx ]) ) ;
  return ret_val ;

#elif __GNUC__
  dir_entry = readdir( dirp [ (int)stack_idx ] ) ;
  if (dir_entry == NULL) {
    DEB((stderr,"nothing read\n"))
    return NULL ;
  }
  DEB((stderr,"now returning FILEINFO for >%s<\n",dir_entry->d_name))
  return file_sys_get_fileinfo( dir_entry->d_name,
                                ret_val, FILE_SYS_DUP_NAME ) ;

#else
#  error  file_sys.c: no compiler specified (dir_get_next)
#endif
}

/*-------------------->   file_sys_closedir   <------------------ 2016-Jul-22
This function closes a directory which was previously opened with
"file_sys_opendir".  Closing is not necessary for all compilers.
Call "file_sys_closedir" anyway to keep your code compatible.
-----------------------------------------------------------------------------
Used functions: _findclose, closedir, fprintf, exit
Globals/Internals: stack_idx, handle, dirp
Parameters:	--
Return value:	void
Exitcode:	EXIT_CODE__WRONG_CALL upon close without previous open
                EXITCODE_UNEXPECTED_ERR if closedir fails
---------------------------------------------------------------------------*/
void file_sys_closedir( void )
{
THIS_FUNC(file_sys_closedir)
  int status ;

  if (stack_idx == -1) { /*stack is "idle"*/
    fprintf( stderr, "file_sys_closedir: unbalanced call\n" ) ;
    exit( EXITCODE_WRONG_CALL ) ;
  }
  DEB((stderr,"closing handle #%d\n", (int)stack_idx))
  file_sys_set_cwd( & (remember_dir [ stack_idx ] [0]) ) ;
  PRT_VAR(file_sys_get_cwd(),s)

#if defined _TURBO_C_ || _MS_C_
  /*intentionally left blank, no action needed*/

#elif __TINYC__
  status = _findclose( handle [ stack_idx ] ) ;
  if (status != 0 ) {
    fprintf( stderr, "file_sys_closedir: _findclose returns %d\n", status ) ;
    exit( EXITCODE_UNEXPECTED_ERR ) ;
  }

#elif __GNUC__
  status = closedir( dirp [ (int)stack_idx ] ) ;
  if (status != 0 ) {
    fprintf( stderr, "file_sys_closedir: closedir returns %d\n", status ) ;
    exit( EXITCODE_UNEXPECTED_ERR ) ;
  }

#else
#  error  file_sys.c: no compiler specified (closedir)
#endif

  stack_idx-- ;
  return ;
}

#ifndef __unix__
/*-------------------->   find_file   <-------------------------- 2016-Apr-05
This function looks for the given filename in the current working directory.
"Filename" may be case-insensitive.
-----------------------------------------------------------------------------
Used functions:
Globals:    --
Internals:  findinfo
Parameters: - filename   name of file to search (in cwd)
            - dest       where to store FILEINFO or NULL for internal static
            - flags      set FILE_SYS_IGNORE_CASE for DOS/Windows
Return value:   pointer to FILE_INFO or NULL if filename does not exist
Exitcode:       EXITCODE_UNEXPECTED_ERR
---------------------------------------------------------------------------*/
static FILE_SYS_FILEINFO* find_file( char* filename, FILE_SYS_FILEINFO* dest,
                                     U8 flags )
{
THIS_FUNC(find_file)
  FILE_SYS_FILEINFO* ret_val ;

#ifdef _MS_C_
  unsigned find_status ;
  struct find_t find_result ;

#elif __TINYC__
  int handle ;
  int findnext_status ;
  struct _finddata_t find_result ;

#elif __GNUC__
  /*left blank*/

#else
#  error  file_sys.c: no compiler specified (findfile)
#endif
  BIT found ; /*not used under Linux?*/


  ret_val = dest ;
  if ( dest == NULL ) {
    ret_val = & fileinfo ;
  }

#ifdef _MS_C_
  find_status = _dos_findfirst( "*.*", _A_ALL, & find_result ) ;
  while (find_status == 0) { /*matching file is found (i. e. ANY file)*/
    found = STR_I_CMP( flags, filename, find_result.name ) ;
    if (found) {
      findtype2fileinfo( ret_val, & find_result ) ;
      return ret_val ;
    }
    find_status = _dos_findnext( & find_result ) ;
  }
  return NULL ;

#elif __TINYC__
  found = FALSE ; /*default*/
  handle = _findfirst( "*.*", & find_result ) ;
  if (handle == -1) { /*could not open, no match*//*no need to close?*/
    return NULL ;
  }

  findnext_status = 0 ; /*default*/
  while (findnext_status == 0) {
    found = STR_I_CMP( flags, filename, find_result.name ) ;
    if (found) {
      findtype2fileinfo( ret_val, & find_result ) ;
      break ;
    }
    findnext_status = _findnext( handle, & find_result ) ;
  }

  findnext_status = _findclose( handle ) ;
  if (findnext_status != 0 ) {
    fprintf( stderr, "file_sys::find_file: _findclose returns %d\n",
                     findnext_status ) ;
    exit( EXITCODE_UNEXPECTED_ERR ) ;
  }
  return found ? ret_val : NULL ;

#elif __GNUC__
  fprintf( stderr, "Do not use find_file under Linux -- not implemented\n" ) ;
  exit(1) ;
  ret_val = NULL ;  /*to prevent gcc warnings*/
  return ret_val ;

#else
#  error  file_sys.c: no compiler specified (opendir)
#endif
}
#endif

/*-------------------->   findtype2fileinfo   <------------------ 2011-Mar-02
This function copies info from the compiler specific findtype structures
to FILE_SYS_FILEINFO.
-----------------------------------------------------------------------------
Used functions: strdup, mktime, localtime
Parameters:	- dest: FILE_SYS_FILEINFO to copy to
		- src: findtype structure to copy from
Return value:	void
Exitcode:	--
---------------------------------------------------------------------------*/
#ifdef _TURBO_C_
static void findtype2fileinfo( FILE_SYS_FILEINFO* dest, struct ffblk* src )
{
THIS_FUNC(findtype2fileinfo)
  struct tm temp ;

  dest->name = strdup( src->ff_name ) ;
  dest->size = (U32)(src->ff_fsize) ;
  dest->attr = (U8)(src->ff_attrib) ;
  dest->mod_time_year = (U16)(((src->ff_fdate) >> 9) + 1980) ;
  dest->mod_time_month = (U8)(((src->ff_fdate) >> 5) & 0xf) ;
  dest->mod_time_day =   (U8)( (src->ff_fdate) & 0x1f) ;
  dest->mod_time_hour =  (U8)( (src->ff_ftime) >> 11) ;
  dest->mod_time_min =   (U8)(((src->ff_ftime) >> 5) & 0x3f) ;
  dest->mod_time_sec =   (U8)(((src->ff_ftime) & 0x1f) << 1) ;

  temp.tm_sec = (int)(dest->mod_time_sec) ;
  temp.tm_min = (int)(dest->mod_time_min) ;
  temp.tm_hour = (int)(dest->mod_time_hour) ;
  temp.tm_mday = (int)(dest->mod_time_day) ;
  temp.tm_mon = (int)(dest->mod_time_month -1) ;
  temp.tm_year = (int)(dest->mod_time_year - 1900) ;
  dest->mod_time = mktime( & temp ) ;
}

#elif _MS_C_
static void findtype2fileinfo( FILE_SYS_FILEINFO* dest, struct find_t* src )
{
THIS_FUNC(findtype2fileinfo)
  struct tm temp ;

  dest->name = strdup( src->name ) ;
  dest->size = (U32)(src->size) ;
  dest->attr = (U8)(src->attrib) ;
  dest->mod_time_year = (U16)(((src->wr_date) >> 9) + 1980) ;
  dest->mod_time_month = (U8)(((src->wr_date) >> 5) & 0xf) ;
  dest->mod_time_day =   (U8)( (src->wr_date) & 0x1f) ;
  dest->mod_time_hour =  (U8)( (src->wr_time) >> 11) ;
  dest->mod_time_min =   (U8)(((src->wr_time) >> 5) & 0x3f) ;
  dest->mod_time_sec =   (U8)(((src->wr_time) & 0x1f) << 1) ;

  temp.tm_sec = (int)(dest->mod_time_sec) ;
  temp.tm_min = (int)(dest->mod_time_min) ;
  temp.tm_hour = (int)(dest->mod_time_hour) ;
  temp.tm_mday = (int)(dest->mod_time_day) ;
  temp.tm_mon = (int)(dest->mod_time_month -1) ;
  temp.tm_year = (int)(dest->mod_time_year - 1900) ;
  dest->mod_time = mktime( & temp ) ;
}

#else
static void findtype2fileinfo( FILE_SYS_FILEINFO* dest,
                               struct _finddata_t* src )
{
THIS_FUNC(findtype2fileinfo)
  struct tm* temp_p ;
  time_t temp_debug ;

  dest->name = strdup( src->name ) ;
  dest->size = (U32)(src->size) ;
  dest->attr = (U8)(src->attrib) ;
  dest->mod_time = (U32)(src->time_write) ;

  temp_p = localtime( & (src->time_write) ) ;
  dest->mod_time_year = (U16)((temp_p->tm_year) + 1900) ;
  dest->mod_time_month = (U8)((temp_p->tm_mon) + 1) ;
  dest->mod_time_day =   (U8)( temp_p->tm_mday) ;
  dest->mod_time_hour =  (U8)( temp_p->tm_hour) ;
  dest->mod_time_min =   (U8)( temp_p->tm_min) ;
  dest->mod_time_sec =   (U8)( temp_p->tm_sec) ;
}

#endif

/*-------------------->   x   <---------------------------------- 2016-Jul-22
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
/*x()*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/


/*--  test main  ----------------------------------------------------------*/

#ifdef TEST

   /*is this a regular/plain file?*/
   /*"attr_byte" is from FILE_SYS_FILEINFO*/
#define IS_REGULAR(attr_byte)  \
         ((attr_byte & (FILE_SYS_A_SUBDIR | FILE_SYS_A_VOLID)) == 0x0)

   /*is this a directory?*/
#define IS_DIR(attr_byte)  \
         ((attr_byte & FILE_SYS_A_SUBDIR ) == FILE_SYS_A_SUBDIR)

/*--------------------------------------------------------------*/
static void print_entry(struct dirent* p, char* indent )
  /*uses internal static struct dirent* dir_entry ;*/
{
  FILE_SYS_FILEINFO fileinfo ;


  file_sys_get_fileinfo( p->d_name, & fileinfo, FILE_SYS_DUP_NAME ) ;
  /*printf( "%s%s%s%s\n", indent, p->d_name,*/
  printf( "%s%s%s%s\n", indent, fileinfo.name,
          IS_REGULAR(fileinfo.attr) ? "(reg)":"",
              IS_DIR(fileinfo.attr) ? "(dir)":""
        ) ;
}

/*--------------------------------------------------------------*/
static void list_dir( char* dirname, char* indent )
{
  FILE_SYS_FILEINFO fileinfo ;
  int status ;
  char compose_indent [60] ;
  char* name ;


  strcpy( compose_indent, "  " ) ;
  strcat( compose_indent, indent ) ;


  stack_idx++ ;
  /*dirp [stack_idx] = opendir( "/common/projects/lib/file_sys/testdata" ) ;*/
  dirp [stack_idx] = opendir( dirname ) ;
  if (dirp [stack_idx] == NULL) {
    fprintf( stderr, "opendir(%s) returns NULL\n", dirname ) ;
    exit( 1 ) ;
  }
  if (file_sys_set_cwd( dirname ) ) { /*TRUE if we cannot goto dir*/
    fprintf( stderr, "cannot cwd\n" ) ;
    exit( 1 ) ;
  }

  dir_entry = readdir( dirp [ stack_idx ] ) ; /*1st read*/
  if (dir_entry == NULL) {
    fprintf( stderr, "Not even ./.. ???\n" ) ;
    exit( 1 ) ;
  }
  print_entry( dir_entry, indent ) ;

  while ( (dir_entry = readdir( dirp [ stack_idx ] )) != NULL ) {
    print_entry( dir_entry, indent ) ;
    name = dir_entry->d_name ;
    if (strcmp( name,  "." ) == 0) continue ;
    if (strcmp( name, ".." ) == 0) continue ;
    file_sys_get_fileinfo( name, & fileinfo, FILE_SYS_DUP_NAME ) ;
    if (IS_DIR(fileinfo.attr)) {
      list_dir( name, compose_indent ) ;
    }
  }
  if (file_sys_set_cwd( ".." ) ) { /*TRUE if we cannot goto dir*/
    fprintf( stderr, "cannot cwd to ..\n" ) ;
    exit( 1 ) ;
  }

  status = closedir( dirp [ stack_idx] ) ;
  if (status != 0 ) {
    fprintf( stderr, "file_sys_closedir: closedir returns %d\n", status ) ;
    exit( EXITCODE_UNEXPECTED_ERR ) ;
  }
  stack_idx-- ;
}

/*-------------------->   main   <------------------------------- 2016-Jul-22
Purpose see file header
-----------------------------------------------------------------------------
Used functions: x
Globals:   --
Internals: --
Parameters: - argc, argv
Return value: --
Exitcode:     EXITCODE_OK, EXITCODE_STANDARD_ERR
---------------------------------------------------------------------------*/
void main( int argc, char** argv )
{
THIS_FUNC(main)
  int status ;
  /*char indent [60] ;*/


  /*indent [0] = '\0' ;*/
  stack_idx = -1 ;


  list_dir( "/common/projects/lib/file_sys/testdata", "" ) ;

  /*if ( ) {*/
    /*printf( "Test \"xxx\" failed\n" ) ;*/
    /*exit( EXITCODE_STANDARD_ERR ) ;*/
  /*}*/

  /*printf( "\nAll tests passed successfully\n" ) ;*/
  exit( EXITCODE_OK ) ;
}
#endif /* TEST */

/*-------------------->   x   <---------------------------------- 2016-Jul-22
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
/*x()*/
/*{*/
/*THIS_FUNC(x)*/
/*}*/
/***************************************************************************/
