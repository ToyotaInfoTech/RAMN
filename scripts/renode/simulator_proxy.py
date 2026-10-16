#!/usr/bin/env python3
# Copyright (c) 2024 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Dual-transport superserver proxy for containerized Renode RAMN simulator.

Bridges incoming connections on port 29536 to the internal Renode LPUART1
terminal. Automatically detects client protocol:
- RFC 6455 WebSocket: for browser-based clients such as SloppyCAN.
- Raw TCP stream: for python-can, socat, slcand, and automation scripts.

Multiplexes client traffic to and from Renode's single internal terminal,
performs active readiness probing, and cleanly manages process lifecycle.
"""

from __future__ import annotations

import asyncio
import base64
import hashlib
import logging
import os
import signal
import sys
from typing import Optional, Set

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] [simulator-proxy] %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger("simulator_proxy")

LISTEN_HOST = os.environ.get("LISTEN_HOST", "0.0.0.0")
LISTEN_PORT = int(os.environ.get("LISTEN_PORT", "29536"))
INTERNAL_PORT = int(os.environ.get("INTERNAL_PORT", "29600"))
STARTUP_TIMEOUT_SECS = float(os.environ.get("STARTUP_TIMEOUT_SECS", "60.0"))
RESC_FILE = os.environ.get(
    "RESC_FILE", sys.argv[1] if len(sys.argv) > 1 else "/opt/renode/ramn_4ecu.resc"
)
RENODE_BIN = os.environ.get("RENODE_BIN", "/opt/renode/renode")


def make_ws_frame(payload: bytes, opcode: int = 1) -> bytes:
    """Build an unmasked RFC 6455 WebSocket frame from server to client."""
    length = len(payload)
    fin_and_opcode = 0x80 | (opcode & 0x0F)
    if length < 126:
        header = bytes([fin_and_opcode, length])
    elif length < 65536:
        header = bytes([fin_and_opcode, 126]) + length.to_bytes(2, "big")
    else:
        header = bytes([fin_and_opcode, 127]) + length.to_bytes(8, "big")
    return header + payload


class SuperserverProxy:
    def __init__(
        self,
        resc_file: str = RESC_FILE,
        renode_bin: str = RENODE_BIN,
        internal_port: int = INTERNAL_PORT,
        listen_host: str = LISTEN_HOST,
        listen_port: int = LISTEN_PORT,
    ) -> None:
        self.resc_file = resc_file
        self.renode_bin = renode_bin
        self.internal_port = internal_port
        self.listen_host = listen_host
        self.listen_port = listen_port

        self.renode_proc: Optional[asyncio.subprocess.Process] = None
        self.sim_reader: Optional[asyncio.StreamReader] = None
        self.sim_writer: Optional[asyncio.StreamWriter] = None
        self.server: Optional[asyncio.Server] = None

        self.raw_clients: Set[asyncio.StreamWriter] = set()
        self.ws_clients: Set[asyncio.StreamWriter] = set()
        self._shutting_down = False

    async def start_renode(self) -> None:
        """Starts child Renode process, probes readiness, and maintains backend connection."""
        cmd = [
            self.renode_bin,
            "--port",
            "-1",
            "--plain",
            "--disable-gui",
            "-e",
            f"include @{self.resc_file}",
        ]
        logger.info(
            f"Spawning Renode process with script: {self.resc_file} "
            f"(internal port: {self.internal_port})..."
        )
        self.renode_proc = await asyncio.create_subprocess_exec(
            *cmd,
            stdout=asyncio.subprocess.DEVNULL,
            stderr=asyncio.subprocess.DEVNULL,
        )

        logger.info(
            f"Probing Renode SLCAN readiness on 127.0.0.1:{self.internal_port}..."
        )
        deadline = asyncio.get_running_loop().time() + STARTUP_TIMEOUT_SECS
        connected = False
        while asyncio.get_running_loop().time() < deadline:
            if self.renode_proc.returncode is not None:
                logger.error(
                    f"Renode exited unexpectedly with code {self.renode_proc.returncode}"
                )
                raise RuntimeError(
                    f"Renode process exited with code {self.renode_proc.returncode}"
                )
            writer = None
            try:
                reader, writer = await asyncio.wait_for(
                    asyncio.open_connection("127.0.0.1", self.internal_port),
                    timeout=1.0,
                )
                writer.write(b"O\r")
                await writer.drain()
                data = await asyncio.wait_for(reader.read(16), timeout=1.0)
                if b"\r" in data or b"\n" in data:
                    self.sim_reader = reader
                    self.sim_writer = writer
                    connected = True
                    logger.info("Renode LPUART1 SLCAN terminal verified ready.")
                    break
            except (ConnectionRefusedError, OSError, asyncio.TimeoutError):
                pass
            finally:
                if not connected and writer is not None:
                    try:
                        writer.close()
                        await writer.wait_closed()
                    except Exception:
                        pass
            await asyncio.sleep(0.5)

        if not connected or self.sim_reader is None or self.sim_writer is None:
            raise TimeoutError(
                f"Timed out after {STARTUP_TIMEOUT_SECS}s waiting for Renode on port {self.internal_port}"
            )

        asyncio.create_task(self._sim_reader_loop())

    async def _sim_reader_loop(self) -> None:
        """Continuously reads from Renode and broadcasts to raw TCP and WebSocket clients."""
        assert self.sim_reader is not None
        ws_buffer = bytearray()
        try:
            while not self._shutting_down:
                data = await self.sim_reader.read(4096)
                if not data:
                    break

                # Forward raw bytes to all raw TCP clients
                dead_raw = set()
                for writer in list(self.raw_clients):
                    try:
                        writer.write(data)
                    except Exception:
                        dead_raw.add(writer)
                for w in dead_raw:
                    self.raw_clients.discard(w)

                # Forward line-framed text to all WebSocket clients
                if self.ws_clients:
                    ws_buffer.extend(data)
                    while True:
                        r_idx = ws_buffer.find(b"\r")
                        n_idx = ws_buffer.find(b"\n")
                        if r_idx == -1 and n_idx == -1:
                            break
                        end_idx = (
                            min(r_idx, n_idx)
                            if (r_idx != -1 and n_idx != -1)
                            else (r_idx if r_idx != -1 else n_idx)
                        )
                        if (
                            end_idx == r_idx
                            and end_idx + 1 < len(ws_buffer)
                            and ws_buffer[end_idx + 1] == 10
                        ):
                            line_end = end_idx + 2
                        else:
                            line_end = end_idx + 1
                        line = bytes(ws_buffer[:line_end])
                        del ws_buffer[:line_end]
                        if line:
                            frame = make_ws_frame(line, opcode=1)
                            dead_ws = set()
                            for writer in list(self.ws_clients):
                                try:
                                    writer.write(frame)
                                except Exception:
                                    dead_ws.add(writer)
                            for w in dead_ws:
                                self.ws_clients.discard(w)
                else:
                    ws_buffer.clear()
        except (asyncio.CancelledError, OSError) as e:
            logger.debug(f"Sim reader loop ended: {e}")

    async def handle_client(
        self, client_reader: asyncio.StreamReader, client_writer: asyncio.StreamWriter
    ) -> None:
        peer = client_writer.get_extra_info("peername")
        logger.info(f"Incoming connection from {peer}")

        initial_bytes = b""
        try:
            chunk = await asyncio.wait_for(client_reader.read(4096), timeout=0.2)
            if isinstance(chunk, (bytes, bytearray)):
                initial_bytes = bytes(chunk)
        except (asyncio.TimeoutError, Exception):
            initial_bytes = b""

        is_websocket = False
        ws_key = ""
        if initial_bytes.startswith(b"GET "):
            http_buf = initial_bytes
            while b"\r\n\r\n" not in http_buf and len(http_buf) < 8192:
                try:
                    more = await asyncio.wait_for(
                        client_reader.read(1024), timeout=0.5
                    )
                    if not more or not isinstance(more, (bytes, bytearray)):
                        break
                    http_buf += bytes(more)
                except Exception:
                    break

            try:
                headers_text = http_buf.decode("utf-8", errors="replace")
                lines = headers_text.split("\r\n")
                headers = {}
                for line in lines[1:]:
                    if ":" in line:
                        k, v = line.split(":", 1)
                        headers[k.strip().lower()] = v.strip()
                if (
                    "upgrade" in headers.get("connection", "").lower()
                    and headers.get("upgrade", "").lower() == "websocket"
                ):
                    is_websocket = True
                    ws_key = headers.get("sec-websocket-key", "")
            except Exception:
                is_websocket = False

        if is_websocket and ws_key:
            accept_key = base64.b64encode(
                hashlib.sha1(
                    (ws_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode()
                ).digest()
            ).decode()
            resp = (
                "HTTP/1.1 101 Switching Protocols\r\n"
                "Upgrade: websocket\r\n"
                "Connection: Upgrade\r\n"
                f"Sec-WebSocket-Accept: {accept_key}\r\n\r\n"
            )
            client_writer.write(resp.encode("ascii"))
            await client_writer.drain()
            logger.info(f"Upgraded {peer} to WebSocket (RFC 6455)")
            self.ws_clients.add(client_writer)

            try:
                while True:
                    header = await client_reader.readexactly(2)
                    b1, b2 = header[0], header[1]
                    opcode = b1 & 0x0F
                    is_masked = (b2 & 0x80) != 0
                    length = b2 & 0x7F

                    if length == 126:
                        ext_len = await client_reader.readexactly(2)
                        length = int.from_bytes(ext_len, "big")
                    elif length == 127:
                        ext_len = await client_reader.readexactly(8)
                        length = int.from_bytes(ext_len, "big")

                    mask_key = b""
                    if is_masked:
                        mask_key = await client_reader.readexactly(4)

                    payload = (
                        await client_reader.readexactly(length)
                        if length > 0
                        else b""
                    )
                    if is_masked and mask_key:
                        unmasked = bytes(
                            b ^ mask_key[i % 4] for i, b in enumerate(payload)
                        )
                    else:
                        unmasked = payload

                    if opcode == 0x8:  # Close
                        try:
                            client_writer.write(make_ws_frame(b"", opcode=0x8))
                            await client_writer.drain()
                        except Exception:
                            pass
                        break
                    elif opcode == 0x9:  # Ping
                        client_writer.write(make_ws_frame(unmasked, opcode=0xA))
                        await client_writer.drain()
                        continue
                    elif opcode in (0x1, 0x2):
                        if self.sim_writer:
                            self.sim_writer.write(unmasked)
                            await self.sim_writer.drain()
            except (
                asyncio.CancelledError,
                ConnectionResetError,
                BrokenPipeError,
                asyncio.IncompleteReadError,
                OSError,
            ):
                pass
            finally:
                self.ws_clients.discard(client_writer)
                client_writer.close()
                try:
                    await client_writer.wait_closed()
                except Exception:
                    pass
                logger.info(f"WebSocket client {peer} disconnected.")
        else:
            logger.info(f"Bridging {peer} as raw TCP stream")
            self.raw_clients.add(client_writer)
            if initial_bytes and self.sim_writer:
                self.sim_writer.write(initial_bytes)
                await self.sim_writer.drain()

            try:
                while True:
                    data = await client_reader.read(4096)
                    if not data:
                        break
                    if self.sim_writer:
                        self.sim_writer.write(data)
                        await self.sim_writer.drain()
            except (
                asyncio.CancelledError,
                ConnectionResetError,
                BrokenPipeError,
                OSError,
            ):
                pass
            finally:
                self.raw_clients.discard(client_writer)
                client_writer.close()
                try:
                    await client_writer.wait_closed()
                except Exception:
                    pass
                logger.info(f"Raw TCP client {peer} disconnected.")

    async def run(self) -> None:
        """Starts Renode and serves the dual-transport proxy."""
        await self.start_renode()

        self.server = await asyncio.start_server(
            self.handle_client, self.listen_host, self.listen_port
        )
        logger.info(
            f"Renode superserver listening on {self.listen_host}:{self.listen_port} "
            f"(bridging to internal port {self.internal_port})"
        )
        async with self.server:
            await self.server.serve_forever()

    async def shutdown(self) -> None:
        """Gracefully closes server and stops Renode process."""
        if self._shutting_down:
            return
        self._shutting_down = True
        logger.info("Shutting down superserver proxy...")

        if self.server:
            self.server.close()
            await self.server.wait_closed()

        for w in list(self.raw_clients) + list(self.ws_clients):
            try:
                w.close()
            except Exception:
                pass

        if self.sim_writer:
            try:
                self.sim_writer.close()
                await self.sim_writer.wait_closed()
            except Exception:
                pass

        if self.renode_proc and self.renode_proc.returncode is None:
            logger.info(
                f"Terminating Renode process (pid {self.renode_proc.pid})..."
            )
            try:
                self.renode_proc.terminate()
                await asyncio.wait_for(self.renode_proc.wait(), timeout=3.0)
            except (asyncio.TimeoutError, ProcessLookupError):
                try:
                    self.renode_proc.kill()
                    await self.renode_proc.wait()
                except ProcessLookupError:
                    pass


async def main() -> None:
    proxy = SuperserverProxy()

    loop = asyncio.get_running_loop()
    stop_event = asyncio.Event()

    def handle_signal() -> None:
        logger.info("Received termination signal.")
        stop_event.set()

    for sig in (signal.SIGINT, signal.SIGTERM):
        try:
            loop.add_signal_handler(sig, handle_signal)
        except NotImplementedError:
            pass

    server_task = asyncio.create_task(proxy.run())
    stop_task = asyncio.create_task(stop_event.wait())

    done, pending = await asyncio.wait(
        [server_task, stop_task],
        return_when=asyncio.FIRST_COMPLETED,
    )

    for t in pending:
        t.cancel()

    await proxy.shutdown()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except (KeyboardInterrupt, asyncio.CancelledError):
        pass
