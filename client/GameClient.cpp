#pragma once
#include "../Common.h"
#include "GameClient.h"

#include <QObject>
#include <QString>

void* GameClient::receiveMsg(void* arg) {
        GameClient* client = static_cast<GameClient*>(arg);
        char buffer[1024];
    
        while (client->isRunning) {
            int bytesRead = recv(client->clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead <= 0) {
                client->isRunning = false;
                break;
            }
    
            buffer[bytesRead] = '\0';
            emit client->messageReceived(QString(buffer));
        }
        return nullptr;
    }

GameClient::GameClient(QObject* parent) : QObject(parent), isRunning(false){}

GameClient::~GameClient() {
        disconnect();
    }

    bool GameClient::connectToServer(const char* ip, int port) {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) return false;
    
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(ip);
    
        if (::connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            close(clientSocket);
            return false;
        }
    
        // 수신 스레드 시작
        isRunning = true;
        pthread_create(&receiveThread, NULL, &GameClient::receiveMsg, this);
        pthread_detach(receiveThread);
    
        return true;
    }

    void GameClient::disconnect() {
        if (isRunning) {
            isRunning = false;
            close(clientSocket);
        }
    }

    

