#!/bin/sh

# usbdev-filter.sh (compact mode)
# Sat 17 Jan 2026 12:41:24 +03

vidpid=""
compact=0
tree=0

while getopts "d:ct" opt; do
	case "$opt" in
		d) vidpid="$OPTARG" ;;
		c) compact=1 ;;
		t) tree=1 ;;
	esac
done

usb-devices | awk -v vidpid="$vidpid" -v compact="$compact" -v tree="$tree" '
BEGIN {
	block=""
	hit=0
}

function flush() {
	if (block=="") return

if (vidpid=="" || hit) {
    # parse block as before
    bus=""; dev=""; drv=""; spd=""; port=""; vid=""; pid=""
    n = split(block, lines, "\n")
    for (j=1;j<=n;j++) {
        l = lines[j]
        if (l ~ /^T:/) {
            if (match(l,/Bus=([0-9]+)/)) bus=substr(l,RSTART+4,RLENGTH-4)
            if (match(l,/Dev#=[ ]*([0-9]+)/)) dev=substr(l,RSTART+6,RLENGTH-6)
            if (match(l,/Spd=([0-9]+)/)) spd=substr(l,RSTART+4,RLENGTH-4)
        }
        if (l ~ /^P:/) {
            if (match(l,/Vendor=([0-9a-f]+)/)) vid=substr(l,RSTART+7,RLENGTH-7)
            if (match(l,/ProdID=([0-9a-f]+)/)) pid=substr(l,RSTART+7,RLENGTH-7)
        }
        if (l ~ /^I:/ && l ~ /Driver=/) {
            if (match(l,/Driver=([a-z0-9_]+)/)) drv=substr(l,RSTART+7,RLENGTH-7)
            if (match(l,/Port=([0-9]+)/)) port=substr(l,RSTART+5,RLENGTH-5)
        }
    }

    if (tree==1) {
        if (bus!="") {
            printf "Bus %s\n", bus
            if (port=="") port="?"
            printf " └─ Port %s → %s:%s (%s, %sM)\n", port, vid, pid, drv, spd
        } else {
            printf "%s:%s (%s, %sM)\n", vid, pid, drv, spd
        }
    } else if (compact==1) {
        line = "Bus " bus " Dev " dev " " vid ":" pid " " drv " " spd "M " port
        printf "%s\n", line
    } else {
        printf "%s\n", block
    }
}

	block=""
	hit=0
}


{
	if ($0=="") {
		flush()
		next
	}

	block = block $0 "\n"

	if (vidpid!="" && $0 ~ /^P:/) {
		split(vidpid,a,":")
		line = tolower($0)
		if (line ~ "vendor=" tolower(a[1]) && line ~ "prodid=" tolower(a[2]))
			hit=1
	}
}

END { flush() }
'
