#pragma once

#include <QObject>
#include <QJsonObject>
#include <QQmlEngine>

#include "queueitemmodel.h"

class MaClient;

class QueueController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString currentQueueId READ currentQueueId NOTIFY currentQueueIdChanged)
    Q_PROPERTY(bool shuffleEnabled READ shuffleEnabled NOTIFY queueStateChanged)
    Q_PROPERTY(QString repeatMode READ repeatMode NOTIFY queueStateChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY queueStateChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY queueStateChanged)
    Q_PROPERTY(QString currentItemName READ currentItemName NOTIFY queueStateChanged)
    Q_PROPERTY(bool dontStopTheMusic READ dontStopTheMusic NOTIFY queueStateChanged)
    Q_PROPERTY(bool flowMode READ flowMode NOTIFY queueStateChanged)
    Q_PROPERTY(QString activePlaylist READ activePlaylist NOTIFY queueStateChanged)
    Q_PROPERTY(QueueItemModel *itemModel READ itemModel CONSTANT)

public:
    explicit QueueController(QObject *parent = nullptr);

    void setClient(MaClient *client);

    QString currentQueueId() const;
    void setCurrentQueueId(const QString &id);
    bool shuffleEnabled() const;
    QString repeatMode() const;
    int currentIndex() const;
    int itemCount() const;
    QString currentItemName() const;
    bool dontStopTheMusic() const;
    bool flowMode() const;
    QString activePlaylist() const;
    QueueItemModel *itemModel() const;

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void playPause();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void seek(int position);
    Q_INVOKABLE void setShuffle(bool enabled);
    Q_INVOKABLE void setRepeat(const QString &mode);
    Q_INVOKABLE void playMedia(const QString &uri, const QString &option = QStringLiteral("play"));
    Q_INVOKABLE void playMediaList(const QStringList &uris, const QString &option = QStringLiteral("replace"));
    Q_INVOKABLE void playIndex(int index);
    Q_INVOKABLE void removeItem(const QString &itemId);
    Q_INVOKABLE void moveItem(const QString &itemId, int shift);
    Q_INVOKABLE void clearQueue();

Q_SIGNALS:
    void currentQueueIdChanged();
    void queueStateChanged();
    void queueItemsChanged();

private Q_SLOTS:
    void onEvent(const QString &event, const QString &objectId, const QJsonObject &data);

private:
    void sendQueueCommand(const QString &cmd, const QJsonObject &extraArgs = {});
    void fetchQueueState();
    void fetchQueueItems();

    MaClient *m_client = nullptr;
    QString m_currentQueueId;
    QJsonObject m_queueState;
    QueueItemModel *m_itemModel;
};
