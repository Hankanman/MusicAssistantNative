#pragma once

#include <QQuickAsyncImageProvider>
#include <QNetworkAccessManager>
#include <QThreadPool>

class MaClient;

class MaImageResponse : public QQuickImageResponse
{
    Q_OBJECT

public:
    MaImageResponse(const QString &url, const QSize &requestedSize,
                    QNetworkAccessManager *nam, const QString &authToken = {});

    QQuickTextureFactory *textureFactory() const override;
    QString errorString() const override;

private Q_SLOTS:
    void onFinished();

private:
    QImage m_image;
    QString m_error;
    QSize m_requestedSize;
};

class MaImageProvider : public QQuickAsyncImageProvider
{
public:
    explicit MaImageProvider(MaClient *client);

    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;

private:
    MaClient *m_client;
    QNetworkAccessManager m_nam;
};
