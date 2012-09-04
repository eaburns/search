#!/bin/sh

w=50
h=50
n=100
mkinst=/home/aifs2/eaburns/src/search/segments/mkinst
pathfor=/home/aifs2/eaburns/src/search/rdb/pathfor
root=/home/aifs2/group/data/segments

for nsegs in `seq 3`; do
	for i in `seq $n`; do
		path=$($pathfor $root type=instance number of segments=$nsegs width=$w height=$w num=$i)
		echo "$mkinst -width $w -height $h -nsegs $nsegs > $path"
		$mkinst -width $w -height $h -nsegs $nsegs > $path || exit 1
	done
done