"""Test player listing and control commands."""

import pytest


@pytest.mark.asyncio
async def test_list_players(ma_client):
    """Players list returns non-empty list."""
    players = await ma_client.send_command("players/all")
    assert isinstance(players, list)
    assert len(players) > 0, "Expected at least one player"


@pytest.mark.asyncio
async def test_player_has_required_fields(ma_client):
    """Each player has the required fields for the UI."""
    players = await ma_client.send_command("players/all")
    required_fields = {"player_id", "name", "provider", "available", "playback_state"}
    for player in players:
        missing = required_fields - set(player.keys())
        assert not missing, f"Player {player.get('name', '?')} missing fields: {missing}"


@pytest.mark.asyncio
async def test_get_single_player(ma_client):
    """Can fetch a specific player by ID."""
    players = await ma_client.send_command("players/all")
    player_id = players[0]["player_id"]
    player = await ma_client.send_command("players/get", {"player_id": player_id})
    assert isinstance(player, dict)
    assert player["player_id"] == player_id


@pytest.mark.asyncio
async def test_player_queues_all(ma_client):
    """Queue listing returns queues matching players."""
    queues = await ma_client.send_command("player_queues/all")
    assert isinstance(queues, (list, tuple))
    assert len(queues) > 0


@pytest.mark.asyncio
async def test_queue_has_required_fields(ma_client):
    """Each queue has the required fields for the UI."""
    queues = await ma_client.send_command("player_queues/all")
    required_fields = {"queue_id", "display_name", "state", "shuffle_enabled", "repeat_mode"}
    for queue in queues[:5]:
        missing = required_fields - set(queue.keys())
        assert not missing, f"Queue {queue.get('display_name', '?')} missing fields: {missing}"
