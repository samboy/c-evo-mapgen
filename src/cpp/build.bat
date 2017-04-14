@echo off
rem --------------------------------------------------------------------------
rem This file is part of map_gen, an external random map generator for C-Evo
rem Copyright (C) 2017  Ulrich Krueger
rem
rem Map_gen is free software: you can redistribute it and/or modify
rem it under the terms of the GNU General Public License as published by
rem the Free Software Foundation, either version 3 of the License, or
rem (at your option) any later version.
rem
rem Map_gen is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem GNU General Public License for more details.
rem
rem You should have received a copy of the GNU General Public License
rem along with map_gen.  If not, see <http://www.gnu.org/licenses/>.
rem
rem --------------------------------------------------------------------------
rem build.bat  for cpp.exe                                         2017-Mar-26
rem ==========================================================================
rem An optional %1 parameter is the path prefix for tcc tools.
rem Since batch scripts cannot return an exit code, an environment
rem variable (MAP_GEN_BUILD_STATUS) is set appropriately.
rem ==========================================================================

rem         Overwrite an existing TCC_PATH only if %1 is set
if "%1" == "" goto no_par1
set TCC_PATH=%1
:no_par1

rem            default exit status: failed
set MAP_GEN_BUILD_STATUS=failed


rem ============  set some variables  ========================================

rem        --> --> !!!!  READ  THIS  !!!! <-- <--
rem    Set TCC_PATH here if you want to use this script as a stand-alone
rem    script and you don't have the tcc tools in your %PATH%.
rem set TCC_PATH=path to tcc tools, including the trailing backslash
rem    E. g.   set TCC_PATH=C:\tinyC\     if C:\tinyC\tcc.exe is the compiler
rem    Alternativly, you might call this script with an apropriate path
rem    as positional parameter %1, like src\build.bat does it.

set TCC_OPTIONS=-Wall -Werror -I..\include -L..\lib
set TCC_CMD=%TCC_PATH%tcc.exe %TCC_OPTIONS%


rem ============  compile  ===================================================

%TCC_CMD% cpp_main.c cpp_getc.c inc_stk.c mac_tab.c -o cpp.exe -lmisc
if errorlevel 1 goto clean_up

set MAP_GEN_BUILD_STATUS=success
echo cpp.exe built successfully


rem ============  clean up  ==================================================
:clean_up
set TCC_OPTIONS=
set TCC_CMD=
