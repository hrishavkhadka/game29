#include "./bot.hpp"
#include "./utils.hpp"
#include <algorithm>

#ifdef RANGES_SUPPORT
#undef RANGES_SUPPORT
#endif

#if _HAS_CXX20
#define RANGES_SUPPORT 1
#include <ranges>
#endif

// Internal
std::ostream &operator<<(std::ostream &os, Card const &card)
{
    return os << Card::ToStr(card);
}

std::ostream &operator<<(std::ostream &os, PlayPayload const &payload)
{
    os << "Player ID :\t" << payload.player_id << "\n";
    os << "Player IDs :\t";
    for (auto const &player : payload.player_ids)
        os << player << "\t";

    os << "\nCards :\t";
    for (auto card : payload.cards)
        os << card << "\t";

    os << "\nPlayed :\t";
    for (auto card : payload.played)
        os << card << "\t";

    os << "\nBid History :\n";
    for (auto const &entry : payload.bid_history)
    {
        os << "player    : " << entry.player_id << "\n"
           << "Bid value : " << entry.bid_value << std::endl;
    }

    // Hand history
    for (auto const &x : payload.hand_history)
    {
        os << "Initiator : " << x.initiator;
        os << "\nWinner   : " << x.winner;
        os << "\nCards Played : ";
        for (auto card : x.card)
            os << card << "   ";
        os << std::endl;
    }
    return os;
}

static GameState game_instance;

GameState& GetGameInstance()
{
    return game_instance;
}

void InitGameInstance()
{
    // Initialization code of game state goes here, if any
}

//
// Actual gameplay goes here
//
// These three functions are called in response to the http request calls /chooseTrump, /bid and /play respectively. 
// Logic of your code should go inside respective functions
//

Suit GameState::ChooseTrump(PlayerID myid, std::vector<PlayerID> player_ids, std::vector<Card> mycards,
                            int32_t time_remaining, std::vector<BidEntry> bid_history)
{
    Suit suit_to_return = Suit::CLUBS;
    int bid_to_return = 0;
    int bid_for_this_suit = 0;
    for(Suit suit = Suit::CLUBS ; suit <= Suit::SPADES ; suit = Suit(suit + 1))
    {
        bid_for_this_suit = 0;
        for(Card card : mycards)
        {
            if(card.suit == suit)
            {
                switch(card.rank)
                {
                    case Rank::JACK:
                        bid_for_this_suit += 10000;
                        break;
                    case Rank::NINE:
                        bid_for_this_suit += 1000;
                        break;
                    case Rank::ACE:
                        bid_for_this_suit += 100;
                        break;
                    case Rank::TEN:
                        bid_for_this_suit += 10;
                        break;
                    default:
                        bid_for_this_suit += 1;
                        break;
                } 
            }
        }
        if(bid_map[bid_for_this_suit] > bid_to_return)
        {
            bid_to_return = bid_map[bid_for_this_suit];
            suit_to_return = suit;
        }
    }
    return Suit(suit_to_return);
}

int GameState::Bid(PlayerID myid, std::vector<PlayerID> player_ids, std::vector<Card> mycards, int32_t time_remaining,
                   std::vector<BidEntry> bid_history, BidState const &bid_state)
{
    // Either bid or pass
    // if (bid_history.empty())
    //     return 16;
    int bid_to_return = 0;
    int bid_for_this_suit = 0;
    for(Suit suit = Suit::CLUBS ; suit <= Suit::SPADES ; suit = Suit(suit + 1))
    {
        bid_for_this_suit = 0;
        for(Card card : mycards)
        {
            if(card.suit == suit)
            {
                switch(card.rank)
                {
                    case Rank::JACK:
                        bid_for_this_suit += 10000;
                        break;
                    case Rank::NINE:
                        bid_for_this_suit += 1000;
                        break;
                    case Rank::ACE:
                        bid_for_this_suit += 100;
                        break;
                    case Rank::TEN:
                        bid_for_this_suit += 10;
                        break;
                    default:
                        bid_for_this_suit += 1;
                        break;
                } 
            }
        }
        if(bid_map[bid_for_this_suit] > bid_to_return)
            bid_to_return = bid_map[bid_for_this_suit];
    }
    //if we are first to bid
    if(bid_history.size() == 0)
    {
        if(bid_to_return > 0)
            return 16;
        else
            return 0;
    }

    //if previous 3 passed their bid then we must bid 16
    bool must_bid_16 = true;
    for(auto bid : bid_history)
    {
        if(bid.bid_value != 0)
        {
            must_bid_16 = false;
            break;
        }
    }
    if(must_bid_16)
    {
        return 16;
    }
    
    //if we are the challenger send +1;
    if((bid_state.challenger.player_id == myid) && (bid_to_return > bid_state.defender.bid_value))
    {
        return (bid_state.defender.bid_value + 1);
    }
    //if we are the defender match
    else if((bid_state.defender.player_id == myid) && (bid_to_return >= bid_state.challenger.bid_value))
    {
        return bid_to_return;
    }

    return 0;
}

PlayAction GameState::Play(PlayPayload payload)
{
    //std::cout << "Payload Received and parsed: \n" << payload << std::endl;

    if (std::holds_alternative<PlayPayload::RevealedObject>(payload.trumpRevealed)) // or just dump variants and use tagged unions
    {
        // Trump revealed code goes here
    }

    PlayAction p_action;
    p_action.action = PlayAction::PlayCard;

    if (payload.played.empty())
    {
        // Cards in player hands
        p_action.played_card = payload.cards[0];
        return p_action;
    }

    Suit lead_suit        = payload.played[0].suit;

    auto same_suit_filter = [=](Card card) { return card.suit == lead_suit; };
#if defined(RANGES_SUPPORT)
    auto same_suit_cards = std::views::filter(payload.cards, same_suit_filter);
#else
    std::vector<Card> same_suit_cards;
    std::copy_if(payload.cards.begin(), payload.cards.end(), std::back_inserter(same_suit_cards), same_suit_filter);
#endif
    if (same_suit_cards.empty())
    {
        p_action.played_card = payload.cards[0];
        return p_action;
    }

    // Play card of same suit if available
    p_action.played_card = *same_suit_cards.begin();
    return p_action;
    // This isn't complete implementation for total gameplay
}

