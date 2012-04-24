#!/bin/sh
#
# Compute the dependencies of a file
# From "Recursive Make Considered Harmful,"
#	Peter Miller 2008
#
PROG="$1"
DIR="$2"
shift 2

case "$DIR" in
"" | "." )
	$PROG -MM "$@" | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@'
	;;
* )
	shift 1
	$PROG -MM "$@" | sed -e "s@^\(.*\)\.o:@$DIR/\1.d $DIR/\1.o:@"
	;;
esac
