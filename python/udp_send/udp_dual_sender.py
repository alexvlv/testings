#!/usr/bin/env python3

import socket
import threading
import argparse
import time
from datetime import datetime

HOST = "192.168.1.254"
PORTS = [4000, 5000]
PACKET_SIZE = 512   # bytes

def pad_packet(data: str, size: int) -> str:
    """Pad packet to fixed size."""
    b = data.encode()
    if len(b) >= size:
        return b[:size].decode(errors='ignore')
    return (b + b' ' * (size - len(b))).decode()

def send_udp(port, payload):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.sendto(payload.encode(), (HOST, port))
    s.close()

def main():
    parser = argparse.ArgumentParser(
        description="Send two simultaneous 512-byte UDP packets per iteration.")
    parser.add_argument("bitrate_mbit", type=float,
                        help="Target bitrate in Mbit/s")

    args = parser.parse_args()

    # Bits per iteration = 2 packets * 512 bytes * 8
    bits_per_iteration = 2 * PACKET_SIZE * 8

    # Convert bitrate to iterations per second
    bitrate_bits = args.bitrate_mbit * 1_000_000
    iteration_rate = bitrate_bits / bits_per_iteration

    if iteration_rate <= 0:
        raise ValueError("Bitrate too low")

    period = 1.0 / iteration_rate

    print(f"Target bitrate: {args.bitrate_mbit} Mbit/s")
    print(f"Iterations per second: {iteration_rate:.2f}")
    print(f"Sending 2×512B packets to {HOST}:{PORTS}")
    print("Press Ctrl+C to stop\n")

    try:
        while True:
            # timestamp + padding
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
            payload = pad_packet(timestamp, PACKET_SIZE)

            threads = []
            for port in PORTS:
                t = threading.Thread(target=send_udp, args=(port, payload))
                t.start()
                threads.append(t)

            for t in threads:
                t.join()

            time.sleep(period)

    except KeyboardInterrupt:
        print("\nStopped.")

if __name__ == "__main__":
    main()
