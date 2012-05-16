#!/bin/sh

w=50
h=50
n=25
lvlgen=/home/aifs2/eaburns/src/mid-game/cmd/lvlgen/lvlgen
pathfor=/home/aifs2/eaburns/src/search/rdb/pathfor
root=/home/aifs2/group/data/plat2d

for i in `seq $n`; do
	path=$($pathfor $root width=$w height=$w num=$i)
	echo "$lvlgen $w $h 1 -w -r > $path"
	$lvlgen $w $h 1 -w -r > $path
	sleep 1
done