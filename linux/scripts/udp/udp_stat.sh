#!/bin/sh

while true; do
awk '
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
        printf "[%s] InDatagrams: %s | InErrors: %s | RcvbufErrors: %s\n", timestamp, $in_idx, $err_idx, $rcv_idx
        exit
    }
}' /proc/net/snmp
sleep 1
done
