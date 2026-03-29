# Music Assistant - Comprehensive API & Architecture Research

## 1. What Is Music Assistant

Music Assistant (MA) is a free, open-source media library manager in the Home Assistant ecosystem. It aggregates music from 40+ streaming services and local sources into a unified library and streams to a wide range of connected speakers.

### Architecture

Three-layer architecture:

1. **Server** (`music-assistant/server`) - The core, runs on always-on hardware (RPi, NAS, NUC). Written in Python (asyncio). Default branch: `dev`.
2. **Providers** - Adapters for music sources (Spotify, Tidal, Qobuz, Plex, Jellyfin, local files, etc.) and player targets (Sonos, AirPlay, Google Cast, Alexa, DLNA, Snapcast, etc.)
3. **Frontend** (`music-assistant/frontend`) - Vue 3 Progressive Web App

### Key Repositories

| Repository | Purpose |
|---|---|
| `music-assistant/server` | Core server (Python, branch: `dev`) |
| `music-assistant/models` | Shared data models (`music_assistant_models` PyPI package) |
| `music-assistant/client` | Python client library (`music_assistant_client` PyPI package) |
| `music-assistant/frontend` | Vue 3 web frontend |
| `music-assistant/mobile-app` | Kotlin Multiplatform mobile app (Android/iOS) |

### Key Features

- Unified library across all music sources with cross-provider track matching
- Gapless playback, crossfade, and volume normalization for all players
- Player grouping/syncing across different ecosystems
- DSP: parametric EQ, tone control, per-player configuration
- Announcements during playback
- Radio mode ("Don't Stop The Music")
- Audiobook and podcast support with resume positions

---

## 2. Connection Architecture

### Endpoints

The server exposes an aiohttp web server with these endpoints:

| Path | Method | Purpose |
|---|---|---|
| `/ws` | GET (WebSocket) | **Primary API** - WebSocket for real-time bidirectional communication |
| `/api` | POST | JSON-RPC API (same commands as WebSocket) |
| `/info` | GET | Server information (no auth required) |
| `/auth/login` | POST | Login with credentials |
| `/auth/logout` | POST | Logout |
| `/auth/me` | GET | Current user info |
| `/auth/providers` | GET | Available auth providers |
| `/auth/authorize` | GET | OAuth authorization initiation |
| `/auth/callback` | GET | OAuth callback |
| `/setup` | GET/POST | First-time admin setup |
| `/imageproxy` | GET | Proxied/resized images |
| `/preview` | GET | Audio preview streams |
| `/api-docs` | GET | API documentation |
| `/api-docs/openapi.json` | GET | OpenAPI spec |
| `/api-docs/swagger` | GET | Swagger UI |

### WebSocket Protocol

The primary API is WebSocket-based at `ws://<host>:<port>/ws`. All communication uses JSON messages.

#### Message Types

**CommandMessage** (Client -> Server):
```json
{
  "message_id": "uuid-string",
  "command": "command/path",
  "args": {
    "param1": "value1",
    "param2": "value2"
  }
}
```

**SuccessResultMessage** (Server -> Client):
```json
{
  "message_id": "uuid-matching-command",
  "result": <any>,
  "partial": false
}
```
Note: For large results (e.g., library listings), the server streams results in 500-item batches with `"partial": true`, followed by a final message with `"partial": false`.

**ErrorResultMessage** (Server -> Client):
```json
{
  "message_id": "uuid-matching-command",
  "error_code": 500,
  "details": "Error description"
}
```

**EventMessage** (Server -> Client, after subscription):
```json
{
  "event": "player_updated",
  "object_id": "player_id_here",
  "data": { ... }
}
```

**ServerInfoMessage** (Server -> Client, on connect):
```json
{
  "server_id": "uuid",
  "server_version": "2.x.x",
  "schema_version": 29,
  "min_supported_schema_version": 25,
  "base_url": "http://host:port",
  "homeassistant_addon": false,
  "onboard_done": true,
  "name": "My Music Assistant",
  "status": "running"
}
```

---

## 3. Authentication & Connection Flow

### Connection Sequence

1. Client opens WebSocket to `ws://<host>:<port>/ws`
2. Server sends `ServerInfoMessage` with version info
3. Client validates schema version compatibility
4. Client sends `auth` command with token:
   ```json
   {
     "message_id": "1",
     "command": "auth",
     "args": {"token": "your-jwt-token"}
   }
   ```
5. Server validates token, responds with user details
6. Server automatically subscribes client to event stream
7. Client fetches initial state (providers, players, queues)

### Token Types

- **Short-lived tokens**: 30-day expiration with sliding window (auto-renewed on use)
- **Long-lived tokens**: 10-year expiration, no auto-renewal
- **Join codes**: 6-character alphanumeric codes (8-hour default expiry) for easy device pairing

### Getting a Token

**Option A - Login with credentials:**
```json
{
  "message_id": "1",
  "command": "auth/login",
  "args": {
    "username": "admin",
    "password": "secret",
    "provider_id": "builtin",
    "device_name": "My Desktop Client"
  }
}
```

**Option B - Exchange a join code:**
```json
{
  "message_id": "1",
  "command": "auth/join_code/exchange",
  "args": {"code": "ABC123"}
}
```

**Option C - Create a long-lived token (when already authenticated):**
```json
{
  "message_id": "2",
  "command": "auth/token/create",
  "args": {"name": "Desktop Client Token"}
}
```

### User Roles

| Role | Access |
|---|---|
| `admin` | Full access, can manage users/providers/config |
| `user` | Standard access, playback and library management |
| `guest` | Limited access |

---

## 4. Complete API Command Reference

### Authentication Commands

| Command | Auth Required | Role | Parameters |
|---|---|---|---|
| `auth/login` | No | - | `username, password, provider_id, device_name, **extra_credentials` |
| `auth/providers` | No | - | (none) |
| `auth/authorization_url` | No | - | `provider_id, return_url` |
| `auth/join_code/exchange` | No | - | `code` |
| `auth/me` | Yes | - | (none) |
| `auth/logout` | Yes | - | (none) |
| `auth/users` | Yes | admin | (none) |
| `auth/user` | Yes | admin | `user_id` |
| `auth/user/create` | Yes | admin | `username, password, role, display_name, avatar_url, player_filter, provider_filter` |
| `auth/user/delete` | Yes | admin | `user_id` |
| `auth/user/enable` | Yes | admin | `user_id` |
| `auth/user/disable` | Yes | admin | `user_id` |
| `auth/user/update` | Yes | - | `user_id, username, display_name, avatar_url, password, role, preferences, player_filter, provider_filter` |
| `auth/user/providers` | Yes | - | (none) |
| `auth/user/unlink_provider` | Yes | admin | `user_id, provider_type` |
| `auth/tokens` | Yes | - | `user_id` (optional) |
| `auth/token/create` | Yes | - | `name, user_id` (optional) |
| `auth/token/revoke` | Yes | - | `token_id` |
| `auth/join_codes` | Yes | admin | `user_id` (optional) |
| `auth/join_code/revoke` | Yes | admin | `code_id` |

### Music Library - Generic Commands

| Command | Parameters | Returns |
|---|---|---|
| `music/search` | `search_query, media_types?, limit=25, library_only=False` | `SearchResults` |
| `music/browse` | `path?` | `list[MediaItem \| BrowseFolder]` |
| `music/item` | `media_type, item_id, provider_instance_id_or_domain, allow_update_metadata=True` | `MediaItem` |
| `music/item_by_uri` | `uri, allow_update_metadata=False` | `MediaItem \| BrowseFolder` |
| `music/get_library_item` | `media_type, item_id, provider_instance_id_or_domain` | `MediaItem \| None` |
| `music/sync` | `media_types?, providers?` | `list[BackgroundTask]` |
| `music/recently_played_items` | `limit=10, media_types?, userid?, queue_id?, fully_played_only=True, user_initiated_only=False` | `list[ItemMapping]` |
| `music/recently_added_tracks` | `limit=10` | `list[Track]` |
| `music/in_progress_items` | `limit=10, all_users=False` | `list[ItemMapping]` |
| `music/recommendations` | (none) | `list[RecommendationFolder]` |
| `music/favorites/add_item` | `item` (URI string or media item) | `None` |
| `music/favorites/remove_item` | `media_type, library_item_id` | `None` |
| `music/library/add_item` | `item, overwrite_existing=False` | `MediaItem` |
| `music/library/remove_item` | `media_type, library_item_id, recursive=True` | `None` |
| `music/refresh_item` | `media_item` | `MediaItem \| None` |
| `music/mark_played` | `media_item, fully_played=True, seconds_played?, is_playing=False, userid?, queue_id?, user_initiated=True` | `None` |
| `music/mark_unplayed` | `media_item, userid?` | `None` |
| `music/track_by_name` | `track_name, artist_name?, album_name?, track_version?` | `Track \| None` |
| `music/add_provider_mapping` | `media_type, db_id, mapping` | `None` |
| `music/remove_provider_mapping` | `media_type, db_id, mapping` | `None` |
| `music/match_providers` | `media_type, db_id` | `None` |

### Music Library - Per-Type Commands

For each media type (`artists`, `albums`, `tracks`, `playlists`, `radios`, `audiobooks`, `podcasts`):

| Command Pattern | Parameters | Returns |
|---|---|---|
| `music/{type}/library_items` | `favorite?, search?, limit?, offset?, order_by?, provider?, genre?` | `list[ItemType]` |
| `music/{type}/count` | `favorite_only=False` | `int` |
| `music/{type}/get` | `item_id, provider_instance_id_or_domain, allow_update_metadata=True` | `ItemType` |
| `music/{type}/update` | `item_id, update, overwrite=False` | `ItemType` (admin) |
| `music/{type}/remove` | `item_id, recursive=True` | `None` (admin) |

#### Type-Specific Additional Commands

**Artists:**
| Command | Parameters | Returns |
|---|---|---|
| `music/artists/artist_tracks` | `item_id, provider_instance_id_or_domain, in_library_only?` | `list[Track]` |
| `music/artists/artist_albums` | `item_id, provider_instance_id_or_domain, in_library_only?` | `list[Album]` |

**Albums:**
| Command | Parameters | Returns |
|---|---|---|
| `music/albums/album_tracks` | `item_id, provider_instance_id_or_domain, in_library_only?` | `list[Track]` |
| `music/albums/album_versions` | `item_id, provider_instance_id_or_domain` | `list[Album]` |

**Tracks:**
| Command | Parameters | Returns |
|---|---|---|
| `music/tracks/track_versions` | `item_id, provider_instance_id_or_domain` | `list[Track]` |
| `music/tracks/track_albums` | `item_id, provider_instance_id_or_domain, in_library_only?` | `list[Album]` |
| `music/tracks/similar_tracks` | `item_id, provider_instance_id_or_domain, limit?, allow_lookup?` | `list[Track]` |
| `music/tracks/preview` | `provider_instance_id_or_domain, item_id` | `str` (URL) |

**Playlists:**
| Command | Parameters | Returns |
|---|---|---|
| `music/playlists/playlist_tracks` | `item_id, provider_instance_id_or_domain, page?` | `list[Track]` |
| `music/playlists/add_playlist_tracks` | `db_playlist_id, uris` | `None` |
| `music/playlists/remove_playlist_tracks` | `db_playlist_id, positions_to_remove` | `None` |
| `music/playlists/create_playlist` | `name, provider_instance_or_domain` | `Playlist` |

**Podcasts:**
| Command | Parameters | Returns |
|---|---|---|
| `music/podcasts/podcast_episodes` | `item_id, provider_instance_id_or_domain` | `list[PodcastEpisode]` |
| `music/podcasts/podcast_episode` | `item_id, provider_instance_id_or_domain` | `list[PodcastEpisode]` |

**Radios:**
| Command | Parameters | Returns |
|---|---|---|
| `music/radios/radio_versions` | `item_id, provider_instance_id_or_domain` | `list[Radio]` |

### Player Commands

| Command | Parameters | Returns |
|---|---|---|
| `players/all` | `return_unavailable?, return_disabled?, provider_filter?, return_protocol_players?` | `list[Player]` |
| `players/get` | `player_id, raise_unavailable?` | `Player \| None` |
| `players/get_by_name` | `name` | `Player \| None` |
| `players/cmd/stop` | `player_id` | `None` |
| `players/cmd/play` | `player_id` | `None` |
| `players/cmd/pause` | `player_id` | `None` |
| `players/cmd/play_pause` | `player_id` | `None` |
| `players/cmd/resume` | `player_id, source?, media?` | `None` |
| `players/cmd/seek` | `player_id, position` | `None` |
| `players/cmd/next` | `player_id` | `None` |
| `players/cmd/previous` | `player_id` | `None` |
| `players/cmd/volume_set` | `player_id, volume_level` | `None` |
| `players/cmd/volume_up` | `player_id` | `None` |
| `players/cmd/volume_down` | `player_id` | `None` |
| `players/cmd/volume_mute` | `player_id, muted` | `None` |
| `players/cmd/group_volume` | `player_id, volume_level` | `None` |
| `players/cmd/group_volume_up` | `player_id` | `None` |
| `players/cmd/group_volume_down` | `player_id` | `None` |
| `players/cmd/group_volume_mute` | `player_id, muted` | `None` |
| `players/cmd/power` | `player_id, powered` | `None` |
| `players/cmd/select_source` | `player_id, source` | `None` |
| `players/cmd/select_sound_mode` | `player_id, sound_mode` | `None` |
| `players/cmd/set_option` | `player_id, option_key, option_value` | `None` |
| `players/cmd/play_announcement` | `player_id, url, pre_announce?, volume_level?, pre_announce_url?` | `None` |
| `players/cmd/group` | `player_id, target_player` | `None` |
| `players/cmd/group_many` | `target_player, child_player_ids` | `None` |
| `players/cmd/ungroup` | `player_id` | `None` |
| `players/cmd/ungroup_many` | `player_ids` | `None` |
| `players/cmd/set_members` | `target_player, player_ids_to_add?, player_ids_to_remove?` | `None` |
| `players/create_group_player` | `provider, name, members, dynamic?` | `Player` (admin) |
| `players/remove_group_player` | `player_id` | `None` (admin) |
| `players/remove` | `player_id` | `None` (admin) |
| `players/add_currently_playing_to_favorites` | `player_id` | `None` |
| `players/player_controls` | (none) | `list[PlayerControl]` |
| `players/player_control` | `control_id` | `PlayerControl \| None` |
| `players/plugin_sources` | (none) | `list[PluginSource]` |
| `players/plugin_source` | `source_id` | `PluginSource \| None` |

### Player Queue Commands

| Command | Parameters | Returns |
|---|---|---|
| `player_queues/all` | (none) | `tuple[PlayerQueue, ...]` |
| `player_queues/get` | `queue_id` | `PlayerQueue \| None` |
| `player_queues/items` | `queue_id, limit=500, offset=0` | `list[QueueItem]` |
| `player_queues/get_active_queue` | `player_id` | `PlayerQueue \| None` |
| `player_queues/play_media` | `queue_id, media, option?, radio_mode=False, start_item?, username?` | `None` |
| `player_queues/play_index` | `queue_id, index, seek_position=0, fade_in=False` | `None` |
| `player_queues/play` | `queue_id` | `None` |
| `player_queues/pause` | `queue_id` | `None` |
| `player_queues/play_pause` | `queue_id` | `None` |
| `player_queues/stop` | `queue_id` | `None` |
| `player_queues/resume` | `queue_id, fade_in?` | `None` |
| `player_queues/next` | `queue_id` | `None` |
| `player_queues/previous` | `queue_id` | `None` |
| `player_queues/skip` | `queue_id, seconds=10` | `None` |
| `player_queues/seek` | `queue_id, position=10` | `None` |
| `player_queues/shuffle` | `queue_id, shuffle_enabled` | `None` |
| `player_queues/repeat` | `queue_id, repeat_mode` | `None` |
| `player_queues/dont_stop_the_music` | `queue_id, dont_stop_the_music_enabled` | `None` |
| `player_queues/set_playback_speed` | `queue_id, speed, queue_item_id?` | `None` |
| `player_queues/move_item` | `queue_id, queue_item_id, pos_shift=1` | `None` |
| `player_queues/move_item_end` | `queue_id, queue_item_id` | `None` |
| `player_queues/delete_item` | `queue_id, item_id_or_index` | `None` |
| `player_queues/clear` | `queue_id, skip_stop=False` | `None` |
| `player_queues/transfer` | `source_queue_id, target_queue_id, auto_play?` | `None` |
| `player_queues/save_as_playlist` | `queue_id, name` | `BackgroundTask` |

### Configuration Commands

**Provider Config:**
| Command | Parameters | Returns |
|---|---|---|
| `config/providers` | `provider_type?, provider_domain?, include_values?` | `list[ProviderConfig]` |
| `config/providers/get` | `instance_id` | `ProviderConfig` |
| `config/providers/get_value` | `instance_id, key, default?, return_type?` | `ConfigValueType` |
| `config/providers/get_entries` | `provider_domain, instance_id?, action?, values?` | `list[ConfigEntry]` |
| `config/providers/save` | `provider_domain, values, instance_id?` | `ProviderConfig` (admin) |
| `config/providers/remove` | `instance_id` | `None` (admin) |
| `config/providers/reload` | `instance_id` | `None` (admin) |

**Player Config:**
| Command | Parameters | Returns |
|---|---|---|
| `config/players` | `provider?, include_values?, include_unavailable?, include_disabled?` | `list[PlayerConfig]` |
| `config/players/get` | `player_id` | `PlayerConfig` |
| `config/players/get_value` | `player_id, key, unpack_splitted_values?, default?` | `ConfigValueType` |
| `config/players/get_entries` | `player_id, action?, values?` | `list[ConfigEntry]` |
| `config/players/save` | `player_id, values` | `PlayerConfig` (admin) |
| `config/players/remove` | `player_id` | `None` (admin) |
| `config/players/dsp/get` | `player_id` | `DSPConfig` |
| `config/players/dsp/save` | `player_id, config` | `DSPConfig` (admin) |

**DSP Presets:**
| Command | Parameters | Returns |
|---|---|---|
| `config/dsp_presets/get` | (none) | `list[DSPConfigPreset]` |
| `config/dsp_presets/save` | `preset` | `DSPConfigPreset` (admin) |
| `config/dsp_presets/remove` | `preset_id` | `None` (admin) |

**Core Config:**
| Command | Parameters | Returns |
|---|---|---|
| `config/core` | `include_values?` | `list[CoreConfig]` |
| `config/core/get` | `domain` | `CoreConfig` |
| `config/core/get_value` | `domain, key, default?` | `ConfigValueType` |
| `config/core/get_entries` | `domain, action?, values?` | `list[ConfigEntry]` |
| `config/core/save` | `domain, values` | `CoreConfig` (admin) |

### Metadata Commands

| Command | Parameters | Returns |
|---|---|---|
| `metadata/set_default_preferred_language` | `lang` | `None` |
| `metadata/set_preferred_language` | `lang` | `None` |
| `metadata/update_metadata` | `item, force_refresh=False` | `MediaItem` |
| `metadata/get_track_lyrics` | `track` | `tuple[str \| None, str \| None]` |

---

## 5. Data Models

### Artist
```
item_id: str
provider: str
name: str
version: str = ""
uri: str | None
external_ids: set[tuple[ExternalID, str]]
media_type: MediaType = ARTIST
provider_mappings: set[ProviderMapping]
metadata: MediaItemMetadata
favorite: bool = False
position: int | None
date_added: datetime | None
```

### Album
```
(all Artist fields plus:)
media_type: MediaType = ALBUM
year: int | None
artists: list[Artist | ItemMapping]
album_type: AlbumType = UNKNOWN  (ALBUM, SINGLE, LIVE, SOUNDTRACK, COMPILATION, EP)
```

### Track
```
(all base fields plus:)
media_type: MediaType = TRACK
duration: int = 0  (seconds)
artists: list[Artist | ItemMapping]
last_played: int = 0
album: Album | ItemMapping | None
disc_number: int = 0
track_number: int = 0
```

### Playlist
```
(all base fields plus:)
media_type: MediaType = PLAYLIST
owner: str = ""
is_editable: bool = False
is_dynamic: bool = False
supported_mediatypes: set[MediaType] = {TRACK}
```

### Radio
```
(all base fields plus:)
media_type: MediaType = RADIO
duration: int | None
```

### Audiobook
```
(all base fields plus:)
media_type: MediaType = AUDIOBOOK
publisher: str | None
authors: list[str]
narrators: list[str]
duration: int = 0
fully_played: bool | None
resume_position_ms: int | None
```

### Podcast
```
(all base fields plus:)
media_type: MediaType = PODCAST
publisher: str | None
total_episodes: int | None
```

### PodcastEpisode
```
(all base fields plus:)
media_type: MediaType = PODCAST_EPISODE
position: int
podcast: Podcast | ItemMapping
duration: int = 0
fully_played: bool | None
resume_position_ms: int | None
```

### Player
```
player_id: str
provider: str
type: PlayerType  (PLAYER, STEREO_PAIR, GROUP, PROTOCOL)
name: str
available: bool
powered: bool | None
playback_state: PlaybackState  (IDLE, PAUSED, PLAYING)
elapsed_time: float | None
elapsed_time_last_updated: float | None
current_media: PlayerMedia | None
volume_level: int | None
volume_muted: bool | None
group_volume: int | None
group_members: list[str]
synced_to: str | None
active_group: str | None
supported_features: set[PlayerFeature]
source_list: list[PlayerSource]
active_source: str | None
sound_mode_list: list[PlayerSoundMode]
active_sound_mode: str | None
icon: str = "mdi-speaker"
enabled: bool = True
hide_in_ui: bool = False
```

### PlayerQueue
```
queue_id: str
active: bool
display_name: str
available: bool
items: int  (count)
shuffle_enabled: bool = False
repeat_mode: RepeatMode  (OFF, ONE, ALL)
dont_stop_the_music_enabled: bool = False
current_index: int | None
elapsed_time: float = 0
elapsed_time_last_updated: float
state: PlaybackState = IDLE
current_item: QueueItem | None
next_item: QueueItem | None
radio_source: list[MediaItem]
flow_mode: bool = False
resume_pos: int = 0
```

### QueueItem
```
queue_id: str
queue_item_id: str
name: str
duration: int | None
sort_index: int = 0
streamdetails: StreamDetails | None
media_item: PlayableMediaItemType | None
image: MediaItemImage | None
index: int = 0
available: bool = True
```

### MediaItemMetadata
```
description: str | None
review: str | None
explicit: bool | None
images: list[MediaItemImage] | None
genres: set[str] | None
mood: str | None
style: str | None
copyright: str | None
lyrics: str | None
lrc_lyrics: str | None
label: str | None
links: set[MediaItemLink] | None
performers: set[str] | None
preview: str | None
popularity: int | None
release_date: datetime | None
languages: list[str] | None
chapters: list[MediaItemChapter] | None
```

### MediaItemImage
```
type: ImageType  (THUMB, LANDSCAPE, FANART, LOGO, CLEARART, BANNER, etc.)
path: str
provider: str
remotely_accessible: bool = False
```

### ItemMapping (lightweight reference)
```
item_id: str
provider: str
name: str
version: str = ""
uri: str | None
available: bool = True
image: MediaItemImage | None
year: int | None
media_type: MediaType
```

### BrowseFolder
```
item_id: str
provider: str
name: str
media_type: MediaType = FOLDER
path: str = ""
image: MediaItemImage | None
is_playable: bool = False
```

---

## 6. Real-Time Events

After authentication, the server automatically subscribes the client to events. Events are filtered based on the user's `player_filter` setting.

### Event Types

| EventType | object_id | data |
|---|---|---|
| `player_added` | player_id | Player |
| `player_updated` | player_id | Player |
| `player_removed` | player_id | player_id string |
| `player_config_updated` | player_id | PlayerConfig |
| `player_dsp_config_updated` | player_id | DSPConfig |
| `player_options_updated` | player_id | - |
| `dsp_presets_updated` | - | list[DSPConfigPreset] |
| `queue_added` | queue_id | PlayerQueue |
| `queue_updated` | queue_id | PlayerQueue |
| `queue_items_updated` | queue_id | - |
| `queue_time_updated` | queue_id | float (elapsed_time) |
| `media_item_played` | uri | - |
| `media_item_added` | uri | MediaItem |
| `media_item_updated` | uri | MediaItem |
| `media_item_deleted` | uri | MediaItem |
| `providers_updated` | - | list[ProviderInstance] |
| `sync_tasks_updated` | - | list[SyncTask] |
| `tasks_updated` | - | list[BackgroundTask] |
| `music_sync_completed` | - | - |
| `auth_session` | - | - |
| `core_state_updated` | - | CoreState |
| `shutdown` | - | - |

### Event Message Format
```json
{
  "event": "player_updated",
  "object_id": "sonos_kitchen",
  "data": {
    "player_id": "sonos_kitchen",
    "name": "Kitchen Speaker",
    "playback_state": "playing",
    "volume_level": 45,
    ...
  }
}
```

### Client-Side Subscription (Python client)
```python
def subscribe(
    cb_func: EventCallBackType,
    event_filter: EventType | tuple[EventType, ...] | None = None,
    id_filter: str | tuple[str, ...] | None = None,
) -> Callable[[], None]  # returns unsubscribe function
```

---

## 7. Python Client Library

### Installation
```bash
pip install music-assistant-client
```

**Dependencies:** aiohttp, music-assistant-models, orjson. Requires Python >= 3.11.

### Usage Example

```python
import asyncio
import aiohttp
from music_assistant_client import MusicAssistantClient
from music_assistant_models.enums import EventType, MediaType, QueueOption

async def main():
    async with aiohttp.ClientSession() as session:
        client = MusicAssistantClient("http://192.168.1.100:8095", session, token="your-jwt-token")

        # Connect and start listening
        await client.connect()

        # Subscribe to events
        def on_player_update(event):
            print(f"Player {event.object_id} updated: {event.data}")

        unsub = client.subscribe(on_player_update, event_filter=EventType.PLAYER_UPDATED)

        # Start listening (blocks until disconnected)
        # Usually run in a task:
        listen_task = asyncio.create_task(client.start_listening())

        # --- Music Library Operations ---

        # Search
        results = await client.music.search("Bohemian Rhapsody", media_types=[MediaType.TRACK])

        # Browse library
        artists = await client.music.get_library_artists(limit=50, offset=0)
        albums = await client.music.get_library_albums(favorite=True)
        tracks = await client.music.get_library_tracks(search="rock")

        # Get specific items
        artist = await client.music.get_artist("123", "spotify")
        album = await client.music.get_album("456", "spotify")
        album_tracks = await client.music.get_album_tracks("456", "spotify")
        artist_albums = await client.music.get_artist_albums("123", "spotify")

        # Playlists
        playlists = await client.music.get_library_playlists()
        playlist_tracks = await client.music.get_playlist_tracks("789", "library")
        await client.music.create_playlist("My Playlist", "spotify")
        await client.music.add_playlist_tracks("789", ["spotify://track/abc", "spotify://track/def"])

        # Browse provider hierarchy
        browse_items = await client.music.browse("spotify://")
        sub_items = await client.music.browse("spotify://playlists")

        # Favorites
        await client.music.add_item_to_favorites("spotify://track/abc")
        await client.music.remove_item_from_favorites(MediaType.TRACK, "123")

        # Recommendations
        recs = await client.music.recommendations()

        # --- Player Operations ---

        # List players
        players = await client.players.get_players()  # uses players/all

        # Control playback
        await client.players.play("sonos_kitchen")
        await client.players.pause("sonos_kitchen")
        await client.players.stop("sonos_kitchen")
        await client.players.next_track("sonos_kitchen")
        await client.players.previous_track("sonos_kitchen")

        # Volume
        await client.players.volume_set("sonos_kitchen", 50)
        await client.players.volume_up("sonos_kitchen")
        await client.players.volume_mute("sonos_kitchen", True)

        # Power
        await client.players.power("sonos_kitchen", True)

        # Grouping
        await client.players.group("sonos_bedroom", "sonos_kitchen")
        await client.players.ungroup("sonos_bedroom")

        # --- Queue Operations ---

        # Get queues
        queues = client.player_queues.player_queues  # local state
        queue = client.player_queues.get("sonos_kitchen")
        items = await client.player_queues.get_queue_items("sonos_kitchen", limit=100)

        # Play media on a queue
        await client.player_queues.play_media(
            "sonos_kitchen",
            "spotify://track/abc",
            option=QueueOption.PLAY
        )
        # Play multiple items
        await client.player_queues.play_media(
            "sonos_kitchen",
            ["spotify://track/abc", "spotify://track/def"],
            option=QueueOption.REPLACE
        )
        # Radio mode (auto-generate similar tracks)
        await client.player_queues.play_media(
            "sonos_kitchen",
            "spotify://track/abc",
            radio_mode=True
        )

        # Queue controls
        await client.player_queues.play("sonos_kitchen")
        await client.player_queues.pause("sonos_kitchen")
        await client.player_queues.next("sonos_kitchen")
        await client.player_queues.previous("sonos_kitchen")
        await client.player_queues.seek("sonos_kitchen", 120)
        await client.player_queues.shuffle("sonos_kitchen", True)
        await client.player_queues.repeat("sonos_kitchen", RepeatMode.ALL)

        # Queue manipulation
        await client.player_queues.move_item("sonos_kitchen", "queue_item_id", pos_shift=2)
        await client.player_queues.delete_item("sonos_kitchen", "queue_item_id")
        await client.player_queues.clear("sonos_kitchen")

        # Transfer queue between players
        await client.player_queues.transfer("sonos_kitchen", "sonos_bedroom")

        # Save queue as playlist
        await client.player_queues.save_as_playlist("sonos_kitchen", "Party Mix")

        # --- Image URLs ---
        image_url = client.get_image_url(some_image, size=500)
        item_thumb = client.get_media_item_image_url(track, type=ImageType.THUMB, size=300)

        # Cleanup
        unsub()  # unsubscribe from events
        await client.disconnect()

asyncio.run(main())
```

### Client Handler Properties

| Property | Type | Purpose |
|---|---|---|
| `client.music` | Music handler | Library browsing, search, metadata |
| `client.players` | Players handler | Player control, grouping |
| `client.player_queues` | PlayerQueues handler | Queue management, playback |
| `client.config` | Config handler | Server/player/provider configuration |
| `client.auth` | Auth handler | User and token management |
| `client.metadata` | Metadata handler | Metadata updates, lyrics |
| `client.server_info` | ServerInfoMessage | Server version, ID, status |
| `client.providers` | list[ProviderInstance] | Active provider instances |
| `client.provider_manifests` | list[ProviderManifest] | Available provider types |

---

## 8. Other Client Libraries

### Kotlin Multiplatform Mobile App
- Repository: `music-assistant/mobile-app`
- Platforms: Android, iOS, JVM desktop
- Framework: Kotlin Multiplatform + Compose Multiplatform
- Uses the same WebSocket API described above

### No Official TypeScript/JavaScript Client
The Vue 3 frontend communicates directly via WebSocket using the same protocol. A JS/TS client could be built following the same patterns.

### REST API (JSON-RPC)
The same commands available via WebSocket can also be sent as HTTP POST to `/api`:
```bash
curl -X POST http://192.168.1.100:8095/api \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer your-jwt-token" \
  -d '{
    "message_id": "1",
    "command": "music/search",
    "args": {
      "search_query": "Beatles",
      "media_types": ["track", "album"],
      "limit": 10
    }
  }'
```

---

## 9. URI Format

Music Assistant uses URIs to identify items across providers:

```
{provider_domain}://{media_type}/{item_id}
```

Examples:
- `spotify://track/4uLU6hMCjMI75M1A2tKUQC`
- `library://album/42`
- `tidal://artist/1234`
- `filesystem_local://track/path/to/file.flac`

---

## 10. Key Enums Reference

### MediaType
`ARTIST, ALBUM, TRACK, PLAYLIST, RADIO, AUDIOBOOK, PODCAST, PODCAST_EPISODE, FOLDER, ANNOUNCEMENT, FLOW_STREAM, PLUGIN_SOURCE, SOUND_EFFECT, GENRE, UNKNOWN`

### PlaybackState
`IDLE, PAUSED, PLAYING, UNKNOWN`

### PlayerType
`PLAYER, STEREO_PAIR, GROUP, PROTOCOL, UNKNOWN`

### QueueOption
`PLAY, REPLACE, NEXT, REPLACE_NEXT, ADD, UNKNOWN`

### RepeatMode
`OFF, ONE, ALL, UNKNOWN`

### AlbumType
`ALBUM, SINGLE, LIVE, SOUNDTRACK, COMPILATION, EP, UNKNOWN`

### PlayerFeature
`POWER, VOLUME_SET, VOLUME_MUTE, PAUSE, SET_MEMBERS, MULTI_DEVICE_DSP, SEEK, NEXT_PREVIOUS, PLAY_ANNOUNCEMENT, ENQUEUE, SELECT_SOUND_MODE, SELECT_SOURCE, OPTIONS, GAPLESS_PLAYBACK, GAPLESS_DIFFERENT_SAMPLERATE, PLAY_MEDIA, UNKNOWN`

### ImageType
`THUMB, LANDSCAPE, FANART, LOGO, CLEARART, BANNER, CUTOUT, BACK, DISCART, OTHER`

### ProviderType
`MUSIC, PLAYER, METADATA, PLUGIN, CORE, UNKNOWN`

### EventType
`PLAYER_ADDED, PLAYER_UPDATED, PLAYER_REMOVED, PLAYER_CONFIG_UPDATED, PLAYER_DSP_CONFIG_UPDATED, PLAYER_OPTIONS_UPDATED, DSP_PRESETS_UPDATED, QUEUE_ADDED, QUEUE_UPDATED, QUEUE_ITEMS_UPDATED, QUEUE_TIME_UPDATED, MEDIA_ITEM_PLAYED, MEDIA_ITEM_ADDED, MEDIA_ITEM_UPDATED, MEDIA_ITEM_DELETED, PROVIDERS_UPDATED, SYNC_TASKS_UPDATED, TASKS_UPDATED, MUSIC_SYNC_COMPLETED, AUTH_SESSION, CORE_STATE_UPDATED, SHUTDOWN, UNKNOWN`

---

## 11. WebSocket Heartbeat & Limits

- **Heartbeat interval**: 30 seconds
- **Connection timeout**: 10 seconds for setup
- **Max pending messages**: 512 (connection dropped if exceeded)
- **Large result streaming**: 500-item batches with `partial: true`
- **JSON serialization**: Uses `orjson` for performance

---

## 12. Image Proxy

Images can be fetched via the built-in proxy:

```
GET /imageproxy?path={encoded_path}&provider={provider}&size={pixels}
```

The client library also has helper methods:
- `client.get_image_url(image: MediaItemImage, size: int = 0) -> str`
- `client.get_media_item_image_url(item, type=ImageType.THUMB, size=0) -> str | None`

For remote images, the server can proxy through weserv.nl for resizing.
