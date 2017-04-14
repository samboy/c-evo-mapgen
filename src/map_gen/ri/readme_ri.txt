readme_ri.txt                                                      2017-Mar-30
==============================================================================
This directory contains files which do the configuration for the read_ini
module, i. e. the definition of all parameters which may appear in
"map_gen.ini".

"read_ini.ini" is the only file that needs to be edited when you want to add
a new parameter.  Simply add a new line similar to the others you will find
there.  Each parameter has a
 - name
 - type (string, integer)
 - default value   which is used when the parameter
                   doesn't show up in "map_gen.inin"
 - minimum value
 - maximum value

The minimum/maximum values are verified by the read_ini module;
a syntax error is raised when the value in "map_gen.ini" is outside
that range.  String variables are not checked against the min/max values.

-----------------------------------
For each parameter specified in "read_ini.ini", there is a second global
variable generated which tells whether the parameter did show up in
"map_gen.ini".  This variable has the same name, but preceeded with "found_".

    Example:
    You placed the following line into "read_ini.ini"
    READ_INI_ITEM( my_param, U32, 1234, 1000, 2000 )

    A global variable named "my_param" (type: unsigned 32 bits) is filled with
    the default value 1234 during runtime.
    A global variable named "found_my_param" (type: boolean) is filled with
    TRUE during runtime if "my_param" showed up in "map_gen.ini", else FALSE.
    If the value given in "map_gen.ini" is >=2001 or <=999, a syntax error is
    raised.  Otherwise the value is written into "my_param".
    NOTE: Never define a parameter starting with "found_".  That might lead
    to compilation errors.

By this mechanism, the scenarios can tell whether a parameter was given or not.
If it was not given, the default value as defined in "read_ini.ini" might be
replaced by a scenario-specific default value.

-----------------------------------
The definition file "read_ini.ini" is included by the three .c files
which define the macro READ_INI_ITEM in an appropriate way.
These files are preprocessed and their output (.i files) are moved to
the .. directory where they are included by map_gen during the compilation
process.
