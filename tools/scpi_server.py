#!/usr/bin/env python3
"""A small TCP SCPI device simulator for DevTerm."""

import argparse
import math
import re
import socketserver
import time


MAX_COMMAND_BYTES = 4096

RESPONSES = {
    "DEV": "DevTerm SCPI Simulator",
    "VER": "10.234",
    "SER": "SIM000001",
}
MEASUREMENTS = {
    "MEAS:CURR": (0.5, 0.25),
    "MEAS:VOLT": (12.0, 2.0),
    "MEAS:POW": (6.0, 1.5),
}
REQUEST = re.compile(r"\*([A-Z]+(?::[A-Z]+)*)\s*\?", re.ASCII)


def response_for(command, elapsed):
    match = REQUEST.fullmatch(command.strip().upper())
    if not match:
        return ":ERROR(0x2004): Unknown SCPI-Kommand."
    name = match.group(1)
    if name in RESPONSES:
        return f":{name} {RESPONSES[name]}"
    if name in MEASUREMENTS:
        offset, amplitude = MEASUREMENTS[name]
        value = offset + amplitude * math.sin(elapsed)
        return f":{name} {value:.6f}"
    return ":ERROR(0x2004): Unknown SCPI-Kommand."


class ScpiHandler(socketserver.StreamRequestHandler):
    def handle(self):
        while True:
            line = self.rfile.readline(MAX_COMMAND_BYTES + 2)
            if not line:
                return
            if not line.endswith(b"\n"):
                if len(line) > MAX_COMMAND_BYTES:
                    self.wfile.write(b':ERROR(0x2004): Command too long.\n')
                return

            command_bytes = line[:-1]
            if command_bytes.endswith(b"\r"):
                command_bytes = command_bytes[:-1]
            if len(command_bytes) > MAX_COMMAND_BYTES:
                self.wfile.write(b':ERROR(0x2004): Command too long.\n')
                return

            try:
                command = command_bytes.decode("ascii").strip().upper()
            except UnicodeDecodeError:
                self.wfile.write(b':ERROR(0x2004): Invalid character.\n')
                continue

            if self.server.echo:
                self.wfile.write(command_bytes + b"\n")
            response = response_for(command, time.monotonic() - self.server.start_time)
            self.wfile.write(response.encode("ascii") + b"\n")


class ScpiServer(socketserver.ThreadingTCPServer):
    allow_reuse_address = True
    daemon_threads = True

    def __init__(self, server_address, handler_class, echo=False):
        self.echo = echo
        self.start_time = time.monotonic()
        super().__init__(server_address, handler_class)


def port_number(value):
    try:
        port = int(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError("port must be an integer") from error
    if not 1 <= port <= 65535:
        raise argparse.ArgumentTypeError("port must be between 1 and 65535")
    return port


def main():
    parser = argparse.ArgumentParser(description="Run a simulated SCPI TCP device.")
    parser.add_argument("--host", default="127.0.0.1", help="listen address (default: 127.0.0.1)")
    parser.add_argument("--port", type=port_number, default=23, help="TCP port (default: 23)")
    parser.add_argument("--echo", action="store_true", help="echo requests before responses")
    args = parser.parse_args()

    try:
        with ScpiServer((args.host, args.port), ScpiHandler, echo=args.echo) as server:
            print(f"SCPI simulator listening on {args.host}:{args.port}", flush=True)
            try:
                server.serve_forever()
            except KeyboardInterrupt:
                print("\nSCPI simulator stopped.")
    except OSError as error:
        parser.exit(1, f"Could not start SCPI simulator: {error}\n")


if __name__ == "__main__":
    main()
