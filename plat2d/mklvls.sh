#!/bin/sh

w=100
h=100
n=10
lvlgen=/home/aifs2/eaburns/src/mid-game/cmd/lvlgen/lvlgen
pathfor=/home/aifs2/eaburns/src/search/rdb/pathfor
root=/home/aifs2/group/data/plat2d

for i in `seq $n`; do
	path=$($pathfor $root width=$w height=$w type=instance num=$i)
	echo "$lvlgen $w $h 1 -w -r > $path"
	$lvlgen $w $h 1 -w -r > $path || exit 1
	sleep 1
done