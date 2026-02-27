#!/bin/sh

input="$1"
[ -s "$input" ] || exit

extract_h264() {
	outname="${input%.*}.h264"
	echo "$input => $outname"
	ffmpeg -i "$input" \
		-map 0:v:0 -an -sn \
		-err_detect ignore_err \
		-c:v copy \
		-bsf:v h264_mp4toannexb \
		-f h264 \
		$outname
}
extract_h264
