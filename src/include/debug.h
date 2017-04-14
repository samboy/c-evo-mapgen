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
debug.h
*****************************************************************************
Aenderungen:
2013-Apr-08: changed ASSERT macros for better readability (assert failed: ...)
2011-Mar-03: added GNU-C (not complete yet)
2010-Jul-14: added TRACE_ON_COND, TRACE_PUSH/POP
2010-May-14: added TIME_STAMP
2010-Apr-14: added more TinyC macros
2010-Apr-13: added: "debug_pause"
2010-Apr-10: added TinyC macros (not complete yet)
2010-Apr-03: added PAUSE
2007-Sep-29: - debug_trace_flag wird von ENTRY, EXIT und DEB beachtet
             - TRACE_OFF wird nur geschrieben, wenn TRACE vorher ON war
2006-Jul-08: TRACE_ON, TRACE_OFF, <misc.h>
22.10.96: anpassen an Unix
12.04.96: ggf. <process.h> includen
11.04.96: Anpassung an MS-C
20.02.89: Ersterstellung
****************************************************************************/
#ifndef DEBUGH
#define DEBUGH

#ifndef DEBUG
/*define all debug keywords to expand to nothing*/
/*Exception: "RETURN"*/

# define ENTRY
# define EXIT
# define THIS_FUNC(name)
# define DEB(msg)
# define DEB_STATEMENT(statement)
# define ASSERT(cond)
# define ASSERT_ALT(cond, statement)
# define PRT_VAR(var, form)
# define PRT_COND(cond, var, form)
# define REDIRECT(filename)
# define RETURN(ret_val,fmt) return (ret_val) ;
# define TRACE_ON
# define TRACE_OFF
# define TRACE_ON_COND(cond)
# define TRACE_PUSH
# define TRACE_POP
# define PAUSE
# define TIME_STAMP(msg)

/*-------------------------------------------------------------------------*/
#else
# include <stdio.h>
# include <misc.h>
# if defined _MS_C_ || defined _TURBO_C_
#  include <process.h>
# endif

  static char* _this_func = "???" ;
  extern BIT debug_trace_flag ;
  void trace_push( void ) ;
  void trace_pop( void ) ;
  void debug_pause( char* func_name ) ;
  void debug_time_stamp( char* msg ) ;

# define DEB_STATEMENT(statement) statement
# define ENTRY if(debug_trace_flag){fprintf(stderr,"Entry %s\n", _this_func);}
# define EXIT if(debug_trace_flag){fprintf(stderr, "Exit %s\n", _this_func) ;}
# define TRACE_ON   debug_trace_flag = TRUE ;
# define TRACE_OFF  if(debug_trace_flag){fprintf(stderr,"%s: TRACE_OFF\n\n", _this_func);} debug_trace_flag = FALSE;
# define TRACE_ON_COND(cond)   if (cond) debug_trace_flag = TRUE ;
# define TRACE_PUSH debug_trace_push() ;
# define TRACE_POP debug_trace_pop() ;
# define PAUSE  debug_pause( _this_func) ;
# define TIME_STAMP(msg)  if(debug_trace_flag){debug_time_stamp( msg ) ;}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
# if defined _MANX_
#  define THIS_FUNC(name) char* _this_func = "name" ;
#  define ASSERT(cond) if(!(cond)){fprintf(stderr, "%s: assert failed: (cond)\n", _this_func) ; exit(1) ; }
#  define ASSERT_ALT(cond,statement) if(!(cond)){fprintf(stderr, "%s: assert failed: (cond)\n", _this_func); statement exit(1) ; }
#  define PRT_VAR(var, fmt) fprintf(stderr, "%s: var = %fmt\n", _this_func, var);
#  define PRT_COND(cond,var,fmt) if(cond)fprintf(stderr,"%s: var = %fmt\n",_this_func,var);
#  define REDIRECT(filename) if(fclose(stderr)){printf("DEBUG: fclose(stderr)\n");exit(1);}if(fopen(filename,"w")==NULL){printf("DEBUG: fopen(%s, \"w\")");exit(1);}
#  define RETURN(ret_val,fmt) fprintf(stderr,"%s: ret.val. = >%fmt<\n",_this_func,ret_val);return(ret_val);

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
# elif defined _MS_C_ || defined _TURBO_C_
#  define THIS_FUNC(name) char* _this_func = #name ;
#  define ASSERT(cond)\
  if ( ! (cond)) {\
    fprintf(stderr, "%s: assert failed: %s\n", _this_func, #cond);\
    exit(1) ;\
  }
#  define ASSERT_ALT(cond,statement)\
  if ( ! (cond)) {\
    fprintf(stderr, "%s: assert failed: %s\n", _this_func, #cond);\
    statement \
    exit(1) ;\
  }
#  define PRT_VAR_AUX(var,fmt_str)\
  if (debug_trace_flag) {  \
    fprintf( stderr, #fmt_str, _this_func, var) ;  \
  }
#  define PRT_VAR(var, fmt) PRT_VAR_AUX(var,%s: var = %fmt\n)
#  define PRT_COND(cond,var,fmt)   if (cond) PRT_VAR_AUX(var,%s: var = %fmt\n)
#  define REDIRECT(filename)\
  if (fclose(stderr)) {\
    printf("DEBUG: fclose(stderr)\n");\
    exit(1);\
  }\
  if (fopen(filename,"w")==NULL) {\
    printf("DEBUG: fopen(%s, \"w\")", filename );\
    exit(1);\
  }
#  define RETURN_AUX(ret_val,fmt_str)\
  fprintf( stderr, #fmt_str, _this_func, ret_val) ;
#  define RETURN(ret_val,fmt)\
  RETURN_AUX(ret_val,%s: ret.val. = >%fmt<\n)\
  return(ret_val);

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
# elif defined __TINYC__ || defined __GNUC__
#  define THIS_FUNC(name) char* _this_func = #name ;
#  define PRT_VAR_AUX(var,fmt_str)\
  if (debug_trace_flag) {  \
    fprintf( stderr, #fmt_str, _this_func, var, '\n') ;  \
  }
#  define PRT_VAR(var, fmt) PRT_VAR_AUX(var,%s: var = %fmt%c)
#  define PRT_COND(cond,var,fmt)   if (cond) PRT_VAR_AUX(var,%s: var = %fmt%c)
#  define ASSERT(cond)\
  if ( ! (cond)) {\
    fprintf(stderr,"%s: assert failed: %s\n",_this_func,#cond);\
    exit(1) ;\
  }
#  define ASSERT_ALT(cond,statement)\
  if ( ! (cond)) {\
    fprintf(stderr, "%s: assert failed: %s\n", _this_func, #cond);\
    statement \
    exit(1) ;\
  }
#  define REDIRECT(filename)\
  if (fclose(stderr)) {\
    printf("DEBUG: fclose(stderr)\n");\
    exit(1);\
  }\
  if (fopen(filename,"w")==NULL) {\
    printf("DEBUG: fopen(%s, \"w\")", filename );\
    exit(1);\
  }
#  define RETURN_AUX(ret_val,fmt_str)\
  fprintf( stderr, #fmt_str, _this_func, ret_val,'\n') ;
#  define RETURN(ret_val,fmt)\
  RETURN_AUX(ret_val,%s: ret.val. = >%fmt<%c)\
  return(ret_val);

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
# else /* richtige Compiler */
#  define THIS_FUNC(name) char* _this_func = "name" ;
#  define ASSERT(cond)\
  if ( ! (cond)) {\
    fprintf(stderr,"%s: assert failed: %s\n",_this_func,"cond");\
    exit(1) ;\
  }
#  define ASSERT_ALT(cond,statement)\
  if ( ! (cond)) {\
    fprintf(stderr,"%s: assert failed: %s\n",_this_func,"cond");\
    statement \
    exit(1) ;\
  }
#  define PRT_VAR(var, fmt)\
  fprintf(stderr, "%s: var = %fmt\n", _this_func, var);
#  define PRT_COND(cond,var,fmt)\
  if (cond)\
    fprintf(stderr,"%s: var = %fmt\n",_this_func,var);
#  define REDIRECT(filename)\
  if (fclose(stderr)) {\
    printf("DEBUG: fclose(stderr)\n");\
    exit(1);\
  }\
  if (fopen(filename,"w")==NULL) {\
    printf("DEBUG: fopen(%s, \"w\")", filename );\
    exit(1);\
  }
#  define RETURN(ret_val,fmt)\
  fprintf(stderr,"%s: ret.val. = >%fmt<\n",_this_func,ret_val);\
  return(ret_val);
# endif

# define DEB(msg) if(debug_trace_flag){fprintf(stderr,"%s: ",_this_func);fprintf msg;}
# define DEB_STATEMENT(statement) statement

#endif /* ! DEBUG */
/***************************************************************************/
#endif /* DEBUGH */
/***************************************************************************/
