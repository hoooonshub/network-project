#include "../Common.h"
#include "GameClient.cpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    GameClient* gameClient = new GameClient();
    if (!gameClient->connectToServer("127.0.0.1", 9000)) {
        return -1; // 서버 연결 실패
    }

    engine.rootContext()->setContextProperty("gameClient", gameClient);
    engine.load(QUrl::fromLocalFile("client/ui/home.qml"));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}