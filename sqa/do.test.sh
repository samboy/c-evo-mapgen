#!/bin/sh

cp ../map_gen.ini .
../map_gen -1
MD5=$( md5sum "nav_req.cevo map" | awk '{print $1}' )

if [ "$MD5" -ne "7753c0263aabd4d304de6eb98d1e4cfa" ] ; then
	echo Map gen test FAIL
	exit 1
fi

echo Map gen test PASS
rm "nav_req.cevo map"
rm map_gen.ini
exit 0
 
