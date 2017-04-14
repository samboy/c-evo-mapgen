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
/* table.c */

#define CONCATENATE(a,b)  a/**/b

typedef struct {
  /*char* name ;*/
  char* type ;
  void* var ;
  BIT*  flag ;
  U32 minv ;
  U32 maxv ;
} RI_TAB_ITEM ;

#define READ_INI_ITEM(name, type, def, minv, maxv)  \
  { ___MAKE_STR(type), & name, & CONCATENATE(found_,name), minv, maxv },

static RI_TAB_ITEM ri_table [] = {
#include "read_ini.ini"
  { "", NULL, NULL, 0, 0 } /*dummy entry*/
} ;
#undef READ_INI_ITEM


#define READ_INI_ITEM(name, type, def, minv, maxv)   ___MAKE_STR(name),

static char* name_tab [] = {
#include "read_ini.ini"
  "" /*dummy entry*/
} ;
static const S32 ri_entries = (sizeof(name_tab) / sizeof(char*)) -1 ;
