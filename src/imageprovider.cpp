#include "imageprovider.h"
#include "maclient.h"
#include <QNetworkReply>
#include <QQuickTextureFactory>

MaImageResponse::MaImageResponse(const QString &imageUrl, const QSize &requestedSize,
                                 const QString &authToken)
    : m_requestedSize(requestedSize)
{
    QNetworkRequest req;
    req.setUrl(QUrl(imageUrl));
    if (!authToken.isEmpty()) {
        req.setRawHeader("Authorization",
                         QStringLiteral("Bearer %1").arg(authToken).toUtf8());
    }
    auto *reply = m_nam.get(req);
    connect(reply, &QNetworkReply::finished, this, &MaImageResponse::onFinished);
}

QQuickTextureFactory *MaImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

QString MaImageResponse::errorString() const
{
    return m_error;
}

void MaImageResponse::onFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        m_error = QStringLiteral("No reply");
        Q_EMIT finished();
        return;
    }

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        m_error = reply->errorString();
        Q_EMIT finished();
        return;
    }

    auto data = reply->readAll();
    m_image = QImage::fromData(data);

    if (m_image.isNull()) {
        m_error = QStringLiteral("Failed to decode image");
        Q_EMIT finished();
        return;
    }

    if (m_requestedSize.isValid() && !m_requestedSize.isNull()) {
        m_image = m_image.scaled(m_requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    Q_EMIT finished();
}

MaImageProvider::MaImageProvider(MaClient *client)
    : m_client(client)
{
}

QQuickImageResponse *MaImageProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    // id format: "path|provider" from MediaItemModel::extractImageUrl
    auto parts = id.split(QLatin1Char('|'));
    QString path = parts.value(0);
    QString provider = parts.value(1);
    QString url;

    if (path.startsWith(QStringLiteral("http://")) || path.startsWith(QStringLiteral("https://"))) {
        // Remotely accessible image — use proxy for resizing/caching
        int size = requestedSize.isValid() ? qMax(requestedSize.width(), requestedSize.height()) : 300;
        url = m_client->getImageUrl(path, provider, size);
    } else if (!path.isEmpty()) {
        // Local/provider path — must go through proxy
        int size = requestedSize.isValid() ? qMax(requestedSize.width(), requestedSize.height()) : 300;
        url = m_client->getImageUrl(path, provider, size);
    } else {
        // Empty — return empty response
        return new MaImageResponse(QString(), requestedSize, QString());
    }

    return new MaImageResponse(url, requestedSize, m_client->token());
}
