#!/bin/sh

w=25
h=25
n=25
lvlgen=/home/aifs2/eaburns/src/c/mid-game/cmd/lvlgen/lvlgen
pathfor=/home/aifs2/eaburns/src/c/search/utils/pathfor
root=/home/aifs2/group/data/plat2d

for i in `seq $n`; do
	path=$($pathfor $root width=$w height=$w num=$i)
	echo "$lvlgen $w $h 1 -w > $path"
	$lvlgen $w $h 1 -w > $path
	sleep 1
done