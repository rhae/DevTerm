import math
import socket
import sys
import threading
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from scpi_server import MAX_COMMAND_BYTES, ScpiHandler, ScpiServer, response_for


class ScpiServerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.server = ScpiServer(("127.0.0.1", 0), ScpiHandler)
        cls.echo_server = ScpiServer(("127.0.0.1", 0), ScpiHandler, echo=True)
        cls.threads = []
        for server in (cls.server, cls.echo_server):
            thread = threading.Thread(target=server.serve_forever, daemon=True)
            thread.start()
            cls.threads.append(thread)

    @classmethod
    def tearDownClass(cls):
        for server in (cls.server, cls.echo_server):
            server.shutdown()
            server.server_close()
        for thread in cls.threads:
            thread.join()

    def connect(self, server=None):
        return socket.create_connection((server or self.server).server_address, timeout=2)

    def test_commands_and_unknown_command(self):
        with self.connect() as client:
            client.sendall(b"*DEV ?\n*ver?\r\n*SER ?\n*XYZ ?\n")
            with client.makefile("rb") as response:
                self.assertEqual(response.readline(), b":DEV DevTerm SCPI Simulator\n")
                self.assertEqual(response.readline(), b":VER 10.234\n")
                self.assertEqual(response.readline(), b":SER SIM000001\n")
                self.assertEqual(response.readline(), b":ERROR(0x2004): Unknown SCPI-Kommand.\n")

    def test_echo_preserves_request_and_precedes_response(self):
        with self.connect(self.echo_server) as client:
            client.sendall(b" *ver ? \r\n*XYZ ?\n")
            with client.makefile("rb") as response:
                self.assertEqual(response.readline(), b" *ver ? \n")
                self.assertEqual(response.readline(), b":VER 10.234\n")
                self.assertEqual(response.readline(), b"*XYZ ?\n")
                self.assertEqual(response.readline(), b":ERROR(0x2004): Unknown SCPI-Kommand.\n")

    def test_incomplete_or_invalid_queries_are_rejected(self):
        error = ":ERROR(0x2004): Unknown SCPI-Kommand."
        for command in ("VER", "VER?", "*VER", "*VER ? EXTRA", "*VER??"):
            with self.subTest(command=command):
                self.assertEqual(response_for(command, 0), error)

    def test_measurement_sine_values_and_amplitudes(self):
        for command, offset, amplitude in (
            ("MEAS:CURR", 0.5, 0.25),
            ("MEAS:VOLT", 12.0, 2.0),
            ("MEAS:POW", 6.0, 1.5),
        ):
            for elapsed, expected in ((0, offset), (math.pi / 2, offset + amplitude),
                                      (3 * math.pi / 2, offset - amplitude)):
                self.assertEqual(response_for(f"*{command} ?", elapsed),
                                 f":{command} {expected:.6f}")

        with self.connect() as client:
            client.sendall(b"*MEAS:CURR ?\n*MEAS:VOLT ?\n*MEAS:POW ?\n")
            with client.makefile("rb") as response:
                for name, low, high in (("CURR", 0.25, 0.75),
                                         ("VOLT", 10.0, 14.0),
                                         ("POW", 4.5, 7.5)):
                    label, value = response.readline().decode("ascii").split()
                    self.assertEqual(label, f":MEAS:{name}")
                    self.assertLessEqual(low, float(value))
                    self.assertLessEqual(float(value), high)

    def test_fragmented_commands_and_independent_clients(self):
        with self.connect() as first, self.connect() as second:
            first.sendall(b"*MEAS:")
            second.sendall(b"*DEV ?\n")
            with second.makefile("rb") as response:
                self.assertEqual(response.readline(), b":DEV DevTerm SCPI Simulator\n")
            first.sendall(b"VOLT ?\r\n")
            with first.makefile("rb") as response:
                self.assertTrue(response.readline().startswith(b":MEAS:VOLT "))

    def test_invalid_character_does_not_close_connection(self):
        with self.connect() as client:
            client.sendall(b"\xff\n*DEV ?\n")
            with client.makefile("rb") as response:
                self.assertEqual(response.readline(), b':ERROR(0x2004): Invalid character.\n')
                self.assertEqual(response.readline(), b":DEV DevTerm SCPI Simulator\n")

    def test_oversized_command_is_rejected(self):
        with self.connect() as client:
            client.sendall(b"X" * (MAX_COMMAND_BYTES + 1) + b"\n")
            with client.makefile("rb") as response:
                self.assertEqual(response.readline(), b':ERROR(0x2004): Command too long.\n')
                self.assertEqual(response.readline(), b"")

    def test_maximum_command_length_accepts_crlf(self):
        with self.connect() as client:
            client.sendall(b"X" * MAX_COMMAND_BYTES + b"\r\n*DEV ?\n")
            with client.makefile("rb") as response:
                self.assertEqual(response.readline(), b':ERROR(0x2004): Unknown SCPI-Kommand.\n')
                self.assertEqual(response.readline(), b":DEV DevTerm SCPI Simulator\n")


if __name__ == "__main__":
    unittest.main()
