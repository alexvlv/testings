#!/usr/bin/env python3
# GIT Rev.: $Format:%cd %cn %h %D$

import argparse
import struct

def validate(path):
    errors = 0
    values = 0
    expected = None
    zero_start = None
    zero_count = 0
    chunk_start = None
    chunk_count = 0
    chunk_end = None
    offset = 0

    def report_chunk():
        nonlocal chunk_start, chunk_count, chunk_end
        if chunk_count:
            print(f"0x{chunk_start:04X} ... 0x{chunk_end:04X}: "
                  f"Increment {chunk_count} values, 0x{chunk_start:04X} - 0x{chunk_end:04X}")
        chunk_start = None
        chunk_count = 0
        chunk_end = None

    def report_zeros():
        nonlocal zero_start, zero_count
        if zero_count:
            print(f"0x{zero_start:04X} ... 0x{zero_start:04X}: {zero_count} zeros")
        zero_start = None
        zero_count = 0

    with open(path, "rb") as f:
        while data := f.read(2):
            if len(data) != 2:
                print(f"ERROR: incomplete uint16 at offset 0x{offset:X}")
                errors += 1
                break

            value = struct.unpack("<H", data)[0]
            values += 1

            if value == 0:
                report_chunk()

                if zero_start is None:
                    zero_start = expected if expected is not None else 0
                zero_count += 1
                expected = None
                offset += 2
                continue

            if zero_count:
                report_zeros()
                expected = None

            if expected is not None and value != expected:
                print(f"ERROR: offset 0x{offset:X}: "
                      f"expected 0x{expected:04X}, got 0x{value:04X}")
                errors += 1
                report_chunk()

            if chunk_start is None:
                chunk_start = value

            chunk_count += 1
            chunk_end = value
            expected = (value + 1) & 0xFFFF
            offset += 2

    report_zeros()
    report_chunk()

    if errors:
        print(f"FAILED: {errors} error(s)")
        return 1

    print(f"OK: {values} values validated")
    return 0

def main():
    parser = argparse.ArgumentParser(
        description="Validate incrementing uint16 LE stream")
    parser.add_argument('-v', '--version', action='version', version='%(prog)s $Format:%%cd %%cn %%h %%D$')
    parser.add_argument("file")
    args = parser.parse_args()
    return validate(args.file)

if __name__ == "__main__":
    raise SystemExit(main())
