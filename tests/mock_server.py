"""Mock Music Assistant WebSocket server for CI testing."""

import asyncio
import json
from aiohttp import web

# Sample data that mirrors real MA server responses
MOCK_SERVER_INFO = {
    "server_id": "test-server-id-1234",
    "server_version": "2.8.0",
    "schema_version": 29,
    "min_supported_schema_version": 28,
    "base_url": "http://localhost:{port}",
    "homeassistant_addon": False,
    "onboard_done": True,
    "name": "Test Music Assistant",
    "status": "running",
}

MOCK_TOKEN = "test-valid-token-1234"

MOCK_USER = {
    "user_id": "test-user-id",
    "username": "testuser",
    "role": "admin",
    "enabled": True,
    "display_name": "Test User",
    "avatar_url": "",
    "preferences": {},
    "provider_filter": [],
    "player_filter": [],
}

MOCK_PLAYERS = [
    {
        "player_id": "test_player_1",
        "provider": "test_provider",
        "type": "player",
        "name": "Living Room Speaker",
        "available": True,
        "powered": True,
        "playback_state": "idle",
        "elapsed_time": 0,
        "volume_level": 50,
        "volume_muted": False,
        "group_members": [],
        "supported_features": ["volume_set", "pause", "seek"],
        "icon": "mdi-speaker",
        "enabled": True,
    },
    {
        "player_id": "test_player_2",
        "provider": "test_provider",
        "type": "player",
        "name": "Kitchen Speaker",
        "available": True,
        "powered": False,
        "playback_state": "idle",
        "elapsed_time": 0,
        "volume_level": 30,
        "volume_muted": False,
        "group_members": [],
        "supported_features": ["volume_set", "pause"],
        "icon": "mdi-speaker",
        "enabled": True,
    },
]

MOCK_QUEUES = [
    {
        "queue_id": "test_player_1",
        "active": True,
        "display_name": "Living Room Speaker",
        "available": True,
        "items": 3,
        "shuffle_enabled": False,
        "repeat_mode": "off",
        "dont_stop_the_music_enabled": False,
        "current_index": 0,
        "elapsed_time": 42.5,
        "state": "idle",
        "current_item": {
            "queue_item_id": "qi_1",
            "name": "Bohemian Rhapsody",
            "duration": 354,
        },
        "next_item": None,
    },
    {
        "queue_id": "test_player_2",
        "active": False,
        "display_name": "Kitchen Speaker",
        "available": True,
        "items": 0,
        "shuffle_enabled": False,
        "repeat_mode": "off",
        "dont_stop_the_music_enabled": False,
        "current_index": None,
        "elapsed_time": 0,
        "state": "idle",
        "current_item": None,
        "next_item": None,
    },
]

MOCK_ARTISTS = [
    {"item_id": "1", "provider": "library", "name": "Queen", "media_type": "artist",
     "uri": "library://artist/1", "favorite": True, "metadata": {
         "images": [{"type": "thumb", "path": "https://example.com/queen.jpg", "provider": "test"}]}},
    {"item_id": "2", "provider": "library", "name": "Led Zeppelin", "media_type": "artist",
     "uri": "library://artist/2", "favorite": False, "metadata": {}},
    {"item_id": "3", "provider": "library", "name": "Pink Floyd", "media_type": "artist",
     "uri": "library://artist/3", "favorite": True, "metadata": {}},
]

MOCK_ALBUMS = [
    {"item_id": "1", "provider": "library", "name": "A Night at the Opera", "media_type": "album",
     "uri": "library://album/1", "year": 1975, "favorite": False,
     "artists": [{"item_id": "1", "name": "Queen", "provider": "library"}],
     "provider_mappings": [{"item_id": "1", "provider_domain": "library", "provider_instance": "library"}],
     "metadata": {"images": [{"type": "thumb", "path": "https://example.com/anato.jpg", "provider": "test"}]}},
    {"item_id": "2", "provider": "library", "name": "Led Zeppelin IV", "media_type": "album",
     "uri": "library://album/2", "year": 1971, "favorite": True,
     "artists": [{"item_id": "2", "name": "Led Zeppelin", "provider": "library"}],
     "provider_mappings": [{"item_id": "2", "provider_domain": "library", "provider_instance": "library"}],
     "metadata": {"images": [{"type": "thumb", "path": "https://example.com/lziv.jpg", "provider": "test"}]}},
]

MOCK_TRACKS = [
    {"item_id": "1", "provider": "library", "name": "Bohemian Rhapsody", "media_type": "track",
     "uri": "library://track/1", "duration": 354, "track_number": 11, "favorite": True,
     "artists": [{"item_id": "1", "name": "Queen", "provider": "library"}],
     "album": {"item_id": "1", "name": "A Night at the Opera", "provider": "library"},
     "metadata": {}},
    {"item_id": "2", "provider": "library", "name": "Stairway to Heaven", "media_type": "track",
     "uri": "library://track/2", "duration": 482, "track_number": 4, "favorite": False,
     "artists": [{"item_id": "2", "name": "Led Zeppelin", "provider": "library"}],
     "album": {"item_id": "2", "name": "Led Zeppelin IV", "provider": "library"},
     "metadata": {}},
]

MOCK_PLAYLISTS = [
    {"item_id": "1", "provider": "library", "name": "Classic Rock Hits", "media_type": "playlist",
     "uri": "library://playlist/1", "owner": "testuser", "is_editable": True, "metadata": {}},
]

MOCK_RADIOS = [
    {"item_id": "1", "provider": "tunein", "name": "BBC Radio 6 Music", "media_type": "radio",
     "uri": "tunein://radio/1", "metadata": {}},
]

LIBRARY_DATA = {
    "artists": MOCK_ARTISTS,
    "albums": MOCK_ALBUMS,
    "tracks": MOCK_TRACKS,
    "playlists": MOCK_PLAYLISTS,
    "radios": MOCK_RADIOS,
}


def handle_command(command: str, args: dict) -> dict | list | int | None:
    """Route a command to mock data."""

    # Auth
    if command == "auth":
        token = args.get("token", "")
        if token == MOCK_TOKEN:
            return {"authenticated": True, "user": MOCK_USER}
        raise ValueError("Invalid or expired token")

    if command == "auth/login":
        username = args.get("username", "")
        password = args.get("password", "")
        if username == "testuser" and password == "testpass":
            return {"success": True, "access_token": MOCK_TOKEN, "user": MOCK_USER}
        return {"success": False, "error": "Invalid username or password"}

    # Players
    if command == "players/all":
        return MOCK_PLAYERS
    if command == "players/get":
        pid = args.get("player_id", "")
        for p in MOCK_PLAYERS:
            if p["player_id"] == pid:
                return p
        raise ValueError(f"Player {pid} not found")

    # Queues
    if command == "player_queues/all":
        return MOCK_QUEUES
    if command == "player_queues/get":
        qid = args.get("queue_id", "")
        for q in MOCK_QUEUES:
            if q["queue_id"] == qid:
                return q
        raise ValueError(f"Queue {qid} not found")

    # Library counts
    for media_type in LIBRARY_DATA:
        if command == f"music/{media_type}/count":
            return len(LIBRARY_DATA[media_type])
        if command == f"music/{media_type}/library_items":
            limit = args.get("limit", 500)
            offset = args.get("offset", 0)
            return LIBRARY_DATA[media_type][offset:offset + limit]

    # Album tracks
    if command == "music/albums/album_tracks":
        return [t for t in MOCK_TRACKS if t.get("album", {}).get("item_id") == args.get("item_id")]

    # Artist albums
    if command == "music/artists/artist_albums":
        aid = args.get("item_id")
        return [a for a in MOCK_ALBUMS if any(ar.get("item_id") == aid for ar in a.get("artists", []))]

    # Search
    if command == "music/search":
        query = args.get("search_query", "").lower()
        result = {}
        for media_type, items in LIBRARY_DATA.items():
            result[media_type] = [i for i in items if query in i.get("name", "").lower()]
        return result

    # Playback commands (no-ops in mock)
    if command.startswith("players/cmd/") or command.startswith("player_queues/"):
        return None

    if command == "ping":
        return "pong"

    raise ValueError(f"Unknown command: {command}")


async def websocket_handler(request):
    """Handle a WebSocket connection mimicking the MA protocol."""
    ws = web.WebSocketResponse()
    await ws.prepare(request)

    port = request.app["port"]
    server_info = dict(MOCK_SERVER_INFO)
    server_info["base_url"] = f"http://localhost:{port}"
    await ws.send_json(server_info)

    authenticated = False

    async for msg in ws:
        if msg.type == web.WSMsgType.TEXT:
            try:
                data = json.loads(msg.data)
            except json.JSONDecodeError:
                continue

            message_id = data.get("message_id", "")
            command = data.get("command", "")
            args = data.get("args", {})

            # Auth check (auth and auth/login don't require prior auth)
            if command not in ("auth", "auth/login", "ping") and not authenticated:
                await ws.send_json({
                    "message_id": message_id,
                    "error_code": 401,
                    "details": "Authentication required. Please send auth command first.",
                })
                continue

            try:
                result = handle_command(command, args)
                if command == "auth" and isinstance(result, dict) and result.get("authenticated"):
                    authenticated = True
                await ws.send_json({
                    "message_id": message_id,
                    "result": result,
                    "partial": False,
                })
            except ValueError as e:
                await ws.send_json({
                    "message_id": message_id,
                    "error_code": 500,
                    "details": str(e),
                })
        elif msg.type == web.WSMsgType.ERROR:
            break

    return ws


async def info_handler(request):
    """GET /info endpoint."""
    port = request.app["port"]
    info = dict(MOCK_SERVER_INFO)
    info["base_url"] = f"http://localhost:{port}"
    return web.json_response(info)


# Serve a 1x1 transparent PNG for image proxy
TINY_PNG = (
    b"\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x01"
    b"\x00\x00\x00\x01\x08\x06\x00\x00\x00\x1f\x15\xc4\x89"
    b"\x00\x00\x00\nIDATx\x9cc\x00\x01\x00\x00\x05\x00\x01"
    b"\r\n\xb4\x00\x00\x00\x00IEND\xaeB`\x82"
)


async def imageproxy_handler(request):
    """GET /imageproxy — returns a tiny test image."""
    return web.Response(body=TINY_PNG, content_type="image/png")


def create_app(port: int = 0) -> web.Application:
    app = web.Application()
    app["port"] = port
    app.router.add_get("/ws", websocket_handler)
    app.router.add_get("/info", info_handler)
    app.router.add_get("/imageproxy", imageproxy_handler)
    return app


async def start_mock_server(port: int = 0) -> tuple[web.AppRunner, int]:
    """Start mock server on a random port. Returns (runner, actual_port)."""
    app = create_app(port)
    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, "127.0.0.1", port)
    await site.start()
    actual_port = site._server.sockets[0].getsockname()[1]
    app["port"] = actual_port
    return runner, actual_port
