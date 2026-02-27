#!/usr/bin/env python3
import sys

def find_start_codes(data):
    i = 0
    while i < len(data) - 3:
        if data[i:i+3] == b'\x00\x00\x01':
            yield i, 3
            i += 3
        elif data[i:i+4] == b'\x00\x00\x00\x01':
            yield i, 4
            i += 4
        else:
            i += 1

def patch_idr_alignment(data):
    data = bytearray(data)
    start_codes = list(find_start_codes(data))

    for idx, (offset, sc_len) in enumerate(start_codes):
        nal_start = offset + sc_len
        nal_end = start_codes[idx+1][0] if idx+1 < len(start_codes) else len(data)

        if nal_start >= len(data):
            continue

        nal_type = data[nal_start] & 0x1F

        if nal_type == 5:  # IDR
            payload_start = nal_start + 1
            payload_end = nal_end

            if payload_end <= payload_start:
                continue

            last_byte_offset = payload_end - 1

            original = data[last_byte_offset]
            data[last_byte_offset] |= 0x01  # force LSB to 1
            patched = data[last_byte_offset]

            print(f"IDR at offset {offset}")
            print(f"  Patched byte at {last_byte_offset}: "
                  f"0x{original:02X} -> 0x{patched:02X}")

    return bytes(data)

def main(infile, outfile):
    with open(infile, "rb") as f:
        data = f.read()

    patched = patch_idr_alignment(data)

    with open(outfile, "wb") as f:
        f.write(patched)

    print(f"\nSaved patched file to: {outfile}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: script.py input.h264 output.h264")
        sys.exit(1)

    main(sys.argv[1], sys.argv[2])
