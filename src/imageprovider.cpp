#include "imageprovider.h"
#include "maclient.h"
#include <QNetworkReply>
#include <QQuickTextureFactory>

MaImageResponse::MaImageResponse(const QString &imageUrl, const QSize &requestedSize,
                                 QNetworkAccessManager *nam, const QString &authToken)
    : m_requestedSize(requestedSize)
{
    QNetworkRequest req;
    req.setUrl(QUrl(imageUrl));
    if (!authToken.isEmpty()) {
        req.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(authToken).toUtf8());
    }
    auto *reply = nam->get(req);
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
    // id format: "path|provider|size" or just a full URL
    auto parts = id.split(QLatin1Char('|'));
    QString url;

    if (parts.size() >= 2) {
        int size = requestedSize.isValid() ? qMax(requestedSize.width(), requestedSize.height()) : 300;
        url = m_client->getImageUrl(parts.at(0), parts.at(1), size);
    } else {
        // Assume it's already a full URL
        url = id;
    }

    return new MaImageResponse(url, requestedSize, &m_nam, m_client->token());
}
