#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KDBusService>

#include "maclient.h"
#include "playercontroller.h"
#include "queuecontroller.h"
#include "librarycontroller.h"
#include "mediaitemmodel.h"
#include "playermodel.h"
#include "queueitemmodel.h"
#include "sendspinclient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    KLocalizedString::setApplicationDomain("musicassistant-native");

    KAboutData aboutData(
        QStringLiteral("musicassistant-native"),
        i18n("Music Assistant Native"),
        QStringLiteral("0.1.0"),
        i18n("A native KDE client for Music Assistant"),
        KAboutLicense::GPL_V3,
        i18n("(c) 2024-2026")
    );
    aboutData.setDesktopFileName(QStringLiteral("io.github.musicassistant.native"));
    KAboutData::setApplicationData(aboutData);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("multimedia-player")));

    KDBusService service(KDBusService::Unique);

    // Create singletons
    auto *client = new MaClient(&app);
    auto *playerController = new PlayerController(&app);
    auto *queueController = new QueueController(&app);
    auto *libraryController = new LibraryController(&app);
    auto *playerModel = new PlayerModel(&app);
    auto *sendspinClient = new SendspinClient(&app);

    // Wire up
    playerController->setClient(client);
    queueController->setClient(client);
    libraryController->setClient(client);
    playerModel->setClient(client);

    // Auto-connect Sendspin when MA auth succeeds
    QObject::connect(client, &MaClient::authenticatedChanged, [client, sendspinClient]() {
        if (client->isAuthenticated()) {
            sendspinClient->connectToServer(client->serverUrl(), client->token());
        } else {
            sendspinClient->disconnect();
        }
    });

    // When player changes, update queue
    QObject::connect(playerController, &PlayerController::currentPlayerIdChanged, [&]() {
        queueController->setCurrentQueueId(playerController->currentPlayerId());
    });

    QQmlApplicationEngine engine;

    // Expose singletons to QML
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("MaClient"), client);
    engine.rootContext()->setContextProperty(QStringLiteral("PlayerController"), playerController);
    engine.rootContext()->setContextProperty(QStringLiteral("QueueController"), queueController);
    engine.rootContext()->setContextProperty(QStringLiteral("LibraryController"), libraryController);
    engine.rootContext()->setContextProperty(QStringLiteral("PlayerModel"), playerModel);
    engine.rootContext()->setContextProperty(QStringLiteral("SendspinClient"), sendspinClient);

    // Connect to warnings before loading
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { qCritical("QML object creation failed"); },
                     Qt::QueuedConnection);

    // Try both possible resource paths (depends on Qt QTP0001 policy)
    const QUrl url1(QStringLiteral("qrc:/io/github/musicassistant/native/qml/Main.qml"));
    const QUrl url2(QStringLiteral("qrc:/qt/qml/io/github/musicassistant/native/qml/Main.qml"));

    engine.load(url1);
    if (engine.rootObjects().isEmpty())
        engine.load(url2);

    if (engine.rootObjects().isEmpty()) {
        qCritical("Failed to load QML - no root objects created");
        return -1;
    }

    return app.exec();
}
