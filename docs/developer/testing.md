# Testing

## Test Suite Overview

The project has a Python-based integration test suite that tests against a live Music Assistant server.

```
tests/
├── conftest.py          # Fixtures: ma_client, ma_url, ma_token
├── pytest.ini           # Pytest configuration
├── requirements.txt     # Python dependencies
├── test_connection.py   # Server connectivity and auth (4 tests)
├── test_players.py      # Player and queue listing (5 tests)
├── test_library.py      # Library browsing and search (14 tests)
└── test_images.py       # Image proxy loading (2 tests)
```

**Total: 29 tests**

## Running Tests Locally

### Prerequisites

```bash
pip install -r tests/requirements.txt
```

### Run All Tests

```bash
cd tests
MA_URL="http://192.168.1.100:8095" \
MA_TOKEN="your-long-lived-token" \
python3 -m pytest -v
```

### Run Specific Test File

```bash
MA_URL="http://192.168.1.100:8095" \
MA_TOKEN="your-token" \
python3 -m pytest test_library.py -v
```

### Run a Single Test

```bash
MA_URL="http://192.168.1.100:8095" \
MA_TOKEN="your-token" \
python3 -m pytest test_library.py::test_search -v
```

### Command-Line Options

```bash
python3 -m pytest --ma-url="http://192.168.1.100:8095" --ma-token="your-token" -v
```

## Test Categories

### Connection Tests (`test_connection.py`)

| Test | Validates |
|------|-----------|
| `test_server_reachable` | HTTP `/info` endpoint responds |
| `test_websocket_connects` | WebSocket handshake and ServerInfoMessage |
| `test_auth_with_token` | Authentication with valid token |
| `test_auth_rejects_bad_token` | Invalid token returns error |

### Player Tests (`test_players.py`)

| Test | Validates |
|------|-----------|
| `test_list_players` | `players/all` returns non-empty list |
| `test_player_has_required_fields` | Each player has `player_id`, `name`, `provider`, etc. |
| `test_get_single_player` | `players/get` with specific ID |
| `test_player_queues_all` | `player_queues/all` returns queues |
| `test_queue_has_required_fields` | Each queue has `queue_id`, `display_name`, etc. |

### Library Tests (`test_library.py`)

| Test | Validates |
|------|-----------|
| `test_library_count[*]` | Count endpoint for all 5 media types |
| `test_library_items[*]` | Listing endpoint with limit/offset for all 5 types |
| `test_artist_has_required_fields` | Artists have `item_id`, `name`, `provider` |
| `test_album_has_artist_and_year` | Albums have `artists` array |
| `test_track_has_duration_and_artists` | Tracks have `duration` and `artists` |
| `test_album_has_images` | At least one album has image metadata |
| `test_search` | Search returns results for "rock" |
| `test_search_empty_query_handled` | Non-matching search doesn't error |
| `test_album_tracks` | `album_tracks` endpoint works |
| `test_artist_albums` | `artist_albums` endpoint works |

### Image Tests (`test_images.py`)

| Test | Validates |
|------|-----------|
| `test_image_proxy_returns_image` | Image proxy returns valid image data with auth |
| `test_image_proxy_works_without_auth` | Image proxy is publicly accessible |

## Test Fixtures

### `ma_client`

An authenticated `MaTestClient` instance (per-test scope). Handles WebSocket connection, ServerInfoMessage, and token authentication. Each test gets a fresh connection.

### `ma_url` / `ma_token`

Read from environment variables `MA_URL` and `MA_TOKEN`, or from pytest command-line options `--ma-url` and `--ma-token`. Tests are **skipped** if `MA_TOKEN` is not set.

## CI/CD Integration

### GitHub Actions

The `.github/workflows/ci.yml` workflow has two jobs:

1. **Build** — always runs, compiles the app in a Fedora container, produces RPM artifact
2. **Integration Tests** — runs only when `MA_URL` is configured as a repository variable

### GitHub Repository Setup

| Type | Name | Value |
|------|------|-------|
| Variable | `MA_URL` | `http://your-server:8095` |
| Secret | `MA_TOKEN` | Your long-lived access token |

!!! note "Network access"
    GitHub Actions runners need network access to your MA server. If your server is on a private network, you'll need a VPN, tunnel, or to run tests only locally.

## Writing New Tests

1. Add tests to the appropriate file, or create a new `test_*.py` file
2. Use the `ma_client` fixture for authenticated API access:

    ```python
    @pytest.mark.asyncio
    async def test_something(ma_client):
        result = await ma_client.send_command("some/command", {"arg": "value"})
        assert result is not None
    ```

3. Use `ma_url` and `ma_token` fixtures for HTTP-level tests
4. Use `pytest.skip()` when test data isn't available (e.g., empty library)
