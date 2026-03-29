# Managing Players

The **Players** page lists all speakers and devices registered with your Music Assistant server — including the app itself.

## This App as a Player

Music Assistant Native registers itself as a real Music Assistant player via the **Sendspin protocol**. This means:

- The app appears in the player list with other speakers (e.g. Sonos, AirPlay)
- Music Assistant can stream audio directly to your PC speakers
- On startup, the app's own Sendspin player is **automatically selected** as the active player, so you can start playing music immediately without manual player selection

## Player List

Each player shows:

| Column | Description |
|--------|-------------|
| **Speaker icon** | Colored when available, grey when unavailable |
| **Name** | Player name as configured in Music Assistant |
| **Provider** | Which integration provides this player (e.g. Sonos, AirPlay) |
| **State** | Current playback state (idle, playing, paused) |
| **Power** | Toggle button to power on/off |
| **Volume** | Slider (0–100%) and percentage display |

## Selecting a Player

Click any player in the list to make it your **active player**. The selected player is highlighted, and all playback commands (play, pause, next, volume, etc.) will be sent to that player.

The bottom player bar updates to show the selected player's current track and state.

## Player Types

Music Assistant supports many player types:

| Type | Examples |
|------|----------|
| **Sonos** | Sonos speakers and soundbars |
| **AirPlay** | Apple TV, HomePod, AirPlay receivers |
| **Chromecast** | Google/Nest speakers and displays |
| **DLNA** | UPnP/DLNA media renderers |
| **Snapcast** | Snapcast multi-room clients |
| **Amazon Alexa** | Echo devices |
| **Sendspin** | Native desktop clients (like this app) |
| **Web** | Browser-based players in the MA web UI |
| **Groups** | Virtual groups of multiple players |

## Volume Control

Drag the volume slider next to any player to adjust its volume. This sends a real-time volume command to Music Assistant, which adjusts the physical speaker.

## Power Control

Click the power button to turn a player on or off. Not all player types support power control — the button is only functional for players that expose this feature.

## Refreshing

Click the **Refresh** button in the toolbar to re-fetch the player list from the server. Players are also updated in real-time via WebSocket events when their state changes.
