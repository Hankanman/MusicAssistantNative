#include "playermodel.h"
#include "maclient.h"

static QStringList jsonArrayToStringList(const QJsonArray &arr)
{
    QStringList result;
    result.reserve(arr.size());
    for (const auto &v : arr) result.append(v.toString());
    return result;
}

PlayerModel::PlayerModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void PlayerModel::setClient(MaClient *client)
{
    m_client = client;
    connect(m_client, &MaClient::eventReceived, this, &PlayerModel::onEvent);
    connect(m_client, &MaClient::authenticatedChanged, this, [this]() {
        if (m_client->isAuthenticated())
            refresh();
    });
}

int PlayerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_players.size();
}

QVariant PlayerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_players.size())
        return {};

    const auto player = m_players.at(index.row()).toObject();

    switch (role) {
    case PlayerIdRole:
        return player.value(QStringLiteral("player_id")).toString();
    case NameRole:
    case Qt::DisplayRole:
        return player.value(QStringLiteral("name")).toString();
    case ProviderRole:
        return player.value(QStringLiteral("provider")).toString();
    case TypeRole:
        return player.value(QStringLiteral("type")).toString();
    case AvailableRole:
        return player.value(QStringLiteral("available")).toBool(false);
    case PoweredRole:
        return player.value(QStringLiteral("powered")).toBool(false);
    case PlaybackStateRole:
        return player.value(QStringLiteral("playback_state")).toString();
    case VolumeLevelRole:
        return player.value(QStringLiteral("volume_level")).toInt(0);
    case VolumeMutedRole:
        return player.value(QStringLiteral("volume_muted")).toBool(false);
    case IconRole:
        return player.value(QStringLiteral("icon")).toString(QStringLiteral("speaker"));
    case GroupMembersRole:
        return jsonArrayToStringList(player.value(QStringLiteral("group_members")).toArray());
    case CanGroupWithRole:
        return jsonArrayToStringList(player.value(QStringLiteral("can_group_with")).toArray());
    case DisplayNameRole:
        return player.value(QStringLiteral("display_name")).toString(
            player.value(QStringLiteral("name")).toString());
    }

    return {};
}

QHash<int, QByteArray> PlayerModel::roleNames() const
{
    return {
        {PlayerIdRole, "playerId"},
        {NameRole, "name"},
        {ProviderRole, "provider"},
        {TypeRole, "type"},
        {AvailableRole, "available"},
        {PoweredRole, "powered"},
        {PlaybackStateRole, "playbackState"},
        {VolumeLevelRole, "volumeLevel"},
        {VolumeMutedRole, "volumeMuted"},
        {IconRole, "icon"},
        {GroupMembersRole, "groupMembers"},
        {CanGroupWithRole, "canGroupWith"},
        {DisplayNameRole, "displayName"},
    };
}

void PlayerModel::refresh()
{
    if (!m_client) return;

    m_client->sendCommand(QStringLiteral("players/all"), {},
        [this](const QJsonValue &result, const QString &error) {
            if (error.isEmpty() && result.isArray()) {
                beginResetModel();
                m_players = result.toArray();
                endResetModel();
                Q_EMIT countChanged();
            }
        });
}

QString PlayerModel::playerIdAt(int index) const
{
    if (index < 0 || index >= m_players.size()) return {};
    return m_players.at(index).toObject().value(QStringLiteral("player_id")).toString();
}

int PlayerModel::findPlayer(const QString &playerId) const
{
    for (int i = 0; i < m_players.size(); ++i) {
        if (m_players.at(i).toObject().value(QStringLiteral("player_id")).toString() == playerId)
            return i;
    }
    return -1;
}

void PlayerModel::onEvent(const QString &event, const QString &objectId, const QJsonObject &data)
{
    if (event == QStringLiteral("player_added")) {
        beginInsertRows(QModelIndex(), m_players.size(), m_players.size());
        m_players.append(data);
        endInsertRows();
        Q_EMIT countChanged();
    } else if (event == QStringLiteral("player_updated")) {
        int idx = findPlayer(objectId);
        if (idx >= 0) {
            m_players[idx] = data;
            Q_EMIT dataChanged(index(idx), index(idx));
        }
    } else if (event == QStringLiteral("player_removed")) {
        int idx = findPlayer(objectId);
        if (idx >= 0) {
            beginRemoveRows(QModelIndex(), idx, idx);
            m_players.removeAt(idx);
            endRemoveRows();
            Q_EMIT countChanged();
        }
    }
}
