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

def remove_emulation_prevention(rbsp):
    out = bytearray()
    i = 0
    while i < len(rbsp):
        if i+2 < len(rbsp) and rbsp[i] == 0 and rbsp[i+1] == 0 and rbsp[i+2] == 3:
            out += b'\x00\x00'
            i += 3
        else:
            out.append(rbsp[i])
            i += 1
    return bytes(out)

def find_rbsp_trailing_bits(rbsp):
    # rbsp_trailing_bits = 1 bit '1' + zero padding to byte boundary
    # ищем последний установленный бит в потоке
    total_bits = len(rbsp) * 8
    for bit_pos in reversed(range(total_bits)):
        byte_index = bit_pos // 8
        bit_index = 7 - (bit_pos % 8)
        if (rbsp[byte_index] >> bit_index) & 1:
            return byte_index, rbsp[byte_index]
    return None, None

def main(filename):
    with open(filename, "rb") as f:
        data = f.read()

    start_codes = list(find_start_codes(data))

    for idx, (offset, sc_len) in enumerate(start_codes):
        nal_start = offset + sc_len
        nal_end = start_codes[idx+1][0] if idx+1 < len(start_codes) else len(data)

        nal = data[nal_start:nal_end]
        if not nal:
            continue

        nal_type = nal[0] & 0x1F

        if nal_type == 5:  # IDR
            rbsp = remove_emulation_prevention(nal[1:])
            byte_index, byte_value = find_rbsp_trailing_bits(rbsp)

            if byte_index is not None:
                file_offset = nal_start + 1 + byte_index
                print(f"IDR at file offset: {offset:X}")
                print(f"  cabac_alignment byte offset: {file_offset:X}")
                print(f"  byte value: 0x{byte_value:02X}")
                print()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: script.py input.h264")
        sys.exit(1)
    main(sys.argv[1])
