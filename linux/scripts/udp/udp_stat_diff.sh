#!/bin/sh

last=0
while true; do
read timestamp InDatagrams InErrors RcvbufErrors <<EOF
$(awk '
/^Udp:/ {
    if ($2 == "InDatagrams") {
        for(i=2; i<=NF; i++) {
            if ($i == "InDatagrams") in_idx = i
            if ($i == "InErrors") err_idx = i
            if ($i == "RcvbufErrors") rcv_idx = i
        }
    } else {
        # Format the current time
        timestamp = strftime("%H:%M:%S")
        # Print time followed by the UDP metrics
        printf "%s %s %s %s\n", timestamp, $in_idx, $err_idx, $rcv_idx
        exit
    }
}' /proc/net/snmp)
EOF
if [ $last -ne $InErrors ]; then
	logger -t UDP -s -p WARN  "$timestamp UDP InDatagrams: $InDatagrams InErrors: $InErrors RcvbufErrors: $RcvbufErrors"
	last=$InErrors
fi
sleep 1
done
