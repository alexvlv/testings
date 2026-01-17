#!/bin/sh
# 2026-01-17T12:20Z

vidpid=""

while getopts "d:" opt; do
	case "$opt" in
		d) vidpid="$OPTARG" ;;
	esac
done

usb-devices | awk -v vidpid="$vidpid" '
BEGIN {
	block = ""
	hit = 0
}

function flush() {
	if (block == "")
		return

	if (vidpid == "" || hit)
		printf "%s\n", block

	block = ""
	hit = 0
}

{
	if ($0 == "") {
		flush()
		next
	}

	block = block $0 "\n"

	if (vidpid != "" && $0 ~ /^P:/) {
		split(vidpid, a, ":")
		line = tolower($0)
		if (line ~ "vendor=" tolower(a[1]) &&
		    line ~ "prodid=" tolower(a[2]))
			hit = 1
	}
}

END {
	flush()
}
'
