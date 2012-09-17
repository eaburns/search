#!/bin/sh
#
# Output the WIDTH and HEIGHT -D arguments
# given a tiles board size 8, 15, 24, etc.
#
sz=$(echo "scale=0; sqrt($1+1)" | bc)
echo -DWIDTH=$sz -DHEIGHT=$sz