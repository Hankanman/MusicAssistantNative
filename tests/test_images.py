"""Test image proxy for album art loading."""

import pytest
import aiohttp
from urllib.parse import quote


@pytest.mark.asyncio
async def test_image_proxy_returns_image(ma_client, ma_url, ma_token):
    """Image proxy returns image data."""
    albums = await ma_client.send_command(
        "music/albums/library_items", {"limit": 5}
    )
    album_with_img = None
    for a in (albums or []):
        images = a.get("metadata", {}).get("images", [])
        if images:
            album_with_img = a
            break

    if not album_with_img:
        pytest.skip("No albums with images found")

    img = album_with_img["metadata"]["images"][0]
    path = quote(img["path"], safe="")
    provider = quote(img["provider"], safe="")
    url = f"{ma_url}/imageproxy?path={path}&provider={provider}&size=300"

    async with aiohttp.ClientSession() as session:
        async with session.get(
            url, headers={"Authorization": f"Bearer {ma_token}"}
        ) as resp:
            assert resp.status == 200, f"Image proxy returned {resp.status}"
            content_type = resp.headers.get("Content-Type", "")
            assert "image" in content_type, f"Expected image, got {content_type}"
            data = await resp.read()
            assert len(data) > 0, "Image data is empty"


@pytest.mark.asyncio
async def test_image_proxy_accessible(ma_client, ma_url):
    """Image proxy endpoint responds."""
    albums = await ma_client.send_command(
        "music/albums/library_items", {"limit": 1}
    )
    if not albums:
        pytest.skip("No albums")
    images = albums[0].get("metadata", {}).get("images", [])
    if not images:
        pytest.skip("No images")

    img = images[0]
    path = quote(img["path"], safe="")
    provider = quote(img["provider"], safe="")
    url = f"{ma_url}/imageproxy?path={path}&provider={provider}&size=300"

    async with aiohttp.ClientSession() as session:
        async with session.get(url) as resp:
            # Both 200 (public) and 401 (auth required) are valid
            assert resp.status in (200, 401, 403), \
                f"Unexpected status: {resp.status}"
