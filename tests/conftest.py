"""Shared fixtures for Music Assistant Native integration tests.

By default, tests run against a local mock server (no external dependencies).
Set MA_TOKEN to run against a live Music Assistant server instead.
"""

import os
import pytest
import asyncio
import aiohttp

from mock_server import start_mock_server, MOCK_TOKEN

# Live server config (optional)
LIVE_URL = os.environ.get("MA_URL", "")
LIVE_TOKEN = os.environ.get("MA_TOKEN", "")


def pytest_addoption(parser):
    parser.addoption("--ma-url", default=LIVE_URL, help="Music Assistant server URL")
    parser.addoption("--ma-token", default=LIVE_TOKEN, help="Music Assistant auth token")
    parser.addoption("--live", action="store_true", default=False,
                     help="Run against live MA server (requires --ma-url and --ma-token)")


@pytest.fixture(scope="session")
def event_loop_policy():
    return asyncio.DefaultEventLoopPolicy()


@pytest.fixture
def use_live(request):
    """Whether to use a live server."""
    explicit = request.config.getoption("--live")
    has_token = bool(request.config.getoption("--ma-token") or LIVE_TOKEN)
    return explicit or has_token


@pytest.fixture
async def mock_server():
    """Start a mock MA server on a random port."""
    runner, port = await start_mock_server(port=0)
    yield f"http://127.0.0.1:{port}", MOCK_TOKEN
    await runner.cleanup()


@pytest.fixture
def ma_url(request, use_live, mock_server):
    if use_live:
        url = request.config.getoption("--ma-url") or LIVE_URL
        if not url:
            pytest.skip("--ma-url required for live tests")
        return url
    return mock_server[0]


@pytest.fixture
def ma_token(request, use_live, mock_server):
    if use_live:
        token = request.config.getoption("--ma-token") or LIVE_TOKEN
        if not token:
            pytest.skip("--ma-token required for live tests")
        return token
    return mock_server[1]


class MaTestClient:
    """Lightweight WebSocket client for testing."""

    def __init__(self, ws):
        self._ws = ws
        self._counter = 0

    async def send_command(self, command: str, args: dict = None, timeout: float = 15.0):
        self._counter += 1
        msg_id = str(self._counter)
        payload = {"message_id": msg_id, "command": command}
        if args:
            payload["args"] = args
        await self._ws.send_json(payload)

        accumulated = []
        while True:
            resp = await asyncio.wait_for(self._ws.receive_json(), timeout=timeout)
            if "event" in resp:
                continue
            if resp.get("message_id") == msg_id:
                if resp.get("error_code"):
                    raise RuntimeError(
                        f"MA error {resp['error_code']}: {resp.get('details', '?')}"
                    )
                result = resp.get("result")
                if resp.get("partial") and isinstance(result, list):
                    accumulated.extend(result)
                    continue
                if accumulated:
                    if isinstance(result, list):
                        accumulated.extend(result)
                    return accumulated
                return result


@pytest.fixture
async def ma_client(ma_url, ma_token):
    """Authenticated Music Assistant WebSocket client (per-test)."""
    ws_url = ma_url.replace("http://", "ws://").replace("https://", "wss://")
    if not ws_url.endswith("/ws"):
        ws_url = ws_url.rstrip("/") + "/ws"

    session = aiohttp.ClientSession()
    ws = await session.ws_connect(ws_url)

    # Receive server info
    info = await ws.receive_json()
    assert "server_version" in info, f"Expected ServerInfoMessage, got: {info}"

    # Authenticate
    client = MaTestClient(ws)
    result = await client.send_command("auth", {"token": ma_token})
    assert isinstance(result, dict) and result.get("authenticated"), f"Auth failed: {result}"

    yield client

    await ws.close()
    await session.close()
