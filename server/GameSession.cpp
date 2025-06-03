#pragma once

#include "../Common.h"
#include "Player.cpp"
#include "Deck.cpp"
#include "Card.cpp"

struct Command {
    Player* player;
    std::string playing;
    bool isDisconnect;

    Command(Player* p, const std::string& cmd, bool dis = false) : player(p), playing(cmd), isDisconnect(dis) {}
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
        do {
            currentPlayerIndex = (currentPlayerIndex + 1) % gamePlayers.size();
        } while (gamePlayers[currentPlayerIndex] == nullptr);

        sendToAll("TURN " + std::to_string(currentPlayerIndex), 6, 0);
    }

    // 데이터가 잘못 넘어와서 현재 턴의 플레이어가 한 번 더 플레이하기 위한 처리 함수
    // passturn 전에 호출되어 현재 플레이어의 인덱스를 이전으로 되돌림
    void replayTurn() {
        if (currentPlayerIndex == 0) {
            currentPlayerIndex = 3;
        } else {
            currentPlayerIndex--;
        }
    }

    // 플레이어가 카드를 제출하면 핸드 카드에서 제거하고, 더미 덱의 가장 위에 있는 카드로 설정
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
        // 플레이어가 카드를 draw하는 경우
        // draw 덱에서 뽑아서 플레이어 핸드에 추가
        if (command.playing == "DRAW") {
            Card card = drawDeck.draw();
            playerHands[command.player].push_back(card);
            std::string handMsg = "HAND " + joinCards(playerHands[command.player]) + "\n";
            sendToAll(handMsg.c_str(), handMsg.length(), 0);

            return true;
        }

        // 플레이어가 카드를 제출하는 경우
        if (command.playing.substr(0, 4) == "PLAY") {
            std::string cardStr = command.playing.substr(5);
            Card playedCard = Card::fromString(cardStr);

            // 제출할 카드가 실제 플레이어의 핸드에 있는지 검증
            auto& hand = playerHands[command.player];
            auto it = std::find(hand.begin(), hand.end(), playedCard);
            if (it == hand.end()) {
                return false;
            }
            
            // 제출할 수 있는 카드라면 -> 카드를 더미 덱에 추가하고, 핸드에서 제거
            // 불가하다면 -> 올바른 카드가 아니므로 턴을 다시 플레이하도록 함 (replayTurn 호출)
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

    void handleDisconnect(Player* player) {
        pthread_mutex_lock(&sessionMutex);
        printf("disconnect handling\n");

        // 플레이어 인덱스 찾기
        auto it = std::find(gamePlayers.begin(), gamePlayers.end(), player);
        if (it == gamePlayers.end()) {
            pthread_mutex_unlock(&sessionMutex);
            return;
        }
        int disconnectedIndex = std::distance(gamePlayers.begin(), it);

        // 플레이어 카드 -> dummyDeck에 추가
        auto handIt = playerHands.find(player);
        if (handIt != playerHands.end()) {
            for (const Card& card : handIt->second) {
                dummyDeck.add(card);
            }
            playerHands.erase(handIt);
        }

        // 플레이어 게임에서 제거
        gamePlayers[disconnectedIndex] = nullptr;

        // 클라이언트에 DISCONNECT 메시지 전송
        std::string disconnectMsg = "DISCONNECT " + std::to_string(disconnectedIndex) + "\n";
        sendToAll(disconnectMsg.c_str(), disconnectMsg.length(), 0);
       
        int activePlayers = std::count_if(gamePlayers.begin(), gamePlayers.end(), 
        [](Player* p) { return p != nullptr; });

        // 한 명 남으면 게임 종료 처리
        // 현재 턴이 나간 플레이어라면 다음 플레이어로 턴 넘기기
        if (activePlayers == 1) {
            isGameOver = true;
            for (size_t i = 0; i < gamePlayers.size(); i++) {
                if (gamePlayers[i] != nullptr) {
                    std::string winMsg = "GAME_OVER WIN";
                    send(gamePlayers[i]->getSocket(), winMsg.c_str(), winMsg.length(), 0);
                    sleep(1);
                    break;
                }
            }
            printf("Game--over\n");
        } else if (activePlayers >= 2) {
            if (gamePlayers[currentPlayerIndex] == nullptr) {
                sleep(1);
                passturn();
            }
        }
        
        pthread_mutex_unlock(&sessionMutex);
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

    // 게임 중인 플레이어의 요청을 ServerManager에서 넘겨받아 처리
    // 뮤텍스 & 조건 변수를 이용하여 플레이 명령어 큐에 넣고 조건 변수 시그널 알림
    // run 함수의 대기 중이던 pthread_cond_wait가 깨어 명령어를 처리하도록 함
    void pushCommand(Player* player, const std::string command, bool isDisconnect = false) {
        pthread_mutex_lock(&sessionMutex);

        playingCommands.push(Command(player, command, isDisconnect));
        pthread_cond_signal(&playingCond);

        pthread_mutex_unlock(&sessionMutex);
    }

    void sendToAll(const std::string& message, size_t length, int flags) {
        for (Player* player : gamePlayers) {
            if (player == nullptr) continue;
            send(player->getSocket(), message.c_str(), length, flags);
        }
        printf("Send : %s [to ALL]\n", message.c_str());

    }

    std::string joinCards(std::vector<Card>& cards) {
        std::string result;
        for (Card& card : cards) {
            result += card.toString() + ",";
        }
        return result;
    }

    // 게임의 전체 로직을 실행하는 함수
    // sleep()은 데이터를 받은 클라이언트가 데이터를 처리하고 UI를 그리는 시간을 주기 위한 안전 장치
    void run() {
        sendToAll("GAME_START\n", 11, 0); // 게임 시작을 클라이언트에 알림
        sleep(2);

        sendToAll(("DISCARD " + topCard.toString()).c_str(), 11, 0); // 가장 위에 있는 카드를 하나 뽑아 시작 카드 제시
        sleep(1);

        // 클라이언트의 플레이어 인덱스를 알려주기 위한 로직(화면 배치용)
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
        printf("Initial hands\n");
        sleep(1);

        passturn(); 

        // 각 턴의 플레이어에게 데이터를 받아 게임을 진행하는 메인 로직
        while (!isGameOver) {
            pthread_mutex_lock(&sessionMutex);
            printf("Game In Progress\n");

            // 뮤텍스 & 조건 변수를 이용한 스레드 간 데이터 처리 부분
            // 큐에 데이터가 들어올 때 signal을 보내고, 대기 중인 스레드가 깨어나서 명령어를 처리함
            while (playingCommands.empty()) {
                pthread_cond_wait(&playingCond, &sessionMutex);
            }

            Command command = playingCommands.front();
            playingCommands.pop();
            pthread_mutex_unlock(&sessionMutex);

            if (command.isDisconnect) {
                printf("before handle\n");
                handleDisconnect(command.player);
                continue;
            }

            // 자기 턴이 아닐 때 데이터가 오면 잘못된 데이터이므로 에러 처리,
            if (command.player != gamePlayers[currentPlayerIndex]) {
                send(command.player->getSocket(), ("Failed_PLAY" + std::to_string(currentPlayerIndex)).c_str(), 20, 0);
                replayTurn();
            } else {
                processCommand(command);
            }
            sleep(1);

            // 게임 종료 조건) 플레이어의 핸드가 비어있으면 게임 종료 데이터를 클라이언트로 전송
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

            // 드로우 덱이 모두 드로우되어 비어있으면 드로우 덱 재충전
            if (drawDeck.isExhausted()) {
                drawDeck.reshuffle(dummyDeck);
            }

            passturn(); // 다음 플레이어로 턴을 넘기는 함수
        }

        // 게임 종료 로직
        for (Player* player : gamePlayers) {
            if (player == nullptr) continue;
            player->goInLobby();
            player->setSessionId(-1);
        }
    }

    int getSessionId() {
        return sessionId;
    }
};
