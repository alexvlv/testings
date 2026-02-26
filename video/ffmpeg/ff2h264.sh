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
#to_raw_h264 


#-g 25 -keyint_min 25 -sc_threshold 0 - fixed GOP 25 (not nrcessary for DaVinci decoder)

to_ts() {
	outname="${input%.*}.ts"
	echo "$input => $outname"

	ffmpeg -i "$input" \
		-map 0:v:0 -an -sn \
		-vf "scale=720:576" \
		-c:v libx264 -profile:v high \
		-bf 0 \
		-g 25 -keyint_min 25 -sc_threshold 0 \
		-pix_fmt yuv420p \
		-f mpegts \
		-streamid 0:0x1011 \
		$outname
}
to_ts

# DvDecad - OK
