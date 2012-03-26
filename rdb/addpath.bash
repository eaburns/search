#!/bin/bash
#
# Grabs the path attributes and adds them to each datafile.
#

rdb=./rdb
pathattrs=$rdb/pathattrs
addattrs=$rdb/addattrs

for file in $(find $1 -type f -not -name KEY*); do
	keys=$($pathattrs $file)
	echo $file $keys
	$addattrs $file $keys
done