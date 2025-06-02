#pragma once

#include "../Common.h"

enum class Suit {
    HEARTS,
    DIAMONDS,
    CLUBS,
    SPADES
};

enum class Rank {
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    ACE
};

class Card {
    Suit suit;
    Rank rank;

public:
    Card(Suit s, Rank r) : suit(s), rank(r) {}

    static Card fromString(std::string cardStr) {
        if (cardStr.length() >= 3) {
            throw std::invalid_argument("Invalid card string");
        }

        Suit s;
        Rank r;

        switch (cardStr[0]) {
            case 'H': s = Suit::HEARTS; break;
            case 'D': s = Suit::DIAMONDS; break;
            case 'C': s = Suit::CLUBS; break;
            case 'S': s = Suit::SPADES; break;
            default: throw std::invalid_argument("Invalid suit");
        }

        switch (cardStr[1]) {
            case '2': r = Rank::TWO; break;
            case '3': r = Rank::THREE; break;
            case '4': r = Rank::FOUR; break;
            case '5': r = Rank::FIVE; break;
            case '6': r = Rank::SIX; break;
            case '7': r = Rank::SEVEN; break;
            case '8': r = Rank::EIGHT; break;
            case '9': r = Rank::NINE; break;
            case 'T': r = Rank::TEN; break;
            case 'J': r = Rank::JACK; break;
            case 'Q': r = Rank::QUEEN; break;
            case 'K': r = Rank::KING; break;
            case 'A': r = Rank::ACE; break;
            default: throw std::invalid_argument("Invalid rank");
        }

        return Card(s, r);
    }

    std::string toString() {
        std::string suitStr;
        switch (suit) {
            case Suit::HEARTS: suitStr = "H"; break;
            case Suit::DIAMONDS: suitStr = "D"; break;
            case Suit::CLUBS: suitStr = "C"; break;
            case Suit::SPADES: suitStr = "S"; break;
        }

        std::string rankStr;
        switch (rank) {
            case Rank::TWO: rankStr = "2"; break;
            case Rank::THREE: rankStr = "3"; break;
            case Rank::FOUR: rankStr = "4"; break;
            case Rank::FIVE: rankStr = "5"; break;
            case Rank::SIX: rankStr = "6"; break;
            case Rank::SEVEN: rankStr = "7"; break;
            case Rank::EIGHT: rankStr = "8"; break;
            case Rank::NINE: rankStr = "9"; break;
            case Rank::TEN: rankStr = "T"; break;
            case Rank::JACK: rankStr = "J"; break;
            case Rank::QUEEN: rankStr = "Q"; break;
            case Rank::KING: rankStr = "K"; break;
            case Rank::ACE: rankStr = "A"; break;
        }

        return suitStr + rankStr;
    }

    bool operator==(const Card& other) const {
        return (suit == other.suit) && (rank == other.rank);
    }

    bool operator!=(const Card& other) const {
        return !(*this == other);
    }

    bool canDiscard(Card& card) {
        return (suit == card.suit) || (rank == card.rank);
    }
};