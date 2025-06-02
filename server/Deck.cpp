#include "../Common.h"
#include "Card.cpp"
#include <algorithm> 
#include <random>

class Deck {
private:
    std::vector<Card> cards;
    
    void shuffle() {
        static std::random_device rd;
        static std::mt19937 g(rd());
        std::shuffle(cards.begin(), cards.end(), g);
    }

    Deck() {
        cards.reserve(52);

        for (int suit = static_cast<int>(Suit::HEARTS);
            suit <= static_cast<int>(Suit::SPADES); suit++) {

            for (int rank = static_cast<int>(Rank::TWO);
                rank <= static_cast<int>(Rank::ACE); rank++) {
                    cards.emplace_back(
                        static_cast<Suit>(suit),
                        static_cast<Rank>(rank)
                    );
            }
        }

        shuffle();
    }

public:
    static Deck newOne() {
        return Deck();
    }

    static Deck dummy() {
        Deck dummyDeck;
        dummyDeck.cards.clear();
        return dummyDeck;
    }

    Card draw() {
        Card card = cards.back();
        cards.pop_back();
        return card;
    }

    bool isExhausted() const {
        return cards.empty();
    }

    void reshuffle(Deck& discardedDeck) {
        cards.insert(cards.end(), discardedDeck.cards.begin(), discardedDeck.cards.end());
        discardedDeck.cards.clear();
        shuffle();
    }

    void add(Card card) {
        cards.push_back(card);
    }
};