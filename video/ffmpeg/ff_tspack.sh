#!/bin/sh

input="$1"
[ -s "$input" ] || exit

file_ext=${filename##*.} 
outname="${input%.*}.out.ts"
	echo "$input => $outname"

pack_h264() {
	ffmpeg -i "$input" \
		-c:v copy \
		-bsf:v "h264_mp4toannexb,dump_extra" \
		-mpegts_flags resend_headers \
		-f mpegts \
		-streamid 0:0x1011 \
		$outname
}
#pack_h264

pack_h264_ex() {
	ffmpeg \
		-fflags +genpts \
		-i "$input" \
		-c:v copy \
		-bsf:v "h264_mp4toannexb,dump_extra" \
		-muxdelay 0 \
		-muxpreload 0 \
		-mpegts_flags resend_headers \
		-mpegts_pcr_pid 0x1011 \
		-streamid 0:0x1011 \
		-f mpegts \
		$outname
}
pack_h264_ex
