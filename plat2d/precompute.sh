#!/bin/sh
#
# Finds platform instances without pre-computed heuristics and pre-computes them.
#
search=/home/aifs2/eaburns/src/search
makeh=$search/plat2d/makeh
withattrs=$search/rdb/withattrs
pathattrs=$search/rdb/pathattrs
pathfor=$search/rdb/pathfor
root=/home/aifs2/group/data/plat2d

for path in $($withattrs $root  type=instance); do
	attrs=$($pathattrs $path | sed 's/instance/heuristic/')
	hpath=$($pathfor $root $attrs)
	if test -e $hpath; then
		continue
	fi
	echo -n "$path\n\t"
	$makeh $hpath < $path
done