#!/bin/sh

dfcmplt="./rdb/dfcmplt"
pathfor="./rdb/pathfor"

cmd=$1
shift
root=$1
shift
attrs=$@
datafile=$($pathfor $root $attrs)
$dfcmplt $datafile || /bin/sh -c "$cmd" > $datafile
