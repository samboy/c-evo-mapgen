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
*	misc.h
*****************************************************************************
History: (latest change first)
2013-Aug-17: added EQV(a,b)
2012-Jun-29: added XOR(a,b)
2012-May-13: removed "abs" (name clash with MS-C library)
2011-Dec-16: added "abs" and "fabs"
2003-Jun-29: NULL defined
2002-Oct-26: include compiler.h
05.04.98: include exitcode.h
21.04.96: min(), max(), clip()
03.04.96: Ersterstellung
****************************************************************************/
#ifndef MISC_H
#define MISC_H
/***************************************************************************/

#include <compiler.h>
#include <exitcode.h>


/***  Constants  ***********************************************************/

#ifndef TRUE
# define TRUE (-1)
#endif
#ifndef FALSE
# define FALSE 0
#endif

#ifndef NULL
# define NULL 0
#endif


/***  Macros  **************************************************************/

        /*integer abs*/
/* collides with MS-C "abs" in "math.h"
#ifndef abs
# define abs(x)  (((x) >= 0) ? (x) : (-(x)))
#endif
*/

        /*float/double abs*/
/* collides with tcc "fabs" in "math.h"
#ifndef fabs
# define fabs(x)  (((x) >= 0.0) ? (x) : (-(x)))
#endif
*/

#ifndef min
# define min(a,b) ((a) < (b) ? (a):(b))
#endif

#ifndef max
# define max(a,b) ((a) > (b) ? (a):(b))
#endif

#ifndef XOR
# define XOR(a,b) (((a) && ! (b) ) || ( ! (a) && (b)))
#endif

#ifndef EQV
# define EQV(a,b) (((a) && (b) ) || ( ! (a) && ! (b)))
#endif

#ifndef clip
# define clip(x,low,high) ((x) < (low) ? (low):((x) > (high) ? (high):(x)))
#endif

/***************************************************************************/
#endif	/* MISC_H */
/***************************************************************************/
