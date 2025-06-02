#pragma once
#include "../Common.h"
#include <QObject>
#include <QString>

class GameClient : public QObject {
    Q_OBJECT

private:
    SOCKET clientSocket;
    pthread_t receiveThread;
    bool isRunning;

    static void* receiveMsg(void* arg);

public:
    explicit GameClient(QObject* parent = nullptr);
    ~GameClient();
    bool connectToServer(const char* ip, int port);
    void disconnect();

// slots 과 signals는 Qt 프레임워크에서 이벤트 기반 프로그래밍을 위해 사용
// slots은 ui에서 호출하여 서버로 메시지를 전송하는 함수
// signals는 서버로부터 메시지를 수신했을 때 UI에 알리는 역할
public slots:
Q_INVOKABLE void sendMsg(const QString& msg) {
    if (clientSocket != INVALID_SOCKET) {
        QByteArray ba = msg.toUtf8();
        ba.append('\n'); // 서버가 줄바꿈 단위로 메시지를 구분한다면
        send(clientSocket, ba.constData(), ba.size(), 0);
    }
}

signals:
    void messageReceived(const QString message);
};