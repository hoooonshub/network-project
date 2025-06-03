#include "../Common.h"
#include "ServerManager.cpp"
#include "Player.cpp"

#define SERVER_PORT 9000
#define BUFSIZE 512

// 서버에서 클라이언트의 요청을 받아서 처리하는 함수
// recv 로 받은 데이터를 ServerManager#handleCommand로 넘김
void *handle_client(void *arg) {
    SOCKET client_sock = (SOCKET)(long long)arg;
    int retval;

    struct sockaddr_in peeraddr;
    char addr[INET_ADDRSTRLEN];
    socklen_t peerlen = sizeof(peeraddr);
    char buf[BUFSIZE + 1];
    
    getpeername(client_sock, (struct sockaddr *)&peeraddr, &peerlen);
    inet_ntop(AF_INET, &peeraddr.sin_addr, addr, sizeof(addr));

    Player* player = ServerManager::getInstance()->addPlayer(client_sock);

    while (true) {
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv");
            break;
        }

        if (retval <= 0) break;

        buf[retval] = '\0';

        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }

        ServerManager::getInstance()->handleCommand(player, buf);
    }

    ServerManager::getInstance()->removePlayer(player);
    close(client_sock);
    return nullptr;
}

int main() {
    int retval;

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) {
        err_quit("socket");
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVER_PORT);

    retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) {
        err_quit("bind");
    }

    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) {
        err_quit("listen");
    }

    // ------ 여기까지 소켓 생성 및 바인딩, 리스닝 ------

    while (true) {
        // 클라이언트의 요청을 받아서 통신 연결 (accept)
        struct sockaddr_in clientaddr;
        socklen_t addrlen = sizeof(clientaddr);
        SOCKET client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept");
            continue;
        }

        pthread_t tid;
        retval = pthread_create(&tid, NULL, handle_client, (void *)(long long)client_sock);

        if (retval != 0) {
            close(client_sock);
        } else {
            pthread_detach(tid);
        }
    }

    close(listen_sock);
    return 0;
}

ServerManager* ServerManager::instance = nullptr;
int GameSession::nextSessionId = 0;