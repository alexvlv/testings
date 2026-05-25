#!/usr/bin/env python3
# MPEG-TS raw stream extractor by PID
# GIT Rev.: $Format:%cd %cn %h %D$

import argparse
import logging
import mmap
import os
import struct
import bitstruct

TS_SZ = 188
TS_DATA_SZ = 184
TS_HDR_START_FLD = 2
TS_HDR_PID_FLD = 4
TS_HDR_ADAPT_FLD = 6
TS_HDR_CONT_FLD = 7


class ExtractPid:
    def __init__(self, pid, fout):
        self.pid = pid
        self.fout = fout
        self.pid_packets_cnt = 0
        self.extracted_packets_cnt = 0
        self.bytes_cnt = 0
        self.continuity = -1

    def __call__(self, packet, offset):
        self.packet = packet
        self.offset = offset
        self.head = bitstruct.unpack('>u8u1u1u1u13u2u2u4', self.packet[0:4])
        if not self.check_pid():
            return
        self.check_continuity()
        self.extract()
        self.pid_packets_cnt += 1

    def check_pid(self):
        # print("{:04X}".format(self.head[TS_HDR_PID_FLD],))
        return self.head[TS_HDR_PID_FLD] == self.pid

    def check_continuity(self):
        if self.continuity < 0:
            self.continuity = self.head[TS_HDR_CONT_FLD]
            return
        if (self.continuity + 1) & 0x0F != self.head[TS_HDR_CONT_FLD]:
            lost = self.head[TS_HDR_CONT_FLD] - self.continuity - 1
            if lost < 0:
                lost += 16
            log.warning("Warning: stream broken at 0x{0:X}({0}): {1:X} => {2:X}, {3} lost".format(
                self.offset, self.continuity, self.head[TS_HDR_CONT_FLD], lost))
        self.continuity = self.head[TS_HDR_CONT_FLD]
        return self.head[TS_HDR_CONT_FLD]

    def extract(self):
        # Adaptation field control
        # 01 – no adaptation field, payload only,
        # 10 – adaptation field only, no payload,
        # 11 – adaptation field followed by payload,
        # 00 – RESERVED for future use [10]
        # https://en.wikipedia.org/wiki/MPEG_transport_stream#Packet
        flg = self.head[TS_HDR_ADAPT_FLD]
        if flg == 0b01:
            flst = self.head[TS_HDR_START_FLD]
            # ToDo: PES header parser here!
            ofst = 14 if flst else 0 # Dirty hack: skip PES header with PTS stamp
            #log.debug("Start {0}, offset {1}".format(flst, ofst))
            self.write(ofst)
        elif flg == 0b11:
            payload_offset = 1 + self.packet[4]
            if payload_offset >= TS_DATA_SZ:
                log.warning("Warning: bad packet at 0x{0:X}({0}): adaptation size: {1:02X}({1})".format(self.offset, self.packet[5],))
                return
            self.write(payload_offset)
            pass

    def write(self, offset):
        self.fout.write(self.packet[4 + offset:])
        self.bytes_cnt += TS_DATA_SZ - offset
        self.extracted_packets_cnt += 1


class Parser:
    def __init__(self, fin, functor):
        self.functor = functor
        self.fin = fin
        self.inmm = mmap.mmap(fin.fileno(), length=0, access=mmap.ACCESS_READ)
        self.fsize = self.inmm.size()

    def process(self):
        next_pos = 0
        packet_cnt = 0
        while True:
            position = self.find_packet(next_pos)
            if position < 0:
                remain = self.fsize - next_pos
                if remain > 0:
                    last = next_pos - 1
                    log.debug("Last packet end at 0x{0:X}({0}), remain {1} bytes".format(last, remain))
                break
            if position != next_pos:
                delta = position - next_pos
                log.warning("Warning: skipped {1} bytes at 0x{0:X}({0}), restored at 0x{2:X}({2})".format(next_pos, delta, position))
            self.functor(self.inmm[position:position + TS_SZ],position)
            next_pos = position + TS_SZ
            packet_cnt += 1
        log.info("Processed {0} packets total".format(packet_cnt,))
        return True

    def find_packet(self, start=0):
        pos = start
        end = self.fsize - TS_SZ
        while(pos < end):
            beg_chr = self.inmm[pos]
            end_chr = self.inmm[pos + TS_SZ]
            if beg_chr == 0x47 and end_chr == 0x47:
                #print("{0}: {1:02X}=>{2:02X}".format(pos,beg_chr,end_chr))
                break
            pos += 1
        if pos > end:
            pos = -1
        return pos


def create_out_fname(ifname, pid):
    base = os.path.splitext(ifname)[0]
    return '{0}-{1}.es'.format(base, pid)


def main():
    parser = argparse.ArgumentParser(description='MPEG-TS raw stream extractor')
    parser.add_argument('-p', '--pid', help='Stream pid [1100]', default='1100')
    parser.add_argument('-i', '--input', help='Input file [mpeg.ts]', default='mpeg.ts')
    parser.add_argument('-o', '--output', help='Output file')
    parser.add_argument('-l', '--loglevel', help='Log level [DEBUG]', default='DEBUG')
    parser.add_argument('-v', '--version', action='version', version='%(prog)s GIT Rev.: $Format:%cd %cn %h %D$')
    args = parser.parse_args()

    logging.basicConfig(level=args.loglevel, format='%(message)s')
    global log
    log = logging.getLogger()

    log.info("MPEG-TS raw stream extractor, GIT $Id$")
    try:
        pid = int(args.pid, 16)
    except ValueError as err:
        log.error("Error: PID must be valid HEX value [{0}]".format(err))
        return

    infilename = args.input
    try:
        len = os.path.getsize(infilename)
    except OSError as err:
        print("Error: Input file:[{0}] is not readable: {1}".format(infilename, err, ))
        return
    except BaseException as err:
        print("Input file:[{0}] get size error: {1}".format(infilename, err, ))
        return

    outfname = args.output if args.output else create_out_fname(infilename, args.pid)
    log.info("Input file:[{0}] {3} bytes \nPID:{2:04X} \nOutput file:[{1}]".format(infilename, outfname, pid, len))

    if len < TS_SZ:
        log.error("Error: Input file [{0}] too small! [{0} bytes]".format(infilename, len,))
        return

    with open(infilename, 'rb') as fin, open(outfname, 'wb') as fout:
        extractor = ExtractPid(pid, fout)
        parser = Parser(fin, extractor)
        parser.process()
        log.info("Found {0}, extracted {1} packets, {2} bytes".format(
            extractor.pid_packets_cnt, extractor.extracted_packets_cnt, extractor.bytes_cnt))
        pass


if __name__ == '__main__':
    main()

