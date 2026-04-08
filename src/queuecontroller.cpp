#include "queuecontroller.h"
#include "maclient.h"
#include "queueitemmodel.h"

QueueController::QueueController(QObject *parent)
    : QObject(parent)
    , m_itemModel(new QueueItemModel(this))
{
}

void QueueController::setClient(MaClient *client)
{
    m_client = client;
    connect(m_client, &MaClient::eventReceived, this, &QueueController::onEvent);
}

QString QueueController::currentQueueId() const { return m_currentQueueId; }

void QueueController::setCurrentQueueId(const QString &id)
{
    if (m_currentQueueId != id) {
        m_currentQueueId = id;
        Q_EMIT currentQueueIdChanged();
        fetchQueueState();
        fetchQueueItems();
    }
}

bool QueueController::shuffleEnabled() const
{
    return m_queueState.value(QStringLiteral("shuffle_enabled")).toBool(false);
}

QString QueueController::repeatMode() const
{
    return m_queueState.value(QStringLiteral("repeat_mode")).toString(QStringLiteral("off"));
}

int QueueController::currentIndex() const
{
    return m_queueState.value(QStringLiteral("current_index")).toInt(-1);
}

int QueueController::itemCount() const
{
    return m_queueState.value(QStringLiteral("items")).toInt(0);
}

QString QueueController::currentItemName() const
{
    auto currentItem = m_queueState.value(QStringLiteral("current_item")).toObject();
    return currentItem.value(QStringLiteral("name")).toString();
}

bool QueueController::dontStopTheMusic() const
{
    return m_queueState.value(QStringLiteral("dont_stop_the_music_enabled")).toBool(false);
}

bool QueueController::flowMode() const
{
    return m_queueState.value(QStringLiteral("flow_mode")).toBool(false);
}

QString QueueController::activePlaylist() const
{
    auto extra = m_queueState.value(QStringLiteral("extra_attributes")).toObject();
    return extra.value(QStringLiteral("active_playlist")).toString();
}

QueueItemModel *QueueController::itemModel() const { return m_itemModel; }

void QueueController::play() { sendQueueCommand(QStringLiteral("player_queues/play")); }
void QueueController::pause() { sendQueueCommand(QStringLiteral("player_queues/pause")); }
void QueueController::playPause() { sendQueueCommand(QStringLiteral("player_queues/play_pause")); }
void QueueController::next() { sendQueueCommand(QStringLiteral("player_queues/next")); }
void QueueController::previous() { sendQueueCommand(QStringLiteral("player_queues/previous")); }

void QueueController::seek(int position)
{
    sendQueueCommand(QStringLiteral("player_queues/seek"), {{QStringLiteral("position"), position}});
}

void QueueController::setShuffle(bool enabled)
{
    sendQueueCommand(QStringLiteral("player_queues/shuffle"), {{QStringLiteral("shuffle_enabled"), enabled}});
}

void QueueController::setRepeat(const QString &mode)
{
    sendQueueCommand(QStringLiteral("player_queues/repeat"), {{QStringLiteral("repeat_mode"), mode}});
}

void QueueController::playMedia(const QString &uri, const QString &option)
{
    if (!m_client || m_currentQueueId.isEmpty()) return;

    QJsonObject args;
    args[QStringLiteral("queue_id")] = m_currentQueueId;
    args[QStringLiteral("media")] = uri;
    args[QStringLiteral("option")] = option;
    m_client->sendCommand(QStringLiteral("player_queues/play_media"), args,
        [uri](const QJsonValue &, const QString &error) {
            if (!error.isEmpty()) {
                qWarning() << "QueueController: playMedia failed:" << error << "uri:" << uri;
            }
        });
}

void QueueController::playMediaList(const QStringList &uris, const QString &option)
{
    if (!m_client || m_currentQueueId.isEmpty()) return;

    QJsonObject args;
    args[QStringLiteral("queue_id")] = m_currentQueueId;
    QJsonArray mediaArr;
    for (const auto &uri : uris)
        mediaArr.append(uri);
    args[QStringLiteral("media")] = mediaArr;
    args[QStringLiteral("option")] = option;
    m_client->sendCommand(QStringLiteral("player_queues/play_media"), args);
}

void QueueController::playIndex(int index)
{
    sendQueueCommand(QStringLiteral("player_queues/play_index"), {{QStringLiteral("index"), index}});
}

void QueueController::removeItem(const QString &itemId)
{
    sendQueueCommand(QStringLiteral("player_queues/delete_item"), {{QStringLiteral("item_id_or_index"), itemId}});
}

void QueueController::moveItem(const QString &itemId, int shift)
{
    QJsonObject args;
    args[QStringLiteral("queue_item_id")] = itemId;
    args[QStringLiteral("pos_shift")] = shift;
    sendQueueCommand(QStringLiteral("player_queues/move_item"), args);
}

void QueueController::clearQueue()
{
    sendQueueCommand(QStringLiteral("player_queues/clear"));
}

void QueueController::sendQueueCommand(const QString &cmd, const QJsonObject &extraArgs)
{
    if (!m_client || m_currentQueueId.isEmpty()) return;

    QJsonObject args;
    args[QStringLiteral("queue_id")] = m_currentQueueId;
    for (auto it = extraArgs.begin(); it != extraArgs.end(); ++it) {
        args.insert(it.key(), it.value());
    }
    m_client->sendCommand(cmd, args);
}

void QueueController::fetchQueueState()
{
    if (!m_client || m_currentQueueId.isEmpty()) return;

    QJsonObject args;
    args[QStringLiteral("queue_id")] = m_currentQueueId;
    m_client->sendCommand(QStringLiteral("player_queues/get"), args,
        [this](const QJsonValue &result, const QString &error) {
            if (error.isEmpty() && result.isObject()) {
                m_queueState = result.toObject();
                Q_EMIT queueStateChanged();
            }
        });
}

void QueueController::fetchQueueItems()
{
    if (!m_client || m_currentQueueId.isEmpty()) return;

    QJsonObject args;
    args[QStringLiteral("queue_id")] = m_currentQueueId;
    args[QStringLiteral("limit")] = 500;
    args[QStringLiteral("offset")] = 0;
    m_client->sendCommand(QStringLiteral("player_queues/items"), args,
        [this](const QJsonValue &result, const QString &error) {
            if (error.isEmpty() && result.isArray()) {
                m_itemModel->setItems(result.toArray());
                Q_EMIT queueItemsChanged();
            }
        });
}

void QueueController::onEvent(const QString &event, const QString &objectId, const QJsonObject &data)
{
    if (objectId != m_currentQueueId) return;

    if (event == QStringLiteral("queue_updated")) {
        m_queueState = data;
        Q_EMIT queueStateChanged();
    } else if (event == QStringLiteral("queue_items_updated")) {
        fetchQueueItems();
    }
}
