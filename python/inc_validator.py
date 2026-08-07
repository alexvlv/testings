#!/usr/bin/env python3
# GIT Rev.: $Format:%cd %cn %h %D$

import argparse
import struct

def validate(path):
    errors = 0
    expected = None
    offset = 0

    with open(path, "rb") as f:
        while data := f.read(2):
            if len(data) != 2:
                print(f"ERROR: incomplete uint16 at offset 0x{offset:X}")
                errors += 1
                break

            value = struct.unpack("<H", data)[0]

            if expected is not None and value != expected:
                print(f"ERROR: offset 0x{offset:X}: expected 0x{expected:04X}, got 0x{value:04X}")
                errors += 1

            expected = (value + 1) & 0xFFFF
            offset += 2

    if errors:
        print(f"FAILED: {errors} error(s)")
        return 1

    print(f"OK: {offset // 2} values validated")
    return 0

def main():
    parser = argparse.ArgumentParser(description="Validate incrementing uint16 LE stream")
    parser.add_argument("file")
    args = parser.parse_args()
    return validate(args.file)

if __name__ == "__main__":
    raise SystemExit(main())
