#pragma once

#include <QAbstractListModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QQmlEngine>

class MaClient;

class PlayerModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        PlayerIdRole = Qt::UserRole + 1,
        NameRole,
        ProviderRole,
        TypeRole,
        AvailableRole,
        PoweredRole,
        PlaybackStateRole,
        VolumeLevelRole,
        VolumeMutedRole,
        IconRole,
    };
    Q_ENUM(Roles)

    explicit PlayerModel(QObject *parent = nullptr);

    void setClient(MaClient *client);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString playerIdAt(int index) const;

Q_SIGNALS:
    void countChanged();

private Q_SLOTS:
    void onEvent(const QString &event, const QString &objectId, const QJsonObject &data);

private:
    int findPlayer(const QString &playerId) const;

    MaClient *m_client = nullptr;
    QJsonArray m_players;
};
