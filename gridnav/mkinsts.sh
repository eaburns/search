#!/bin/sh
#
# Make a bunch of seed-instances.
#

width=5000
height=5000
root=/home/aifs2/group/data/grid_instances
pathfor=/home/aifs2/eaburns/src/search/rdb/pathfor
mkseedinst=/home/aifs2/eaburns/src/search/gridnav/mkseedinst

for num in `seq 1 1000`; do
	for costs in Unit Life; do
		lifecost=""
		if test "$costs" = "Life"; then
			lifecost="-lifecost"
		fi

		for moves in Eight-way Four-way; do
			prob=0.35
			eightway=""
			if test "$moves" = "Eight-way"; then
				prob=0.45
				eightway="-eightway"
			fi
	
			out=$($pathfor $root obstacles=uniform type=cpp-seedinst width=$width height=$height prob=$prob costs=$costs moves=$moves num=$num)

			echo $out
			$mkseedinst -width $width -height $height -prob $prob $lifecost $eightway > $out
		done
	done
done