#!/usr/bin/env python3

import argparse
import binascii
import random
import struct
import sys
import time

SYNC = 0xD391DA26
TYPE = 1
MAX_DATA = 128

def make_packet(data, bad_crc=False, bad_sync=False, bad_size=False):
    size = len(data) ^ 1 if bad_size else len(data)
    sync = SYNC ^ 1 if bad_sync else SYNC
    hdr = struct.pack("<IBB", sync, TYPE, size)
    crc = binascii.crc_hqx(hdr + data, 0)
    if bad_crc:
        crc ^= 1
    return hdr + data + struct.pack("<H", crc)

def make_data(counter, size, rnd):
    if rnd:
        return random.randbytes(size) if hasattr(random, "randbytes") else bytes(random.getrandbits(8) for _ in range(size)), counter
    data = bytearray()
    for _ in range((size + 1) // 2):
        data += struct.pack("<H", counter)
        counter = (counter + 1) & 0xffff
    return bytes(data[:size]), counter

def run(args):
    out = sys.stdout.buffer
    counter = 0
    pkt = 0
    packet = None

    while args.count == 0 or pkt < args.count:
        pkt += 1

        if packet is None or not args.repeat:
            data, counter = make_data(counter, args.size, args.random)
            packet = make_packet(
                data,
                bad_crc=args.bad_crc and pkt % args.bad_crc == 0,
                bad_sync=args.bad_sync and pkt % args.bad_sync == 0,
                bad_size=args.bad_size and pkt % args.bad_size == 0)

        out.write(packet)
        out.flush()

        if args.delay:
            time.sleep(args.delay / 1000)

def main():
    parser = argparse.ArgumentParser(
        description="UART protocol test packet generator",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument("-n", "--count", type=int, default=0,
        help="number of packets (0 = infinite)")
    parser.add_argument("-s", "--size", type=int, default=MAX_DATA,
        help="payload size, bytes")
    parser.add_argument("-d", "--delay", type=float, default=0,
        help="delay between packets, ms")
    parser.add_argument("-r", "--random", action="store_true",
        help="generate random payload")
    parser.add_argument("--bad-crc", type=int, default=0, metavar="N",
        help="corrupt every Nth CRC")
    parser.add_argument("--bad-sync", type=int, default=0, metavar="N",
        help="corrupt every Nth SYNC")
    parser.add_argument("--bad-size", type=int, default=0, metavar="N",
        help="corrupt every Nth SIZE field")
    parser.add_argument("-R", "--repeat", action="store_true",
        help="repeat the first packet")
    args = parser.parse_args()

    if not 0 <= args.size <= MAX_DATA:
        parser.error(f"size must be in range 0..{MAX_DATA}")

    run(args)

if __name__ == "__main__":
    main()
