#pragma once

#include "../Common.h"
#include "Player.cpp"
#include "GameSession.cpp"

class ServerManager {
private:
    static ServerManager* instance;

    std::vector<Player*> players;
    std::vector<Player*> gameMatchingQueue;
    std::unordered_map<int, GameSession*> activeSessions;

    pthread_mutex_t serverMutex;

    ServerManager() {
        pthread_mutex_init(&serverMutex, nullptr);
    }

    ~ServerManager() {
        pthread_mutex_destroy(&serverMutex);
    }

public:
    // 싱글턴 패턴을 사용하여 서버 매니저 인스턴스를 생성
    static ServerManager* getInstance() {
        if (instance == nullptr) {
            instance = new ServerManager();
        }
        return instance;
    }

    Player* addPlayer(SOCKET socket) {
        pthread_mutex_lock(&serverMutex);
        Player* player = new Player(socket);
        players.push_back(player);
        pthread_mutex_unlock(&serverMutex);
        return player;
    }

    void removePlayer(Player* player) {
        pthread_mutex_lock(&serverMutex);
        printf("remove Player\n");
        if (player->isInQueue()) {
            auto it = std::find(gameMatchingQueue.begin(), gameMatchingQueue.end(), player);
            if (it != gameMatchingQueue.end()) {
                gameMatchingQueue.erase(it);
            }
            player->goInLobby();
        }

        if (player->isInGame()) {
            int sessionId = player->getSessionId();
            auto it = activeSessions.find(sessionId);
            if (it != activeSessions.end()) {
                it->second->pushCommand(player, "DISCONNECT", true);
            }
            player->goInLobby();
            player->setSessionId(-1);
        }

        auto it = std::find(players.begin(), players.end(), player);
        if (it != players.end()) {
            players.erase(it);
        }
        pthread_mutex_unlock(&serverMutex);
    }

    // server.cpp에서 받은 데이터를 처리하는 함수; 넘어온 데이터를 파싱하여 올바른 로직 처리
    void handleCommand(Player* player, const std::string& command) {
        pthread_mutex_lock(&serverMutex);

        // 게임 대기열 진입 
        if (player->isInLobby() && command == "QUEUE_IN") {
            gameMatchingQueue.push_back(player);
            player->goInQueue();

            send(player->getSocket(), "SUCCESS_IN", 10, 0);

            // 게임 대기열에 4명이 모이면 게임 세션을 시작
            if (gameMatchingQueue.size() == 4) {
                GameSession* gameSession = new GameSession(gameMatchingQueue);
                activeSessions[gameSession->getSessionId()] = gameSession;

                for (auto player : gameMatchingQueue) {
                    player->goInGame();
                    player->setSessionId(gameSession->getSessionId());
                }
                gameMatchingQueue.clear();

                // 각 게임 세션마다 새로운 스레드를 생성하여 게임을 시작
                pthread_t tid;
                pthread_create(&tid, NULL, GameSession::start, gameSession);
                pthread_detach(tid);
            }

            pthread_mutex_unlock(&serverMutex);
            return;
        }

        // 게임 대기열에서 나가기
        if (player->isInQueue() && command == "QUEUE_OUT") {
            auto it = std::find(gameMatchingQueue.begin(), gameMatchingQueue.end(), player);
            if (it != gameMatchingQueue.end()) {
                gameMatchingQueue.erase(it);
                player->goInLobby();
            }
            send(player->getSocket(), "SUCCESS_OUT", 11, 0);

            pthread_mutex_unlock(&serverMutex);
            return;
        }

        // 게임 중인 플레이어의 요청 시 처리 로직
        if (player->isInGame() && (command.substr(0, 4) == "PLAY" || command == "DRAW")) {
            int sessionId = player->getSessionId();
            auto it = activeSessions.find(sessionId);
            if (it != activeSessions.end()) {
                it->second->pushCommand(player, command);
            }

            pthread_mutex_unlock(&serverMutex);
            return;
        }

        pthread_mutex_unlock(&serverMutex);
    }
};

