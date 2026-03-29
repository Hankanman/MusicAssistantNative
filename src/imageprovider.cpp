#include "imageprovider.h"
#include "maclient.h"
#include <QNetworkReply>
#include <QQuickTextureFactory>
#include <QUrl>

MaImageResponse::MaImageResponse(const QString &imageUrl, const QSize &requestedSize,
                                 const QString &authToken, const QString &directUrl)
    : m_requestedSize(requestedSize)
    , m_directUrl(directUrl)
{
    if (imageUrl.isEmpty()) {
        m_error = QStringLiteral("Empty URL");
        QMetaObject::invokeMethod(this, &MaImageResponse::finished, Qt::QueuedConnection);
        return;
    }

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
        // If proxy failed and we have a direct URL, try that instead
        if (!m_directUrl.isEmpty()) {
            QString fallback = m_directUrl;
            m_directUrl.clear(); // prevent infinite loop
            QNetworkRequest req;
            req.setUrl(QUrl(fallback));
            auto *retryReply = m_nam.get(req);
            connect(retryReply, &QNetworkReply::finished, this, &MaImageResponse::onFinished);
            return;
        }
        m_error = reply->errorString();
        Q_EMIT finished();
        return;
    }

    auto data = reply->readAll();
    m_image = QImage::fromData(data);

    if (m_image.isNull()) {
        // Maybe proxy returned an error page, try direct URL
        if (!m_directUrl.isEmpty()) {
            QString fallback = m_directUrl;
            m_directUrl.clear();
            QNetworkRequest req;
            req.setUrl(QUrl(fallback));
            auto *retryReply = m_nam.get(req);
            connect(retryReply, &QNetworkReply::finished, this, &MaImageResponse::onFinished);
            return;
        }
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
    QString decoded = QUrl::fromPercentEncoding(id.toUtf8());
    auto parts = decoded.split(QLatin1Char('|'));
    QString path = parts.value(0);
    QString provider = parts.value(1);

    if (path.isEmpty()) {
        return new MaImageResponse(QString(), requestedSize);
    }

    int size = requestedSize.isValid() ? qMax(requestedSize.width(), requestedSize.height()) : 300;
    QString proxyUrl = m_client->getImageUrl(path, provider, size);

    // If path is a remote URL, pass it as fallback in case proxy 404s
    QString directUrl;
    if (path.startsWith(QStringLiteral("http://")) || path.startsWith(QStringLiteral("https://"))) {
        directUrl = path;
    }

    return new MaImageResponse(proxyUrl, requestedSize, m_client->token(), directUrl);
}
