#!/usr/bin/env python3
"""
UDP Packet Sender - Sends timestamped 512-byte packets in pairs to two ports.
Transmits at a controlled bitrate (Mbps).

Usage:
  ./udp2ports_alisa.py [--ip IP] [--ports PORT1 PORT2] [--bitrate MBPS]

Defaults:
  --ip 192.168.1.254
  --ports 4000 5000
  --bitrate 1.0
"""

import argparse
import socket
import time
from datetime import datetime
import sys



def generate_packet():
    """Create a 512-byte packet with timestamp (ms) + padding."""
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]  # ms precision
    padding = " " * (512 - len(timestamp))
    packet = (timestamp + padding).encode('utf-8')
    assert len(packet) == 512, "Packet size error: {} bytes".format(len(packet))
    return packet



def send_packet_pairs(ip, ports, bitrate_mbps):
    """
    Send one packet to each port in sequence, then pause based on bitrate.

    Args:
        ip (str): Destination IP
        ports (list): Two port numbers [port1, port2]
        bitrate_mbps (float): Transmission rate in Mbps
    """
    # Setup UDP sockets
    sockets = []
    for port in ports:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sockets.append((sock, port))

    # Calculate transmission interval
    packet_size = 512  # bytes per packet
    total_bytes_per_pair = packet_size * 2
    bitrate_bps = bitrate_mbps * 1_000_000  # Mbps → bps
    bytes_per_second = bitrate_bps / 8  # bps → B/s
    interval = total_bytes_per_pair / bytes_per_second  # seconds between pairs

    print("IP: {}".format(ip))
    print("Ports: {}, {}".format(ports[0], ports[1]))
    print("Bitrate: {} Mbps".format(bitrate_mbps))
    print("Packet size: {} bytes".format(packet_size))
    print("Interval between pairs: {:.6f} s".format(interval))
    print("Starting transmission... (Ctrl+C to stop)")

    try:
        while True:
            # Send one packet per port
            for sock, port in sockets:
                packet = generate_packet()
                sock.sendto(packet, (ip, port))
                sys.stdout.write(".")
                sys.stdout.flush()  # Immediate output

            time.sleep(interval)  # Wait before next pair

    except KeyboardInterrupt:
        print("\nTransmission stopped by user.")
    finally:
        for sock, _ in sockets:
            sock.close()



def main():
    parser = argparse.ArgumentParser(
        description="Send UDP packet pairs at a specified bitrate",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=""
    )

    parser.add_argument(
        "--ip",
        type=str,
        default="192.168.1.254",
        help="Destination IP address (default: 192.168.1.254)"
    )
    parser.add_argument(
        "--ports",
        type=int,
        nargs=2,
        default=[4000, 5000],
        help="Two destination ports (default: 4000 5000)"
    )
    parser.add_argument(
        "--bitrate",
        type=float,
        default=1.0,
        help="Transmission rate in Mbps (default: 1.0)"
    )

    args = parser.parse_args()

    # Validate exactly two ports
    if len(args.ports) != 2:
        parser.error("Exactly two ports must be provided.")

    send_packet_pairs(args.ip, args.ports, args.bitrate)



if __name__ == "__main__":
    main()
