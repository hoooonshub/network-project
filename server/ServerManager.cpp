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
        auto it = std::find(players.begin(), players.end(), player);
        if (it != players.end()) {
            players.erase(it);
        }
        pthread_mutex_unlock(&serverMutex);
    }

    void handleCommand(Player* player, const std::string& command) {
        pthread_mutex_lock(&serverMutex);

        if (player->isInLobby() && command == "QUEUE_IN") {
            gameMatchingQueue.push_back(player);
            player->goInQueue();

            send(player->getSocket(), "SUCCESS_IN", 10, 0);

            if (gameMatchingQueue.size() == 4) {
                GameSession* gameSession = new GameSession(gameMatchingQueue);
                activeSessions[gameSession->getSessionId()] = gameSession;

                for (auto player : gameMatchingQueue) {
                    player->goInGame();
                    player->setSessionId(gameSession->getSessionId());
                }
                gameMatchingQueue.clear();

                pthread_t tid;
                pthread_create(&tid, NULL, GameSession::start, gameSession);
                pthread_detach(tid);
            }

            pthread_mutex_unlock(&serverMutex);
            return;
        }

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

