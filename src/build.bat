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
rem build.bat  for all libs and map_gen.exe                        2017-Mar-26
rem ==========================================================================


rem ============  set some variables  ========================================

rem        --> --> !!!!  READ  THIS  !!!! <-- <--
rem    Set TCC_PATH here if you don't have the tcc tools in your %PATH%.
rem set TCC_PATH=path to tcc tools, including the trailing backslash
rem    E. g.   "set TCC_PATH=C:\tinyC\"    if C:\tinyC\tcc.exe is the compiler
rem    Alternativly, you might set TCC_PATH in the environment
rem    before calling this script.


rem ============  check availitility of tcc.exe  =============================

rem   Check the availability of tcc
%TCC_PATH%tcc.exe -v > tcc_vers.txt
if exist tcc_vers.txt goto tcc_ok
echo ERROR: cannot find tcc.exe
echo Please read "readme_if_you_want_to_compile_the_sources.txt".

rem   Check if the reason might be a missing TCC_PATH
if "%TCC_PATH%" == "" goto tccp_empty
rem  TCC_PATH is not empty here.
echo Please check if TCC_PATH has been set to the correct directory.
goto failed

:tccp_empty
echo TCC_PATH is not set.
goto failed

:tcc_ok



rem ============  call all the other build scripts in proper order  ==========

rem            default exit status: failed
set MAP_GEN_BUILD_STATUS=failed
cd lib
call buildlib.bat %TCC_PATH%
cd ..
if not "%MAP_GEN_BUILD_STATUS%" == "success"  goto failed

set MAP_GEN_BUILD_STATUS=failed
cd lib\scan_lib
call buildlib.bat %TCC_PATH%
cd ..\..
if not "%MAP_GEN_BUILD_STATUS%" == "success"  goto failed

set MAP_GEN_BUILD_STATUS=failed
cd cpp
call build.bat %TCC_PATH%
cd ..
if not "%MAP_GEN_BUILD_STATUS%" == "success"  goto failed

set MAP_GEN_BUILD_STATUS=failed
cd map_gen\ri
call build_ri.bat
cd ..\..
if not "%MAP_GEN_BUILD_STATUS%" == "success"  goto failed

set MAP_GEN_BUILD_STATUS=failed
cd cevo_lib
call buildlib.bat %TCC_PATH%
cd ..
if not "%MAP_GEN_BUILD_STATUS%" == "success"  goto failed

set MAP_GEN_BUILD_STATUS=failed
cd map_gen
call build.bat %TCC_PATH%
cd ..
if not "%MAP_GEN_BUILD_STATUS%" == "success"  goto failed


rem     move executable to target dir
move /Y map_gen\map_gen.exe bin
echo "done successfully, map_gen executable has been moved into src\bin"

rem     map_gen.exe successfully built, skip "failed"
goto clean_up

:failed
echo compilation of map_gen failed
rem fall thru to clean_up

rem ============  clean up  ==================================================
:clean_up
set MAP_GEN_BUILD_STATUS=
rem set TCC_PATH=   keep TCC_PATH; it might have been supplied via environment
del tcc_vers.txt
