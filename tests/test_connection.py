"""Test server connection and authentication."""

import pytest
import aiohttp


@pytest.mark.asyncio
async def test_server_reachable(ma_url):
    """Server /info endpoint responds."""
    async with aiohttp.ClientSession() as session:
        async with session.get(f"{ma_url}/info") as resp:
            assert resp.status == 200
            data = await resp.json()
            assert "server_version" in data
            assert data["status"] == "running"


@pytest.mark.asyncio
async def test_websocket_connects(ma_url):
    """WebSocket connects and receives ServerInfoMessage."""
    ws_url = ma_url.replace("http://", "ws://").rstrip("/") + "/ws"
    async with aiohttp.ClientSession() as session:
        async with session.ws_connect(ws_url) as ws:
            info = await ws.receive_json()
            assert "server_id" in info
            assert "server_version" in info


@pytest.mark.asyncio
async def test_auth_with_token(ma_client):
    """Authentication succeeds — client fixture is usable."""
    # If we get here, the ma_client fixture already authenticated
    assert ma_client is not None


@pytest.mark.asyncio
async def test_auth_rejects_bad_token(ma_url):
    """Authentication fails with invalid token."""
    ws_url = ma_url.replace("http://", "ws://").rstrip("/") + "/ws"
    async with aiohttp.ClientSession() as session:
        async with session.ws_connect(ws_url) as ws:
            await ws.receive_json()  # server info
            await ws.send_json({
                "message_id": "1",
                "command": "auth",
                "args": {"token": "invalid-token-value"}
            })
            resp = await ws.receive_json()
            assert resp.get("error_code"), "Expected auth error for bad token"
