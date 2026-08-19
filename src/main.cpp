#include "qt/AppController.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include <cstdlib>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("MoneSys"));
    QCoreApplication::setOrganizationName(QStringLiteral("MoneSys"));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    monesys::AppController controller;
    QQmlApplicationEngine engine;

    // Qt 6.5+ includes qrc:/qt/qml in the default QML import path via
    // QTP0001. Qt 6.4 does not, so add it explicitly for our minimum
    // supported version while keeping the modern resource layout.
    engine.addImportPath(QStringLiteral("qrc:/qt/qml"));
    engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] {
        QCoreApplication::exit(EXIT_FAILURE);
    }, Qt::QueuedConnection);

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/MoneSys/Main.qml")));
    controller.start();
    return app.exec();
}
