# Troubleshooting

## Connection Issues

### App won't start / no window appears

The app enforces a single instance via D-Bus. Kill any existing instance first:

```bash
pkill -f musicassistant-native
musicassistant-native
```

### "Not connected" after entering credentials

1. **Check the URL** — include the port: `http://192.168.1.100:8095`
2. **Verify the server is running**:
   ```bash
   curl http://192.168.1.100:8095/info
   ```
3. **Check your firewall** — port 8095 must be accessible
4. **Check the token** — if expired, create a new long-lived token in the MA web UI

### "Login failed: Invalid username or password"

Your Music Assistant uses Home Assistant authentication (OAuth), not builtin auth. Username/password won't work — use a [long-lived token](connecting.md#method-1-long-lived-token-recommended) instead.

### "Authentication failed" with a token

- The token may have been revoked — check **Settings > Profile > Long-lived access tokens** in the MA web UI
- The token may be corrupted — create a new one and copy it carefully
- Ensure there are no extra spaces or newlines in the token field

### Connection drops after a while

The app sends WebSocket heartbeats every 25 seconds. If your network is unstable, the connection may drop. Reconnect via the Settings page.

## Library Issues

### Library is empty

- Music Assistant needs to sync your music sources first
- Open the MA web UI and go to **Settings > Music sources** to check provider status
- Trigger a sync from the web UI
- After sync completes, refresh the Library page in the app

### Search returns no results

- Ensure you're connected and authenticated
- Try broader search terms — search queries are sent to all configured providers
- Some providers may have limited search capabilities

## Playback Issues

### No players showing up

- Make sure players are configured in Music Assistant
- Check that players are powered on and on the same network
- Click **Refresh** on the Players page
- Some player types (e.g. Chromecast) only appear when they're discoverable on the network

### Clicking a track doesn't play

- Make sure you have a **player selected** on the Players page
- Check that the selected player is available and powered on
- Check the bottom player bar — it should show the selected player name

### No album art

- Album art is loaded via Music Assistant's image proxy
- Ensure your MA server can reach the internet (needed for Spotify/Tidal artwork)
- Images may take a moment to load on first view

## Getting Debug Output

Run the app from a terminal to see debug messages:

```bash
musicassistant-native 2>&1 | grep MaClient
```

Key messages to look for:

| Message | Meaning |
|---------|---------|
| `received ServerInfoMessage` | WebSocket connected, server responded |
| `server ready` | Server info received, ready for auth |
| `sending auth command` | Authentication attempt in progress |
| `authentication successful!` | Fully connected and ready |
| `authentication failed:` | Auth rejected — check token |
| `disconnected` | Connection lost |

## Reporting Bugs

When reporting issues, include:

1. Your Music Assistant server version (shown in the connected status)
2. The debug output from the terminal
3. Your Linux distribution and KDE Plasma version
4. Steps to reproduce the issue
