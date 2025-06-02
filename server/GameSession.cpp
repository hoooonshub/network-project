#pragma once

#include "../Common.h"
#include "Player.cpp"
#include "Deck.cpp"
#include "Card.cpp"

struct Command {
    Player* player;
    std::string playing;

    Command(Player* p, const std::string& cmd) : player(p), playing(cmd) {}
};

class GameSession {
private:
    static int nextSessionId;

    int sessionId;
    std::vector<Player*> gamePlayers;
    std::unordered_map<Player*, std::vector<Card>> playerHands;
    int currentPlayerIndex;

    Deck drawDeck;  // 플레이어가 카드를 뽑는 덱
    Deck dummyDeck; // 제출한 카드를 모아놓은 덱
    Card topCard;

    std::queue<Command> playingCommands;
    pthread_mutex_t sessionMutex;
    pthread_cond_t playingCond;

    bool isGameOver = false;

    void sleep(int seconds) {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
    }

    void passturn() {
        currentPlayerIndex = (currentPlayerIndex + 1) % gamePlayers.size();
        printf("Current player index: %d\n", currentPlayerIndex);

        for (int i = 0; i < gamePlayers.size(); i++) {
            if (i == currentPlayerIndex) {
                send(gamePlayers[i]->getSocket(), ("YOUR_TURN " + std::to_string(currentPlayerIndex)).c_str(), 11, 0);
            } else {
                send(gamePlayers[i]->getSocket(), ("NOT_YOUR_TURN " + std::to_string(currentPlayerIndex)).c_str(), 15, 0);
            }
        }
    }

    void replayTurn() {
        if (currentPlayerIndex == 0) {
            currentPlayerIndex = 3;
        } else {
            currentPlayerIndex--;
        }
    }

    void discard(Player* player, Card card) {
        std::vector<Card>& hand = playerHands.find(player)->second;
        auto cardIt = std::find(hand.begin(), hand.end(), card);
        if (cardIt != hand.end()) {
            hand.erase(cardIt);
            dummyDeck.add(card);
            topCard = card;
        }
    }

    bool processCommand(Command& command) {
        if (command.playing == "DRAW") {
            Card card = drawDeck.draw();
            playerHands[command.player].push_back(card);
            std::string handMsg = "HAND " + joinCards(playerHands[command.player]) + "\n";
            sendToAll(handMsg.c_str(), handMsg.length(), 0);

            return true;
        }

        if (command.playing.substr(0, 4) == "PLAY") {
            std::string cardStr = command.playing.substr(5);
            Card playedCard = Card::fromString(cardStr); // 가정: Card 클래스에 fromString 메소드가 있음

            auto& hand = playerHands[command.player];
            auto it = std::find(hand.begin(), hand.end(), playedCard);
            if (it == hand.end()) {
                return false;
            }
            
            if (playedCard.canDiscard(topCard)) {
                discard(command.player, playedCard);
                sendToAll(("DISCARD " + playedCard.toString()).c_str(), 12, 0);
                return true;
            } else {
                replayTurn();
                return false; 
            }
        }

        return false;
    }

public:
    GameSession(std::vector<Player*> players) : sessionId(nextSessionId++), gamePlayers(players), currentPlayerIndex(-1),
            drawDeck(Deck::newOne()), dummyDeck(Deck::dummy()), topCard(drawDeck.draw()) {
        pthread_mutex_init(&sessionMutex, nullptr);
        pthread_cond_init(&playingCond, nullptr);

        sleep(1);
    }

    static void* start(void* arg) {
        GameSession* session = static_cast<GameSession*>(arg);
        session->run();
        return nullptr;
    }

    void pushCommand(Player* player, const std::string command) {
        pthread_mutex_lock(&sessionMutex);

        playingCommands.push(Command(player, command));
        pthread_cond_signal(&playingCond);

        pthread_mutex_unlock(&sessionMutex);
    }

    void sendToAll(const std::string& message, size_t length, int flags) {
        pthread_mutex_lock(&sessionMutex);
        for (Player* player : gamePlayers) {
            send(player->getSocket(), message.c_str(), length, flags);
        }
        pthread_mutex_unlock(&sessionMutex);
    }

    std::string joinCards(std::vector<Card>& cards) {
        std::string result;
        for (Card& card : cards) {
            result += card.toString() + ",";
        }
        return result;
    }

    void run() {
        sendToAll("GAME_START\n", 11, 0);
        sleep(2);

        sendToAll(("DISCARD " + topCard.toString()).c_str(), 11, 0);
        sleep(1);

        for (int i = 0; i < gamePlayers.size(); i++) {
            std::string indexMsg = "INDEX " + std::to_string(i) + "\n";
            send(gamePlayers[i]->getSocket(), indexMsg.c_str(), indexMsg.length(), 0);
        }

        sleep(1);

        // 처음 7장씩 카드 나누기
        for (Player* player : gamePlayers) {
            std::vector<Card> hand;
            for (int i = 0; i < 7; i++) {
                hand.push_back(drawDeck.draw());
            }
            playerHands[player] = hand;
            std::string handMsg = "HAND " + joinCards(hand) + "\n";
            send(player->getSocket(), handMsg.c_str(), handMsg.length(), 0);
        }
        sleep(1);

        while (!isGameOver) {
            pthread_mutex_lock(&sessionMutex);
            passturn();
            sleep(1);

            while (playingCommands.empty()) {
                pthread_cond_wait(&playingCond, &sessionMutex);
            }

            Command command = playingCommands.front();
            playingCommands.pop();
            pthread_mutex_unlock(&sessionMutex);

            if (command.player != gamePlayers[currentPlayerIndex]) {
                send(command.player->getSocket(), ("Failed_PLAY" + std::to_string(currentPlayerIndex)).c_str(), 20, 0);
                replayTurn();
            } else {
                processCommand(command);
            }
            sleep(1);

            if (playerHands[command.player].empty()) {
                for (Player* player : gamePlayers) {
                    if (player == command.player) {
                        send(player->getSocket(), "GAME_OVER WIN", 13, 0);
                    } else {
                        send(player->getSocket(), "GAME_OVER LOSE", 14, 0);
                    }
                }

                isGameOver = true;
                break;
            }

            if (drawDeck.isExhausted()) {
                drawDeck.reshuffle(dummyDeck);
            }
        }

        // 게임 종료 로직
        for (Player* player : gamePlayers) {
            player->goInLobby();
            player->setSessionId(-1);
        }
    }

    int getSessionId() {
        return sessionId;
    }
};
