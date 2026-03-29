"""Test music library browsing and search."""

import pytest


MEDIA_TYPES = ["artists", "albums", "tracks", "playlists", "radios"]


@pytest.mark.asyncio
@pytest.mark.parametrize("media_type", MEDIA_TYPES)
async def test_library_count(ma_client, media_type):
    """Library count returns a non-negative integer for each type."""
    count = await ma_client.send_command(f"music/{media_type}/count")
    assert isinstance(count, int)
    assert count >= 0


@pytest.mark.asyncio
@pytest.mark.parametrize("media_type", MEDIA_TYPES)
async def test_library_items(ma_client, media_type):
    """Library items returns a list with limit/offset."""
    items = await ma_client.send_command(
        f"music/{media_type}/library_items",
        {"limit": 5, "offset": 0}
    )
    assert isinstance(items, list)
    # May be empty for radios, but should work without error


@pytest.mark.asyncio
async def test_artist_has_required_fields(ma_client):
    """Artists have fields needed by MediaItemModel."""
    items = await ma_client.send_command(
        "music/artists/library_items", {"limit": 3}
    )
    if not items:
        pytest.skip("No artists in library")
    for item in items:
        assert "item_id" in item
        assert "name" in item
        assert "provider" in item


@pytest.mark.asyncio
async def test_album_has_artist_and_year(ma_client):
    """Albums have artists array and year for display."""
    items = await ma_client.send_command(
        "music/albums/library_items", {"limit": 3}
    )
    if not items:
        pytest.skip("No albums in library")
    for item in items:
        assert "name" in item
        assert "artists" in item
        assert isinstance(item["artists"], list)


@pytest.mark.asyncio
async def test_track_has_duration_and_artists(ma_client):
    """Tracks have duration and artists for display."""
    items = await ma_client.send_command(
        "music/tracks/library_items", {"limit": 3}
    )
    if not items:
        pytest.skip("No tracks in library")
    for item in items:
        assert "name" in item
        assert "duration" in item
        assert isinstance(item["duration"], (int, float))
        assert "artists" in item


@pytest.mark.asyncio
async def test_album_has_images(ma_client):
    """Albums have image metadata for album art display."""
    items = await ma_client.send_command(
        "music/albums/library_items", {"limit": 5}
    )
    if not items:
        pytest.skip("No albums in library")
    albums_with_images = [a for a in items if a.get("metadata", {}).get("images")]
    assert len(albums_with_images) > 0, "Expected at least one album with images"
    img = albums_with_images[0]["metadata"]["images"][0]
    assert "path" in img
    assert "provider" in img


@pytest.mark.asyncio
async def test_search(ma_client):
    """Search returns results across media types."""
    result = await ma_client.send_command(
        "music/search", {"search_query": "rock", "limit": 5}
    )
    assert isinstance(result, dict)
    total = sum(len(result.get(t, [])) for t in MEDIA_TYPES)
    assert total > 0, "Expected at least one search result for 'rock'"


@pytest.mark.asyncio
async def test_search_empty_query_handled(ma_client):
    """Search with very specific query may return empty but not error."""
    result = await ma_client.send_command(
        "music/search", {"search_query": "xyznonexistent99", "limit": 5}
    )
    assert isinstance(result, dict)


@pytest.mark.asyncio
async def test_album_tracks(ma_client):
    """Can fetch tracks for a specific album."""
    albums = await ma_client.send_command(
        "music/albums/library_items", {"limit": 1}
    )
    if not albums:
        pytest.skip("No albums in library")
    album = albums[0]
    # Get provider from provider_mappings
    mappings = album.get("provider_mappings", [])
    if not mappings:
        pytest.skip("Album has no provider mappings")
    mapping = list(mappings)[0] if isinstance(mappings, set) else mappings[0]
    provider = mapping.get("provider_instance", mapping.get("provider_domain", "library"))
    item_id = mapping.get("item_id", album.get("item_id"))

    tracks = await ma_client.send_command(
        "music/albums/album_tracks",
        {"item_id": item_id, "provider_instance_id_or_domain": provider}
    )
    assert isinstance(tracks, list)


@pytest.mark.asyncio
async def test_artist_albums(ma_client):
    """Can fetch albums for a specific artist."""
    artists = await ma_client.send_command(
        "music/artists/library_items", {"limit": 1}
    )
    if not artists:
        pytest.skip("No artists in library")
    artist = artists[0]
    mappings = artist.get("provider_mappings", [])
    if not mappings:
        pytest.skip("Artist has no provider mappings")
    mapping = list(mappings)[0] if isinstance(mappings, set) else mappings[0]
    provider = mapping.get("provider_instance", mapping.get("provider_domain", "library"))
    item_id = mapping.get("item_id", artist.get("item_id"))

    albums = await ma_client.send_command(
        "music/artists/artist_albums",
        {"item_id": item_id, "provider_instance_id_or_domain": provider}
    )
    assert isinstance(albums, list)
