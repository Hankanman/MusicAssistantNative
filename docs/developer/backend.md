# C++ Backend

The backend is a set of C++ classes that manage the WebSocket connection, data models, and controllers. All classes are exposed to QML via context properties.

## MaClient

**File:** `src/maclient.h/cpp`

The core WebSocket client. Manages connection, authentication, command dispatch, and event routing.

### Key Properties (QML-accessible)

| Property | Type | Description |
|----------|------|-------------|
| `connected` | `bool` | TCP WebSocket connected |
| `authenticated` | `bool` | JWT auth completed |
| `serverReady` | `bool` | ServerInfoMessage received |
| `serverName` | `QString` | Server display name |
| `serverVersion` | `QString` | Server version string |
| `serverUrl` | `QString` | Current server URL |

### Key Methods

| Method | Description |
|--------|-------------|
| `connectToServer(url)` | Opens WebSocket to `ws://host/ws` |
| `disconnect()` | Closes connection, resets state |
| `authenticate(token)` | Sends `auth` command with JWT token |
| `loginWithCredentials(user, pass)` | Sends `auth/login`, then authenticates with returned token |
| `sendCommand(cmd, args, callback)` | Sends JSON-RPC command, routes response to callback |
| `getImageUrl(path, provider, size)` | Builds image proxy URL |

### Message Routing

`onTextMessageReceived` dispatches incoming messages:

1. **Has `server_id`** → `handleServerInfo()` → emits `serverReadyChanged`
2. **Has `event`** → `handleEvent()` → emits `eventReceived(event, objectId, data)`
3. **Has `message_id`** → `handleCommandResult()` → invokes stored callback

### Partial Result Accumulation

For large results, the server sends batches with `"partial": true`. MaClient accumulates these in `m_partialResults` (a `QHash<QString, QJsonArray>`) and delivers the complete array to the callback when `"partial": false` arrives.

## PlayerController

**File:** `src/playercontroller.h/cpp`

Manages the currently selected player's state and provides playback commands.

### Properties

All bound to the current player's state from the latest `player_updated` event:

`currentPlayerId`, `playerName`, `playbackState`, `volumeLevel`, `volumeMuted`, `powered`, `currentTrackTitle`, `currentTrackArtist`, `currentTrackAlbum`, `currentTrackImageUrl`, `elapsed`, `duration`, `isPlaying`

### Elapsed Time Tracking

The `elapsed` property interpolates between server updates using a local `QTimer` (1-second interval). When a `player_updated` event arrives, `m_lastElapsed` and `m_lastElapsedUpdate` are reset. Between events, `elapsed()` returns `m_lastElapsed + (now - m_lastElapsedUpdate)`.

## QueueController

**File:** `src/queuecontroller.h/cpp`

Manages the play queue for the current player.

### Properties

`currentQueueId`, `shuffleEnabled`, `repeatMode`, `currentIndex`, `itemCount`, `currentItemName`, `itemModel`

### Queue Item Model

Owns a `QueueItemModel` instance exposed as `itemModel`. When `queue_items_updated` events arrive, it re-fetches the full queue via `player_queues/items`.

## LibraryController

**File:** `src/librarycontroller.h/cpp`

Handles music library browsing and search.

### Models

Owns separate `MediaItemModel` instances for each media type:

`artistsModel`, `albumsModel`, `tracksModel`, `playlistsModel`, `radiosModel`, `searchResultsModel`

### Key Methods

| Method | Description |
|--------|-------------|
| `loadLibrary(type, favoriteOnly)` | Fetches `music/{type}/library_items` |
| `search(query)` | Sends `music/search`, populates `searchResultsModel` |
| `loadAlbumTracks(id, provider)` | Fetches tracks for an album |
| `loadArtistAlbums(id, provider)` | Fetches albums for an artist |
| `addToFavorites(uri)` | Adds item to favorites |
| `removeFromFavorites(type, id)` | Removes from favorites |

## Data Models

All models extend `QAbstractListModel` and expose role-based data to QML.

### MediaItemModel

**Roles:** `itemId`, `name`, `mediaType`, `provider`, `uri`, `imageUrl`, `artistName`, `albumName`, `duration`, `trackNumber`, `year`, `favorite`, `version`, `rawData`

Image URLs and artist/album names are extracted from nested JSON structures via static helper methods.

### PlayerModel

**Roles:** `playerId`, `name`, `provider`, `type`, `available`, `powered`, `playbackState`, `volumeLevel`, `volumeMuted`, `icon`

Listens to `player_added`, `player_updated`, and `player_removed` events for real-time updates without re-fetching.

### QueueItemModel

**Roles:** `queueItemId`, `name`, `duration`, `imageUrl`, `artistName`, `albumName`, `itemIndex`, `available`

## MaImageProvider

**File:** `src/imageprovider.h/cpp`

A `QQuickAsyncImageProvider` that loads album art via the Music Assistant image proxy.

### URL Format in QML

```qml
Image {
    source: "image://ma/" + model.imageUrl
}
```

The `id` passed to `requestImageResponse` is the image path. The provider builds the full proxy URL and adds the `Authorization: Bearer` header.
