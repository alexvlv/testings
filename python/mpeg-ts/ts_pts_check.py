#!/usr/bin/env python3
# MPEG-TS PTS analyzer
# GIT Rev.: $Format:%cd %cn %h %D$

import argparse
import logging
import mmap
import os

TS_SZ = 188
SYNC = 0x47

STREAM_TYPE_RAW_AUDIO = 0x84

# MPEG-TS stream_type values.
STREAM_TYPES = {
    0x01: 'V',       # MPEG-1 video
    0x02: 'V',       # MPEG-2 video
    0x10: 'V',       # MPEG-4 video
    0x1B: 'V',       # H.264/AVC
    0x24: 'V',       # H.265/HEVC
    0x25: 'V',       # H.265 MVC

    0x03: 'A',       # MPEG-1 audio
    0x04: 'A',       # MPEG-2 audio
    0x0F: 'A',       # AAC
    0x11: 'A',       # AAC LATM
    0x81: 'A',       # AC-3
    0x87: 'A',       # E-AC-3

    0x84: 'A',       # DaVinci raw audio, S16_LE stereo 16 kHz
}

# PES stream_id ranges.
PES_STREAM_VIDEO = 0xE0
PES_STREAM_AUDIO = 0xC0


class TsParser:
    def __init__(self, fin, analyzer):
        self.analyzer = analyzer
        self.fin = fin
        self.inmm = mmap.mmap(fin.fileno(), length=0, access=mmap.ACCESS_READ)
        self.fsize = self.inmm.size()

    def process(self):
        pos = 0
        packets = 0

        while pos + TS_SZ <= self.fsize:
            if self.inmm[pos] != SYNC:
                pos += 1
                continue

            if pos + TS_SZ < self.fsize:
                if self.inmm[pos + TS_SZ] != SYNC:
                    pos += 1
                    continue

            self.analyzer.packet(
                self.inmm[pos:pos + TS_SZ],
                pos)

            pos += TS_SZ
            packets += 1

        log.info("Processed %d TS packets", packets)


class TsAnalyzer:
    def __init__(self):
        self.pmt_pid = None
        self.streams = {}
        self.pes = {}
        self.pts_state = {}

    def packet(self, packet, offset):
        if packet[0] != SYNC:
            return

        b1, b2, b3 = packet[1:4]

        pusi = bool(b1 & 0x40)
        pid = ((b1 & 0x1F) << 8) | b2
        adaptation = (b3 >> 4) & 3

        if adaptation == 0:
            return

        pos = 4

        if adaptation in (2, 3):
            length = packet[pos]
            pos += 1 + length

        if adaptation == 2 or pos >= TS_SZ:
            return

        payload = packet[pos:]

        if pid == 0:
            self.parse_pat(payload, pusi)

        elif pid == self.pmt_pid:
            self.parse_pmt(payload, pusi)

        elif pid in self.streams:
            self.parse_pes(pid, payload, pusi, offset)

    def parse_pat(self, payload, pusi):
        if not pusi or not payload:
            return

        pointer = payload[0]
        pos = 1 + pointer

        if pos + 8 > len(payload):
            return

        if payload[pos] != 0x00:
            return

        section_length = (
            ((payload[pos + 1] & 0x0F) << 8) |
            payload[pos + 2])

        end = min(
            pos + 3 + section_length - 4,
            len(payload))

        pos += 8

        while pos + 4 <= end:
            program = (
                (payload[pos] << 8) |
                payload[pos + 1])

            pid = (
                ((payload[pos + 2] & 0x1F) << 8) |
                payload[pos + 3])

            if program != 0:
                self.pmt_pid = pid
                log.info("PMT PID 0x%04X", pid)
                return

            pos += 4

    def parse_pmt(self, payload, pusi):
        if not pusi or not payload:
            return

        pointer = payload[0]
        pos = 1 + pointer

        if pos + 12 > len(payload):
            return

        if payload[pos] != 0x02:
            return

        section_length = (
            ((payload[pos + 1] & 0x0F) << 8) |
            payload[pos + 2])

        end = min(
            pos + 3 + section_length - 4,
            len(payload))

        program_info_length = (
            ((payload[pos + 10] & 0x0F) << 8) |
            payload[pos + 11])

        pos += 12 + program_info_length

        while pos + 5 <= end:
            stream_type = payload[pos]

            pid = (
                ((payload[pos + 1] & 0x1F) << 8) |
                payload[pos + 2])

            es_info_length = (
                ((payload[pos + 3] & 0x0F) << 8) |
                payload[pos + 4])

            media_type = STREAM_TYPES.get(
                stream_type, '?')

            self.streams[pid] = {
                'stream_type': stream_type,
                'type': media_type,
            }

            log.info(
                "Found PID 0x%04X, stream_type 0x%02X (%s)",
                pid,
                stream_type,
                media_type)

            pos += 5 + es_info_length

    def parse_pes(self, pid, payload, pusi, offset):
        if pusi:
            self.finish_pes(pid)

            if len(payload) < 9:
                return

            if payload[0:3] != b'\x00\x00\x01':
                return

            stream_id = payload[3]

            flags = payload[7]
            header_length = payload[8]

            pts = None

            if flags & 0x80:
                if len(payload) < 14:
                    return

                pts = self.decode_pts(payload[9:14])

            # PES header occupies 9 + header_length bytes.
            header_size = 9 + header_length

            if header_size > len(payload):
                return

            # Prefer PES stream_id classification.
            media_type = self.classify_stream_id(stream_id)

            if media_type is None:
                media_type = self.streams[pid]['type']

            self.pes[pid] = {
                'type': media_type,
                'size': len(payload) - header_size,
                'pts': pts,
                'stream_id': stream_id,
                'offset': offset,
            }

            return

        pes = self.pes.get(pid)

        if pes:
            pes['size'] += len(payload)

    def finish_pes(self, pid):
        pes = self.pes.pop(pid, None)

        if not pes:
            return

        # We are interested only in PES packets having PTS
        # and recognized as audio/video.
        if pes['pts'] is None:
            return

        media_type = pes['type']

        if media_type not in ('A', 'V'):
            return

        pts = pes['pts']

        if media_type not in self.pts_state:
            self.pts_state[media_type] = {
                'first': pts,
                'previous': None,
            }

        state = self.pts_state[media_type]

        pts_ms = (pts - state['first']) / 90.0

        if state['previous'] is None:
            delta = None
        else:
            delta = (pts - state['previous']) / 90.0

        if delta is None:
            print(
                "{0} {1:6d} {2:8.3f}    -".format(
                    media_type,
                    pes['size'],
                    pts_ms))
        else:
            print(
                "{0} {1:6d} {2:8.3f} {3:+8.3f}".format(
                    media_type,
                    pes['size'],
                    pts_ms,
                    delta))

        state['previous'] = pts

    @staticmethod
    def classify_stream_id(stream_id):
        if 0xE0 <= stream_id <= 0xEF:
            return 'V'

        if 0xC0 <= stream_id <= 0xDF:
            return 'A'

        return None

    @staticmethod
    def decode_pts(data):
        return (
            (((data[0] >> 1) & 0x07) << 30) |
            (data[1] << 22) |
            ((data[2] >> 1) << 15) |
            (data[3] << 7) |
            (data[4] >> 1))

    def finish(self):
        for pid in list(self.pes):
            self.finish_pes(pid)


def main():
    parser = argparse.ArgumentParser(
        description='MPEG-TS PTS analyzer')

    parser.add_argument(
        '-i', '--input',
        default='mpeg.ts',
        help='Input TS file [mpeg.ts]')

    parser.add_argument(
        '-l', '--loglevel',
        default='INFO',
        help='Log level [INFO]')

    parser.add_argument(
        '-v', '--version',
        action='version',
        version='%(prog)s GIT Rev.: '
                '$Format:%cd %cn %h %D$'.replace('%', '%%'))

    args = parser.parse_args()

    logging.basicConfig(
        level=args.loglevel,
        format='%(message)s')

    global log
    log = logging.getLogger()

    infilename = args.input

    try:
        size = os.path.getsize(infilename)
    except OSError as err:
        log.error(
            "Error: Input file [%s] is not readable: %s",
            infilename, err)
        return 1

    if size < TS_SZ:
        log.error(
            "Error: Input file [%s] too small: %d bytes",
            infilename, size)
        return 1

    log.info(
        "MPEG-TS PTS analyzer, input: %s, size: %d bytes",
        infilename, size)

    with open(infilename, 'rb') as fin:
        analyzer = TsAnalyzer()
        TsParser(fin, analyzer).process()
        analyzer.finish()

    return 0


if __name__ == '__main__':
    main()
