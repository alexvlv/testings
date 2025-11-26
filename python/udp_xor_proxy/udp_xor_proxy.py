#!/usr/bin/env python3
import argparse
import socket
import threading

BUF_SIZE = 65535

def parse_hostport(s: str):
    """
    Accepts:
      PORT
      HOST:PORT
    Returns (host or None, port)
    """
    # Pure port (digits only)
    if s.isdigit():
        return None, int(s)

    # HOST:PORT
    if ":" in s:
        host, port_s = s.rsplit(":", 1)
        if not port_s.isdigit():
            raise argparse.ArgumentTypeError("port must be digits")
        if host == "":
            raise argparse.ArgumentTypeError("host cannot be empty; use just PORT for default host")
        return host, int(port_s)

    raise argparse.ArgumentTypeError("must be PORT or HOST:PORT")


def xor_bytes(data: bytes, key: bytes) -> bytes:
    if not key:
        return data
    k = len(key)
    return bytes(b ^ key[i % k] for i, b in enumerate(data))


def main():
    p = argparse.ArgumentParser(description="Bidirectional UDP XOR proxy")
    p.add_argument("-l", "--listen", type=parse_hostport,
                   default=("0.0.0.0", 12345),
                   help="Listen PORT or HOST:PORT (default 0.0.0.0:12345)")
    p.add_argument("-r", "--remote", type=parse_hostport, required=True,
                   help="Remote PORT or HOST:PORT (port-only means localhost)")
    p.add_argument("-k", "--key", default="", help="XOR key (string)")
    args = p.parse_args()

    key = args.key.encode()

    # Apply host defaults
    listen_host, listen_port = args.listen
    remote_host, remote_port = args.remote

    if listen_host is None:
        listen_host = "0.0.0.0"
    if remote_host is None:
        remote_host = "127.0.0.1"

    # Local listener socket
    local = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    local.bind((listen_host, listen_port))

    # Remote socket (ephemeral bind)
    remote = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    remote.bind(("0.0.0.0", 0))

    print(f"Listening on {listen_host}:{listen_port}  <->  {remote_host}:{remote_port}")

    remote_addr = (remote_host, remote_port)
    state = {"client_addr": None}  # store last client for remote->local forwarding

    def local_to_remote():
        while True:
            data, client_addr = local.recvfrom(BUF_SIZE)
            state["client_addr"] = client_addr
            obf = xor_bytes(data, key)
            remote.sendto(obf, remote_addr)

    def remote_to_local():
        while True:
            data, _ = remote.recvfrom(BUF_SIZE)
            client = state["client_addr"]
            if client:
                deobf = xor_bytes(data, key)
                local.sendto(deobf, client)

    threading.Thread(target=local_to_remote, daemon=True).start()
    threading.Thread(target=remote_to_local, daemon=True).start()

    # Keep process alive forever
    threading.Event().wait()


if __name__ == "__main__":
    main()
