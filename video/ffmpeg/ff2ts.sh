#!/bin/sh

input="$1"
[ -s "$input" ] || exit

repack_to_ts() {
	outname="${input%.*}_out.ts"
	echo "$input => $outname"
	ffmpeg -i "$input" \
		-c:v copy \
		-f mpegts \
		-streamid 0:0x1011 \
		$outname
}
repack_to_ts

