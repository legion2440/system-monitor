#include "qt/AppController.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include <cstdlib>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("MoneSys"));
    QCoreApplication::setOrganizationName(QStringLiteral("MoneSys"));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    monesys::AppController controller;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] {
        QCoreApplication::exit(EXIT_FAILURE);
    }, Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("MoneSys"), QStringLiteral("Main"));
    controller.start();
    return app.exec();
}
