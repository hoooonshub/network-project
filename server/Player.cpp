#pragma once

#include "../Common.h"

enum class State {
    LOBBY,
    QUEUE,
    GAME
};

class Player {
private:
    SOCKET socket;
    State state;
    int currentGameSessionId = -1;

public:
    Player(SOCKET sock) : socket(sock), state(State::LOBBY) {}

    SOCKET getSocket() {
        return socket;
    }

    void goInQueue() {
        state = State::QUEUE;
    }

    void goInLobby() {
        state = State::LOBBY;
    }

    void goInGame() {
        state = State::GAME;
    }

    void setSessionId(int sessionId) {
        currentGameSessionId = sessionId;
    }

    int getSessionId() {
        return currentGameSessionId;
    }

    bool isInLobby() const {
        return state == State::LOBBY;
    }

    bool isInQueue() const {
        return state == State::QUEUE;
    }
    
    bool isInGame() const {
        return state == State::GAME;
    }
};