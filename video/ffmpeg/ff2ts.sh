#!/bin/sh

input="$1"
[ -s "$input" ] || exit

repack_to_ts() {
	outname="${input%.*}_out.ts"
	echo "$input => $outname"
	ffmpeg -i "$input" \
		-err_detect ignore_err \
		-c:v copy \
		-bsf:v h264_mp4toannexb \
		-f mpegts \
		-streamid 0:0x1011 \
		$outname
}
repack_to_ts

#		-bsf:v "h264_mp4toannexb, h264_metadata=replace" \
