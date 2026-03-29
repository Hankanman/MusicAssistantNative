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
#include "localplayer.h"

// Suppress noisy Qt warnings that aren't actionable
static void messageFilter(QtMsgType type, const QMessageLogContext &/*ctx*/, const QString &msg)
{
    if (msg.contains(QStringLiteral("Could not register app ID")))
        return;
    if (msg.contains(QStringLiteral("Using Qt multimedia with FFmpeg")))
        return;
    // Qt internal cleanup on shutdown — not actionable
    if (msg.contains(QStringLiteral("Timers can only be used with threads started with QThread")))
        return;
    if (msg.contains(QStringLiteral("Cannot create children for a parent that is in a different thread")))
        return;

    // Default handler for everything else
    switch (type) {
    case QtDebugMsg: fprintf(stderr, "[DBG] %s\n", qPrintable(msg)); break;
    case QtInfoMsg: fprintf(stderr, "[INF] %s\n", qPrintable(msg)); break;
    case QtWarningMsg: fprintf(stderr, "[WRN] %s\n", qPrintable(msg)); break;
    case QtCriticalMsg: fprintf(stderr, "[ERR] %s\n", qPrintable(msg)); break;
    case QtFatalMsg: fprintf(stderr, "[FATAL] %s\n", qPrintable(msg)); abort();
    }
    fflush(stderr);
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageFilter);

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
    auto *localPlayer = new LocalPlayer(&app);

    // Wire up
    playerController->setClient(client);
    queueController->setClient(client);
    libraryController->setClient(client);
    playerModel->setClient(client);

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
    engine.rootContext()->setContextProperty(QStringLiteral("LocalPlayer"), localPlayer);

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
