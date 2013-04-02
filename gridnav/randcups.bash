#!/bin/bash

pathfor=/home/aifs2/eaburns/src/search_running/rdb/pathfor
cup=/home/aifs2/eaburns/doc/phd/dissertation/cup.template
root=/home/aifs2/group/data/grid_instances
keys="obstacles=eaburns-cups width=10000 height=1000"

for i in $(seq 1 25); do
	file=$($pathfor $root $keys num=$i)
	x=$RANDOM
	let "x %= 8000"
	y=$RANDOM
	let "y %= 1000"
	cat $cup > $file
	echo "Unit" >> $file
	echo "Four-way" >> $file
	echo $x $y 9999 500 >> $file
done