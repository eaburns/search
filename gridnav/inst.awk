BEGIN {
	x0=""
	y0=""
	x1=""
	y1=""
	FS="\""
}
/#pair  "start x"	".*"/ { x0 = $4 }
/#pair  "start y"	".*"/ { y0 = $4 }
/#pair  "goal x"	".*"/ { x1 = $4 }
/#pair  "goal y"	".*"/ { y1 = $4 }
END {
	print x0 " " y0 " " x1 " " y1
}