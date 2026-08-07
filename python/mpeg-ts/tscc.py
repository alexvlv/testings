#!/usr/bin/env python3
# GIT Rev.: $Format:%cd %cn %h %D$

import argparse
import sys

TS_SIZE = 188
NULL_PID = 0x1fff

def packet_info(pkt):
    if len(pkt) != TS_SIZE or pkt[0] != 0x47:
        return None
    pid = ((pkt[1] & 0x1f) << 8) | pkt[2]
    afc = (pkt[3] >> 4) & 3
    cc = pkt[3] & 0xf
    discontinuity = False
    if afc in (2, 3) and pkt[4]:
        discontinuity = bool(pkt[5] & 0x80)
    return pid, afc, cc, discontinuity

def has_payload(afc):
    return afc in (1, 3)

def scan(filename, show_null):
    last = {}
    errors = 0
    with open(filename, "rb") as f:
        packet = 0
        while True:
            offset = packet * TS_SIZE
            data = f.read(TS_SIZE)
            if not data:
                break
            if len(data) != TS_SIZE:
                print(f"0x{offset:08x} truncated packet ({len(data)} bytes)", file=sys.stderr)
                break
            info = packet_info(data)
            if info is None:
                print(f"0x{offset:08x} invalid TS packet", file=sys.stderr)
                packet += 1
                continue
            pid, afc, cc, discontinuity = info
            if pid == NULL_PID:
                if show_null:
                    print(f"0x{offset:08x} {offset:10d} {packet:10d} 0x{pid:03x}  {cc:2}     NULL")
                packet += 1
                continue
            if not has_payload(afc):
                packet += 1
                continue
            if pid in last and not discontinuity:
                prev = last[pid]
                expected = (prev + 1) & 0xf
                if cc != expected:
                    delta = (cc - expected) & 0xf
                    if delta == 0xf:
                        status = "DUP 1"
                    else:
                        status = f"LOST {delta}"
                    print(f"0x{offset:08x} {offset:10d} {packet:10d} 0x{pid:03x}  {prev}->{cc:<2}  {status}")
                    errors += 1
            last[pid] = cc
            packet += 1
    return errors

def main():
    parser = argparse.ArgumentParser(description="MPEG-TS continuity counter scanner")
    parser.add_argument("-n", "--null", action="store_true", help="show null packets (PID 0x1FFF)")
    GIT_VERSION = '$Format:%cd %cn %h %D$'.replace('%', '%%')
    parser.add_argument('-v', '--version', action='version',  version='%(prog)s GIT Rev.: ' + GIT_VERSION)
    parser.add_argument("file")
    args = parser.parse_args()
    print("OFFSET       DEC_OFFSET       PACKET       PID    CC      STATUS")
    return 1 if scan(args.file, args.null) else 0

if __name__ == "__main__":
    sys.exit(main())
