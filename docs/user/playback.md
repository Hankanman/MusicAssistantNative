# Controlling Playback

## Local Playback via Sendspin

Music Assistant Native registers itself as a real player in your Music Assistant server using the **Sendspin audio protocol** (WebSocket on port 8927). This means:

- The app appears in the Music Assistant player list alongside your other speakers
- You can play music directly through your **PC speakers** without any additional hardware
- Audio is streamed as FLAC frames from the server and decoded locally — no ffmpeg or external dependencies needed
- The app's Sendspin player is **automatically selected** as the default player on startup

When you select a track from the library, playback always starts from the beginning (the play option is "replace" — the queue is cleared and the selected track begins playing).

### Play Buttons on Library Items

Each item in the library (tracks, albums, playlists, etc.) has an explicit **play button** for quick playback. Click it to immediately start playing that item on the active player.

## Now Playing Page

The **Now Playing** page (accessible from the sidebar) shows the current track with full controls:

- **Album art** — large artwork for the currently playing track
- **Track info** — title, artist, and album name
- **Seek bar** — drag to jump to any position in the track
- **Time display** — elapsed and total duration

### Playback Controls

| Button | Action |
|--------|--------|
| :material-skip-previous: | Previous track |
| :material-play: / :material-pause: | Play / Pause |
| :material-skip-next: | Next track |
| :material-shuffle: | Toggle shuffle (highlighted when active) |
| :material-repeat: | Cycle repeat mode: Off → All → One → Off |

## Bottom Player Bar

A persistent player bar is positioned at the window level via the `contentItem`, independent of the sidebar. It appears at the bottom of every page once you're connected and have selected a player. It provides quick access to:

- **Track thumbnail** — small album art
- **Track info** — title and artist
- **Transport controls** — previous, play/pause, next
- **Volume** — mute toggle and slider (0–100%)

The bottom bar is always visible, regardless of which page you're on.

## Volume Control

- **Slider** — drag to set volume (0–100%)
- **Speaker icon** — click to toggle mute
- Volume changes are sent in real-time to the Music Assistant server, which adjusts the physical speaker volume

## Keyboard Shortcuts

!!! info "Coming soon"
    MPRIS2 integration for media key support (play/pause, next, previous) is planned.
