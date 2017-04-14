readme_if_you_want_to_compile_the_sources.txt                      2017-Mar-22
==============================================================================

Map_gen compiles with
- gcc under Linux (I used gcc version 4.8.2)
- TinyC under Windows (I used TinyC version 0.9.25)
  You can get TinyC here:
    http://www.tinycc.org
    http://www.bellard.org
If you want to use another compiler read also the last section labelled
"If you want to use another compiler" at the bottom of this file.


Provided you have Linux/gcc or Windows/TinyC, you can compile the sources
as described below.  There are no makefiles; I use build scripts instead.


Windows only:
-------------
Please note that you must have the TinyC executables
 - tcc.exe
 - tiny_libmaker.exe
in a directory that is in your %PATH% list; if not, either add this directory
to the %PATH% list or edit "build.bat" and set the environment variable
%TCC_PATH% to the absolute path (including the trailing '\') where
the executables are located, so that the build script can find them.
See "build.bat" for an example.


Linux and Windows:
------------------
All you have to do is the following (instructions for Linux are
in parentheses):
1. Open a DOS box (a console) and cd to the "src" directory, that is to the
   directory where the file you are currently reading is located.
2. Type build.bat (./build)

That's all.  The executable "map_gen.exe" ("map_gen")
will be located in the "src\bin" ("src/bin") directory.



If you want to use another compiler:
------------------------------------
Think twice about it.  Getting and installing TinyC is much easier than to
follow the (possibly incomplete) instructions below.
Gcc should come with your Linux distro.

I didn't test other compilers, so I cannot be of much help here.
The bad news is: You have to edit parts of the source code.
You should also read the
"readme_if_you_want_to_understand_the_source_code.txt" file.

There is at least one file which you have to change,
   src\include\compiler.h.
In the #if/#elif structure you find there, add an #elif clause suitable for
your compiler, similar to the code for the other compilers you will see there.
You'll have to typedef U8, S8, U16, S16, U32, S32 and BIT.
You'll have to define TRUE and FALSE.
I'm not sure about the DOUBLE_* define statements, maybe you can omit them.

Besides the source code, you will have to change the build scripts to use
your compiler.  If you're lucky, that's all.

Depending on your compiler you might have to edit the following files, too
(be warned: this list might be incomplete)
- src\include\debug.h
- src\lib\file_sys\file_sys.c
because I know these files  *do*  contain compiler dependent code,
which might or might not fit your compiler.

If you run into serious trouble, please contact me.  If I can help you,
I will do so.
