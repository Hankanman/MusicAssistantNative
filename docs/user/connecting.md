# Connecting to Music Assistant

## Finding Your Server URL

Your Music Assistant server runs on a specific host and port. The default port is **8095**.

| Setup | Typical URL |
|-------|-------------|
| Home Assistant add-on | `http://<ha-ip>:8095` |
| Standalone server | `http://<server-ip>:8095` |
| Docker container | `http://<docker-host>:8095` |

!!! tip "Verify your URL"
    Open the URL in a browser — you should see the Music Assistant web UI. You can also test from the terminal:
    ```bash
    curl http://192.168.1.100:8095/info
    ```

## Authentication Methods

Music Assistant uses JWT tokens for authentication. There are two ways to authenticate from the desktop client, depending on your setup.

### Method 1: Long-Lived Token (Recommended)

This is the most reliable method and works with all setups, including Home Assistant add-on installations.

**Step 1: Create a token in the Music Assistant web UI**

1. Open the Music Assistant web UI in your browser
2. Navigate to **Settings** > **Profile**
3. Scroll down to **Long-lived access tokens**
4. Click **+ Create new token**
5. Enter a name (e.g. "KDE Desktop Client")
6. Click **Create**
7. **Copy the token immediately** — it will only be shown once

**Step 2: Enter the token in the app**

1. In Music Assistant Native, go to **Settings**
2. Enter your **Server URL** (e.g. `http://192.168.1.100:8095`)
3. Paste the token into the **Token** field
4. Leave the username and password fields empty
5. Click **Connect**

!!! success "You should see"
    A green message: "Connected to [Server Name] (v2.x.x)"

!!! info "Token lifetime"
    Long-lived tokens are valid for approximately 10 years. You won't need to refresh them.

### Method 2: Username and Password

This works only if your Music Assistant server has **builtin authentication** enabled (i.e. you created a username/password directly in Music Assistant, not through Home Assistant).

1. Enter your **Server URL**
2. Enter your **Username** and **Password**
3. Click **Connect**

!!! warning "Home Assistant OAuth users"
    If your Music Assistant runs as a Home Assistant add-on and you log in through Home Assistant, username/password login **will not work**. You must use a long-lived token instead.

### Getting a Token via the API

If you prefer the command line:

```bash
# First, get a short-lived token by logging in
curl -X POST http://192.168.1.100:8095/api \
  -H "Content-Type: application/json" \
  -d '{
    "message_id": "1",
    "command": "auth/login",
    "args": {
      "username": "your-username",
      "password": "your-password",
      "provider_id": "builtin",
      "device_name": "KDE Desktop"
    }
  }'
```

Then create a long-lived token:

```bash
curl -X POST http://192.168.1.100:8095/api \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <short-lived-token>" \
  -d '{
    "message_id": "2",
    "command": "auth/token/create",
    "args": {"name": "KDE Desktop Client"}
  }'
```

## Connection Status

The sidebar header shows your connection status:

| Indicator | Meaning |
|-----------|---------|
| :material-circle:{ style="color: green" } Server Name | Connected and authenticated |
| :material-circle:{ style="color: grey" } "Not connected" | Not connected to any server |
| Spinning indicator | Connecting or authenticating |

## Auto-Connect on Startup

After your first successful connection, the app saves your server URL and authentication token to local settings (via QSettings). On subsequent launches, the app automatically reconnects to your server — no need to re-enter credentials each time.

If the saved credentials become invalid (e.g. a revoked token), the app will show an authentication error and you can enter new credentials on the Settings page.

## Disconnecting

Click **Disconnect** on the Settings page to close the connection. The app will retain your server URL for the next session.
