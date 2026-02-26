#!/bin/sh

input="$1"
[ -s "$input" ] || exit

to_raw_h264() {
	outname="${input%.*}.h264"
	echo "$input => $outname"

	ffmpeg -i "$input" \
		-map 0:v:0 -an -sn \
		-vf "scale=720:576" \
		-c:v libx264 -profile:v high \
		-bf 0 \
		-pix_fmt yuv420p \
		-f h264 $outname
}

to_raw_h264 
