#!/usr/bin/env python3
"""
udp_xor_proxy.py

Simple UDP proxy that obfuscates/deobfuscates traffic with a repeating XOR key.

Behavior:
- Listens on the given local address:port for client UDP packets.
- For each distinct client address, creates one ephemeral UDP socket used to
  communicate with the remote target. That way replies from the remote server
  are forwarded back to the original client.
- Applies XOR(key) to bytes in both directions (symmetric).

Usage example:
    python3 udp_xor_proxy.py --listen 0.0.0.0:12345 --remote 10.0.0.5:4000 --key "secret"
"""

import argparse
import socket
import threading
import time
from typing import Dict, Tuple

# Default idle timeout (seconds) after which client->ephemeral socket is closed
CLIENT_IDLE_TIMEOUT = 120.0
CLEANER_INTERVAL = 10.0
BUF_SIZE = 65535


def parse_hostport(s: str) -> Tuple[str, int]:
    if ":" not in s:
        raise argparse.ArgumentTypeError("must be HOST:PORT")
    host, port_s = s.rsplit(":", 1)
    return host, int(port_s)


def xor_bytes(data: bytes, key: bytes) -> bytes:
    if not key:
        return data
    klen = len(key)
    return bytes(b ^ key[i % klen] for i, b in enumerate(data))


class ClientSession:
    """
    Holds ephemeral socket and bookkeeping for a single client address.
    A thread reads from the ephemeral socket and forwards responses back to client.
    """

    def __init__(self, client_addr: Tuple[str, int], remote_addr: Tuple[str, int],
                 key: bytes, proxy_sock: socket.socket):
        self.client_addr = client_addr
        self.remote_addr = remote_addr
        self.key = key
        self.proxy_sock = proxy_sock  # main listening socket to forward to client
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(1.0)
        # bind to ephemeral port (OS chooses)
        self.sock.bind(("0.0.0.0", 0))
        self.last_activity = time.time()
        self.running = True
        self.thread = threading.Thread(target=self._reader, daemon=True)
        self.thread.start()

    def send_to_remote(self, data: bytes):
        """Obfuscate and send to remote server from ephemeral socket."""
        outbound = xor_bytes(data, self.key)
        try:
            self.sock.sendto(outbound, self.remote_addr)
            self.last_activity = time.time()
        except OSError:
            # ignore transient errors
            pass

    def _reader(self):
        """Read replies from remote, deobfuscate and forward to client via proxy_sock."""
        while self.running:
            try:
                data, addr = self.sock.recvfrom(BUF_SIZE)
            except socket.timeout:
                continue
            except OSError:
                break
            # we expect addr == remote_addr in normal use, but we don't strictly check
            inbound = xor_bytes(data, self.key)
            try:
                self.proxy_sock.sendto(inbound, self.client_addr)
                self.last_activity = time.time()
            except OSError:
                pass

    def close(self):
        self.running = False
        try:
            self.sock.close()
        except OSError:
            pass
        # join thread briefly
        self.thread.join(timeout=1.0)


class UDPXorProxy:
    def __init__(self, listen_addr: Tuple[str, int], remote_addr: Tuple[str, int], key: bytes,
                 client_timeout: float = CLIENT_IDLE_TIMEOUT):
        self.listen_addr = listen_addr
        self.remote_addr = remote_addr
        self.key = key
        self.client_timeout = client_timeout

        self.proxy_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # allow immediate restart
        self.proxy_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # bind
        self.proxy_sock.bind(self.listen_addr)

        # client_addr -> ClientSession
        self.sessions: Dict[Tuple[str, int], ClientSession] = {}
        self.lock = threading.Lock()
        self.running = True

        self.cleaner_thread = threading.Thread(target=self._cleaner, daemon=True)
        self.cleaner_thread.start()

    def _cleaner(self):
        while self.running:
            time.sleep(CLEANER_INTERVAL)
            now = time.time()
            with self.lock:
                stale = [c for c, s in self.sessions.items() if now - s.last_activity > self.client_timeout]
                for c in stale:
                    s = self.sessions.pop(c)
                    print(f"[cleaner] closing session for {c}")
                    s.close()

    def serve_forever(self):
        print(f"Listening on {self.listen_addr[0]}:{self.listen_addr[1]} -> forwarding to {self.remote_addr[0]}:{self.remote_addr[1]}")
        try:
            while self.running:
                try:
                    data, client_addr = self.proxy_sock.recvfrom(BUF_SIZE)
                except OSError:
                    break
                # get or create session for this client
                with self.lock:
                    sess = self.sessions.get(client_addr)
                    if sess is None:
                        sess = ClientSession(client_addr, self.remote_addr, self.key, self.proxy_sock)
                        self.sessions[client_addr] = sess
                        print(f"[proxy] new session {client_addr} -> {self.remote_addr} (ephemeral port {sess.sock.getsockname()[1]})")
                # Forward obfuscated packet to remote using session's ephemeral socket
                sess.send_to_remote(data)
        except KeyboardInterrupt:
            pass
        finally:
            self.shutdown()

    def shutdown(self):
        self.running = False
        print("Shutting down proxy...")
        with self.lock:
            for s in self.sessions.values():
                s.close()
            self.sessions.clear()
        try:
            self.proxy_sock.close()
        except OSError:
            pass
        # allow cleaner to exit
        self.cleaner_thread.join(timeout=1.0)
        print("Proxy stopped.")


def main():
    p = argparse.ArgumentParser(description="Simple UDP obfuscation proxy (XOR).")
    p.add_argument("--listen", "-l", type=parse_hostport, default=("0.0.0.0", 12345),
                   help="listen HOST:PORT (default 0.0.0.0:12345)")
    p.add_argument("--remote", "-r", type=parse_hostport, required=True,
                   help="remote HOST:PORT to forward to (example 10.0.0.5:4000)")
    p.add_argument("--key", "-k", type=str, default="", help="XOR key string or hex (prefix 0x).")
    p.add_argument("--timeout", type=float, default=CLIENT_IDLE_TIMEOUT, help="client idle timeout seconds")
    args = p.parse_args()

    # interpret key: hex if starts with 0x, else take utf-8 bytes
    key_bytes = b""
    if args.key:
        if args.key.startswith("0x") or all(c in "0123456789abcdefABCDEF" for c in args.key) and len(args.key) % 2 == 0:
            try:
                key_bytes = bytes.fromhex(args.key[2:] if args.key.startswith("0x") else args.key)
            except ValueError:
                key_bytes = args.key.encode("utf-8")
        else:
            key_bytes = args.key.encode("utf-8")

    proxy = UDPXorProxy(args.listen, args.remote, key_bytes, client_timeout=args.timeout)
    proxy.serve_forever()


if __name__ == "__main__":
    main()
