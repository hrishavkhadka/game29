#ifndef MYBOT_CPP
#define MYBOT_CPP

#include"./mybot.hpp"
#include"./utils.hpp"


Inference::Inference(const PlayPayload& payload)
{
    /*
    fill unknown cards then
    for player to play all probs for unknown card is zero.
    for each hand completed:
        if card played is not of lead suit
            for all lead suit unknown cards
                reduce probablity for the card player to 0
        if last card played is of lead suit and their team doesn't win trick
            for all lead suit cards
                if lead suit card's rank > last card's rank
                    reduce probablity for the last card player by (hand points * some const'20') to min 10 if not 0 already
                if last card has points
                    if lead suit card's rank < last card's rank
                        reduce probablity for the last card player by some const'40' to min 10 if not 0 already
        if trump is known 
            reduce all other suit probablilty to 0
        else if trump is not known
            for cards played in hand
                if card player in not bid winner
                    reduce trump probability for suit by card's points * some const'10'
    */

    //inference for trump {C,D,H,S}(testing successful for first part)
    bool trump_known = true;
    int bid_winner_index;
    if(std::holds_alternative<Suit>(payload.trumpSuit))
    {
        int suit = std::get<Suit>(payload.trumpSuit);

        //0 for other three
        trump[(suit + 1) % 4] = 0;
        trump[(suit + 2) % 4] = 0;
        trump[(suit + 3) % 4] = 0;
        //100 for known trump suit.
        //trump[suit] = 100;
    }
    else
    {
        for(auto bid_entry : payload.bid_history)
        {
            if(bid_entry.bid_value != 0)
            {
                bid_winner_index = get_player_index(payload , bid_entry.player_id);
            }
        }
        trump_known = false;
    }

    //filling unknown cards (test was successful for this part)
    Card card;
    bool card_is_unknown;
    for(Rank rank = Rank::SEVEN ; rank <= Rank::ACE ; rank = Rank(rank + 1))
        for(Suit suit = Suit::CLUBS ; suit <= Suit::SPADES ; suit = Suit(suit + 1))
        {
            card.suit = suit; card.rank = rank;
            card_is_unknown = true;
            for(auto hand : payload.hand_history)
            {
                for(auto c : hand.card)
                {
                    if(card == c)
                        card_is_unknown = false;
                }
            }
            for(auto c : payload.played)
            {
                if(card == c)
                    card_is_unknown = false;
            }
            for(auto c : payload.cards)
            {
                if(card == c)
                    card_is_unknown = false;
            }
            if(card_is_unknown)
            {
                CardDistProb cdp{card};
                unknown_cards.push_back(cdp);
            }
        }

    
    //make all probablity 0 for player to play
    unsigned int player_index = get_player_index(payload);
    for(auto& cdp : unknown_cards)
    {
        cdp.distProb.at(player_index) = 0;
    }

    //trump reveal info
    int trump_revealed_hand = -1;
    if(std::holds_alternative<PlayPayload::RevealedObject>(payload.trumpRevealed))
    {
        auto trump_info = std::get<PlayPayload::RevealedObject>(payload.trumpRevealed);
        trump_revealed_hand = trump_info.hand - 1;//to shift to 0
    }
    //conditions for inference(test successful for this part)
    unsigned int winner_index, initiator_index, last_player_index, hand_value;
    Suit lead_suit;
    bool trump_revealed_already = false;
    for(int hand_count = 0 ; hand_count < payload.hand_history.size() ; hand_count++)
    {
        auto hand = payload.hand_history.at(hand_count);
        //initialization for hand
        if(hand_count > trump_revealed_hand) trump_revealed_already = true;
        initiator_index = get_player_index(payload , hand.initiator);
        winner_index = get_player_index(payload , hand.winner);
        last_player_index = ((initiator_index + 3) % 4);
        lead_suit = hand.card.at(0).suit;
        Card& first_card = hand.card.at(0);
        Card& last_card = hand.card.at(3);


        //condition when lead suit and played card are different
        for(int i = 1; i < 4; i++)
        {
            if(hand.card.at(i).suit != lead_suit)
            {
                for(auto& cdp : unknown_cards)
                {
                    if(cdp.card.suit == lead_suit)
                    {
                        cdp.distProb.at((initiator_index + i) % 4) = 0;
                    }
                }
            }
        }
        //for last card is same as lead but their team didn't win.
        // int trump_revealed_hand = 8;
        // int hand_points = 0;
        // int hand_winner_index = 0;
        // int initator_ind = 0;
        // Card top_crd;
        // Suit trump_sui = Suit::NOT_KNOWN;
        // Suit lead_sui = Suit::NOT_KNOWN;
        // if(std::holds_alternative<PlayPayload::RevealedObject>(payload.trumpRevealed))
        // {
        //     trump_revealed_hand = std::get<PlayPayload::RevealedObject>(payload.trumpRevealed).hand - 1;
        //     trump_sui = std::get<Suit>(payload.trumpSuit);
        // }
        // std::vector<Card> winning_cards;
        // for(int hand_count = 0 ; hand_count < payload.hand_history.size() ; hand_count++)
        // {
        //     auto hand = payload.hand_history.at(hand_count);

        //     winning_cards.clear();
            
        //     lead_sui = hand.card.at(0).suit;
        //     initator_ind = get_player_index(payload, hand.initiator);
        //     hand_points = get_hand_points(hand.card);
        //     if(hand_count < trump_revealed_hand)
        //     {
        //         top_crd = hand.card.at(get_hand_winner_index(hand.card,0,false,Suit::NOT_KNOWN));
        //         for(auto cdp : unknown_cards)
        //         {
        //             if((CardPower(cdp.card.rank) > CardPower(top_crd.rank)) && (cdp.card.suit == lead_sui))
        //                 winning_cards.push_back(cdp.card);
        //         }
        //         hand_winner_index = get_hand_winner_index(hand.card,initator_ind,false,Suit::NOT_KNOWN);
        //         if((hand_winner_index != (initator_ind +3)%4) && (hand_winner_index != (initator_ind +1)%4))
        //         {
                    
        //         }
        //     }
        //     else
        //     {
        //         top_crd = hand.card.at(get_hand_winner_index(hand.card,0,true,trump_sui));
        //         for(auto cdp : unknown_cards)
        //         {
        //             if((top_crd.suit != trump_sui) && (CardPower(cdp.card.rank) > CardPower(top_crd.rank)) && (cdp.card.suit == lead_sui))
        //                 winning_cards.push_back(cdp.card);
        //             if((top_crd.suit == trump_sui) && (cdp.card.suit == trump_sui) && (CardPower(cdp.card.rank) > CardPower(top_crd.rank)))
        //                 winning_cards.push_back(cdp.card);
        //         }
        //         hand_winner_index = get_hand_winner_index(hand.card,initator_ind,true,trump_sui);
        //         if((hand_winner_index != (initator_ind +3)%4) && (hand_winner_index != (initator_ind +1)%4))
        //         {
                    
        //         }
        //     }

        // }
         /*  NOTE : TOO complicated ... maybe later.
        if((winner_index != last_player_index) && (winner_index != ((last_player_index+2)%4)) && (last_card.suit == lead_suit))
        {
            //find hand value and top card in hand
            hand_value = CardValue(hand.card.at(0).rank) + CardValue(hand.card.at(1).rank) + CardValue(hand.card.at(2).rank) + CardValue(hand.card.at(3).rank);
            Card top_card = hand.card.at(0);
            for(int i = 1; i < 4; i++)
            {
                if((hand.card.at(i).suit == lead_suit) && (CardPower(hand.card.at(i)) > CardPower(top_card.rank)))
                    top_card = hand.card.at(i);
            }
            //condition to decrease higher cards and lower cards of same suit's probablity for last player.
            for(auto& cpd : unknown_cards)
            {
                if(cpd.card.suit == last_card.suit)
                {    
                    //decrease for cards greater than top card
                    if((CardPower(cpd.card.rank) > CardPower(top_card.rank)) && (hand_count < trump_revealed_hand))
                    {
                        if(cpd.distProb.at(last_player_index) > (hand_value * 20)) 
                            cpd.distProb.at(last_player_index) -= (hand_value * 20);
                        else if(cpd.distProb.at(last_player_index) > 0) 
                            cpd.distProb.at(last_player_index) = 10;
                    }
                    //decrease for cards lower than last card
                    else if((CardValue(last_card.rank) > 0) && (CardPower(cpd.card.rank) < CardPower(last_card.rank)))
                    {
                        if(cpd.distProb.at(last_player_index) > 40) 
                            cpd.distProb.at(last_player_index) -= 40;
                        else if(cpd.distProb.at(last_player_index) > 0)
                            cpd.distProb.at(last_player_index) = 10;
                    }
                }
            }
        }
        */
        //condition if trump is not known (better results without using in sandbox)
        // if((trump_known == false) && (initiator_index != bid_winner_index))
        // {
        //     for(int i = 0 ; i < 4 ; i++)
        //     {
        //         auto card = hand.card.at(i);
        //         if((initiator_index + i)%4 != bid_winner_index)
        //             trump[card.suit] -= (CardValue(card.rank) * 5);
        //     }
        // }
    }

    //inference remaining for cards in payload.played and payload.cards(for trump only)
    if(trump_known == false)
        for(auto card : payload.cards)
        {
            trump[card.suit] -= (CardValue(card.rank) * 10);
        }

    //sort the element of unknown_cards by their probablity sum
    sortBySumOfProbs();
}

void Inference::display()
{
    std::cout << "\nthe inference for the received payload is :";
    std::cout << "\ntrump{C,D,H,S} : {" << trump.at(0) << "," << trump.at(1) << "," << trump.at(2) << "," << trump.at(3) << "}";
    std::cout << "\nunknown_cards :\n";
    for(auto u : unknown_cards) std::cout << u.card << " : {" << u.distProb.at(0) << "," << u.distProb.at(1) << "," << u.distProb.at(2) << "," << u.distProb.at(3) << "}\n";
}

void Inference::sortBySumOfProbs()
{
    std::sort(unknown_cards.begin() , unknown_cards.end() , [](CardDistProb a, CardDistProb b){
        return (a.distProb[0]+a.distProb[1]+a.distProb[2]+a.distProb[3]) < (b.distProb[0]+b.distProb[1]+b.distProb[2]+b.distProb[3]);
    });
}

Determinization::staticParameters::staticParameters(const PlayPayload& payload)
{
    our_index = get_player_index(payload);
    index_to_play = our_index;
    hand_initiator = (our_index - payload.played.size())%4;
    for(auto bid : payload.bid_history)
        if(bid.bid_value != 0) bid_winner_index = get_player_index(payload , bid.player_id);
    for(auto team : payload.teams)
        if((payload.player_id == team.players[0]) || (payload.player_id == team.players[1]))
        {
            our_bid = team.bid;
            our_points = team.won;
            continue;
        }
        else
        {
            opponent_bid = team.bid;
            opponent_points = team.won;
            continue;
        }

    if(std::holds_alternative<bool>(payload.trumpRevealed)) 
        trump_revealed = false;
    else    
    {
        trump_revealed = true;

        auto reveal_info = std::get<PlayPayload::RevealedObject>(payload.trumpRevealed);
        if(((reveal_info.hand-1) == payload.hand_history.size()) && (get_player_index(payload , reveal_info.player_id) == index_to_play))
        {
            trump_just_revealed = true;
        }
    }

    if(std::holds_alternative<bool>(payload.trumpSuit))
        trump_suit = Suit::NOT_KNOWN;
    else    
        trump_suit = std::get<Suit>(payload.trumpSuit);

    cards_played_this_hand = payload.played;

    for(int i = 0 ; i < 4 ; i++)
    {
        if(i < payload.played.size())
            card_cap.at((hand_initiator + i)%4) = 8 - payload.hand_history.size() - 1;
        else
            card_cap.at((hand_initiator + i)%4) = 8 - payload.hand_history.size();
    }
}

void Determinization::determinizeState(const PlayPayload& payload , const Inference& inference, const staticParameters& static_param)
{
    /*
    assign our_index
    assign index_to_play
    assign current hand_initiator
    assign bid_winner_index
    assign our_points
    assign opponent_points
    assign trump_revealed
    fill our_index cards 
    fill cards_played_this_hand
    designate trump by probablity
        sum all trump suit probablities
        get random no. in range of sum
        check the probablity bracket
    calculate card capacities of all players
        capacities = (8 - hand_history.size())
        for i in range of played_cards.size()
            capacity - 1 for (initiator + i) player
    designate cards to player by probablities
        for each unknown card
            sum all its player probablities
            get random no. in the range of sum
            check the probablity bracket
                if player reached capacity
                    check for next player
    */
    our_index = static_param.our_index;
    index_to_play = static_param.index_to_play;
    hand_initiator = static_param.hand_initiator;
    bid_winner_index =static_param.bid_winner_index;
    our_bid = static_param.our_bid;
    opponent_bid = static_param.opponent_bid;
    our_points = static_param.our_points;
    opponent_points = static_param.opponent_points;
    trump_revealed = static_param.trump_revealed;
    trump_just_revealed = static_param.trump_just_revealed;

    cards_played_this_hand = payload.played;
    cards_with_player_index[our_index] = payload.cards;
    
    if(static_param.trump_suit == Suit::NOT_KNOWN)//CDHS
    {
        unsigned int prob_sum = inference.trump.at(0) + inference.trump.at(1) + inference.trump.at(2) + inference.trump.at(3);
        unsigned int rand_num = get_random_number(0 , prob_sum);
        if(rand_num < inference.trump.at(0))
            trump_suit = Suit(0);
        else if((rand_num < (inference.trump.at(0) + inference.trump.at(1))) 
        && (rand_num >= inference.trump.at(0)))
            trump_suit = Suit(1);
        else if((rand_num < (inference.trump.at(0) + inference.trump.at(1) + inference.trump.at(2))) 
        && (rand_num >= (inference.trump.at(0) + inference.trump.at(1))))
            trump_suit = Suit(2);
        else
            trump_suit = Suit(3);
    }
    else
        trump_suit = static_param.trump_suit;
    

    for(auto cdp : inference.unknown_cards)
    {
        unsigned int prob_sum = cdp.distProb.at(0) + cdp.distProb.at(1) + cdp.distProb.at(2) + cdp.distProb.at(3);
        unsigned int rand_num = get_random_number(0 , prob_sum);
        if(rand_num < cdp.distProb.at(0))
        {
            if(cards_with_player_index.at(0).size() < static_param.card_cap.at(0))
                cards_with_player_index.at(0).push_back(cdp.card);
            else if(cards_with_player_index.at(1).size() < static_param.card_cap.at(1))
                cards_with_player_index.at(1).push_back(cdp.card);
            else if(cards_with_player_index.at(2).size() < static_param.card_cap.at(2))
                cards_with_player_index.at(2).push_back(cdp.card);
            else
                cards_with_player_index.at(3).push_back(cdp.card);
        }
        else if((rand_num < (cdp.distProb.at(0) + cdp.distProb.at(1))) 
        && (rand_num >= cdp.distProb.at(0)))
        {
            if(cards_with_player_index.at(1).size() < static_param.card_cap.at(1))
                cards_with_player_index.at(1).push_back(cdp.card);
            else if(cards_with_player_index.at(2).size() < static_param.card_cap.at(2))
                cards_with_player_index.at(2).push_back(cdp.card);
            else if(cards_with_player_index.at(3).size() < static_param.card_cap.at(3))
                cards_with_player_index.at(3).push_back(cdp.card);
            else
                cards_with_player_index.at(0).push_back(cdp.card);
        }
        else if((rand_num < (cdp.distProb.at(0) + cdp.distProb.at(1) + cdp.distProb.at(2))) 
        && (rand_num >= (cdp.distProb.at(0) + cdp.distProb.at(1))))
        {
            if(cards_with_player_index.at(2).size() < static_param.card_cap.at(2))
                cards_with_player_index.at(2).push_back(cdp.card);
            else if(cards_with_player_index.at(3).size() < static_param.card_cap.at(3))
                cards_with_player_index.at(3).push_back(cdp.card);
            else if(cards_with_player_index.at(0).size() < static_param.card_cap.at(0))
                cards_with_player_index.at(0).push_back(cdp.card);
            else
                cards_with_player_index.at(1).push_back(cdp.card);
        }
        else
        {
            if(cards_with_player_index.at(3).size() < static_param.card_cap.at(3))
                cards_with_player_index.at(3).push_back(cdp.card);
            else if(cards_with_player_index.at(0).size() < static_param.card_cap.at(0))
                cards_with_player_index.at(0).push_back(cdp.card);
            else if(cards_with_player_index.at(1).size() < static_param.card_cap.at(1))
                cards_with_player_index.at(1).push_back(cdp.card);
            else
                cards_with_player_index.at(2).push_back(cdp.card);
            
        }
    }
}

void Determinization::display()
{
    std::cout<<"\nthe current determinization is : ==================================================\n";
    std::cout<<"our_index : "<<our_index<<"\tindex_to_play : "<<index_to_play<<"\thand_initiator : "<<hand_initiator<<"\tbid_winner_index : "<<bid_winner_index
    <<"\nour_bid : "<<our_bid<<"\tour_points : "<<our_points<<"\topponent_bid : "<<opponent_bid<<"\topponent_points : "<<opponent_points
    <<"\ntrump_revealed : "<<trump_revealed<<"\ttrump_suit : "<<trump_suit<<"\ttrump_just_revealed : "<<trump_just_revealed<<"\n";
    std::cout<<"the cards played this hand are :\t{ ";
    for(auto card : cards_played_this_hand)
        std::cout<<card<<" ,";
    std::cout<<" }\nthe cards with players are :";
    for(auto player_cards : cards_with_player_index)
    {
        std::cout<<"\n{ ";
        for(auto card : player_cards)
            std::cout<< card <<", ";
        std::cout<<" }";
    }
    std::cout<<"\nthe legal actions are :\t{ ";
    for(auto move : legal_actions)
    {
        std::cout << move.action << "_" << move.played_card <<", ";
    }
    std::cout<<" }\n====================================================================================\n";
}

void Determinization::fillLegalMoves()
{
/*
    if terminal
        return
    if we initiate.
        (all cards)
    elseif leading suit.
        (play lead suit)
    else no leading suit.
        if trump is revealed.
            if we just revealed it.
                (play winning trump->play any trump->play any)
            elseif we didn't just reveal it
                (play any).
        elseif trump is not revealed.
            if I don't know the trump
                (reveal trump).
                (play anything else).
            elseif I know the trump
                (reveal and play(winning trump->any trump->any card))
                (play any card.)

    //remember to reset trump just revealed after making move.
*/

    legal_actions.clear();
    
    PlayAction act;
    //if we are the hand_initiator
    act.action = PlayAction::PlayCard;
    if(cards_played_this_hand.size() == 0)
    {   
        for(auto card : cards_with_player_index.at(index_to_play))
        {
            act.played_card = card;
            legal_actions.push_back(act);
        }
        return;
    }
    //if not then

    //getting ready for when we have lead suit or not
    Suit lead_suit = cards_played_this_hand.at(0).suit;
    bool have_lead_suit = false;
    act.action = PlayAction::PlayCard;
    for(auto card : cards_with_player_index.at(index_to_play))
    {
        if(card.suit == lead_suit)
        {
            act.played_card = card;
            legal_actions.push_back(act);
            have_lead_suit = true;
        }
    }
    //when no lead suit available
    bool hand_has_trumps = false;
    bool we_have_trumps = false;
    bool we_have_winning_trumps = false;
    Card top_trump;
    std::vector<Card> winning_trumps;
    std::vector<Card> trumps;
    if(have_lead_suit == false)
    {
        //if trump has been revealed
        if(trump_revealed == true)
        {
            //if we just revealed it
            if(trump_just_revealed)
            {
                act.action = PlayAction::PlayCard;
                //make top_trump the lowest trump
                top_trump.rank = Rank::SEVEN;
                top_trump.suit = Suit(trump_suit);
                //to find top_trump and if hand has trumps
                for(int i = 0 ; i<cards_played_this_hand.size() ; i++)
                {
                    auto card = cards_played_this_hand.at(i);
                    if((card.suit == trump_suit))
                    {
                        hand_has_trumps = true;
                        // if card played is not by our friend
                        if((((hand_initiator+i)%4) != ((index_to_play+2)%4)) && (CardPower(card.rank) > CardPower(top_trump.rank)))
                            top_trump = card;
                    }
                }
                for(auto card : cards_with_player_index.at(index_to_play))
                {
                    if(card.suit == trump_suit)
                    {
                        we_have_trumps = true;
                        if(CardPower(card.rank) >= CardPower(top_trump.rank))
                        {
                            we_have_winning_trumps = true;
                            act.played_card = card;
                            legal_actions.push_back(act);
                        }
                    }
                }
                if(we_have_trumps == false)
                {
                    for(auto card : cards_with_player_index.at(index_to_play))
                    {
                        act.played_card = card;
                        legal_actions.push_back(act);
                    }
                }
                else if(we_have_winning_trumps == false)
                {
                    for(auto card : cards_with_player_index.at(index_to_play))
                        if(card.suit == trump_suit) 
                        {
                            act.played_card = card;
                            legal_actions.push_back(act);
                        }
                }

            }
            else
            {
                act.action = PlayAction::PlayCard;
                for(auto card : cards_with_player_index.at(index_to_play))
                {
                    act.played_card = card;
                    legal_actions.push_back(act);
                }
            }
        }
        else
        {
            //if we know trump card but not revealed
            if(index_to_play == bid_winner_index)
            {
                //to play without revealing
                act.action = PlayAction::PlayCard;
                for(auto card : cards_with_player_index.at(index_to_play))
                {
                    act.played_card = card;
                    legal_actions.push_back(act);
                }

                //to reveal and play
                act.action = PlayAction::RevealAndPlay;
                //make top_trump the lowest trump
                top_trump.rank = Rank::SEVEN;
                top_trump.suit = Suit(trump_suit);
                //to find top_trump and if hand has trumps
                for(int i = 0 ; i<cards_played_this_hand.size() ; i++)
                {
                    auto card = cards_played_this_hand.at(i);
                    if((card.suit == trump_suit))
                    {
                        hand_has_trumps = true;
                        // if card played is not by our friend
                        if((((hand_initiator+i)%4) != ((index_to_play+2)%4)) && (CardPower(card.rank) > CardPower(top_trump.rank)))
                            top_trump = card;
                    }
                }
                for(auto card : cards_with_player_index.at(index_to_play))
                {
                    if(card.suit == trump_suit)
                    {
                        we_have_trumps = true;
                        if(CardPower(card.rank) >= CardPower(top_trump.rank))
                        {
                            we_have_winning_trumps = true;
                            act.played_card = card;
                            legal_actions.push_back(act);
                        }
                    }
                }
                if(we_have_trumps == false)
                {
                    for(auto card : cards_with_player_index.at(index_to_play))
                    {
                        act.played_card = card;
                        legal_actions.push_back(act);
                    }
                }
                else if(we_have_winning_trumps == false)
                {
                    for(auto card : cards_with_player_index.at(index_to_play))
                        if(card.suit == trump_suit) 
                        {
                            act.played_card = card;
                            legal_actions.push_back(act);
                        }
                }
            }
            else
            {
                act.action = PlayAction::PlayCard;
                for(auto card : cards_with_player_index.at(index_to_play))
                {
                    act.played_card = card;
                    legal_actions.push_back(act);
                }
                act.action = PlayAction::RevealTrump;
                legal_actions.push_back(act);
            }
        }
    }

#if defined(_DEBUG)
    if(isTerminal())
    {
        std::cerr << "fillActions() : state is terminal.";
        return;
    }
    if(legal_actions.size() == 0)
    {
        std::cerr << "fillActions() : no legal moves found."<<std::endl;
    }
    if(cards_with_player_index.at(index_to_play).size() == 0)
    {
        std::cerr << "fillActions() : no cards with index_to_play to play."<<std::endl;
    }
#endif
    
}

void Determinization::makeMove(const PlayAction& act)
{
#if defined(_DEBUG)
    if(cards_with_player_index.at(index_to_play).size() == 0)
    {
        std::cerr << "makeMove() : no cards with index_to_play to play."<<std::endl;
        return;
    }
#endif
    /*
    if act is playcard only
        push back played card to played
        pop card form player's cards list
        if last card in hand    
            index_to_play = get_winner()
            hand_initiator = index_to_play
            if(index_to_play is us)
                our points += get_hand_points()
            else
                opponent points += get_hand_points()
            clear played list
        else
            index_to_play = next index
        trump_just_revealed = false

    else if act is reveal only
        trump_revealed = true
        trump_just_revealed = true

    else if act is reveal and play
        trump_revealed = true
        push back played card to played
        pop card form player's cards list
        if last card in hand    
            index_to_play = get_winner()
            hand_initiator = index_to_play
            if(index_to_play is us)
                our points += get_hand_points()
            else
                opponent points += get_hand_points()
            clear played list
        else
            index_to_play = next index
    
    clear legal actions list

    */
    if(act.action == PlayAction::PlayCard)
    {
        Card card_played = act.played_card;

        cards_played_this_hand.push_back(card_played);
        removeCard(cards_with_player_index.at(index_to_play) , card_played);
        if(cards_played_this_hand.size() == 4)
        {
            index_to_play = get_hand_winner_index(cards_played_this_hand, hand_initiator, trump_revealed, trump_suit);
            hand_initiator = index_to_play;
            if((index_to_play == our_index) || (index_to_play == ((our_index+2)%4)))
                our_points += get_hand_points(cards_played_this_hand);
            else
                opponent_points += get_hand_points(cards_played_this_hand);
            cards_played_this_hand.clear();
        }
        else
        {
            index_to_play = (index_to_play+1)%4;
        }
        trump_just_revealed = false;
    }
    else if(act.action == PlayAction::RevealTrump)
    {
        trump_revealed = true;
        trump_just_revealed = true;
    }
    else if(act.action == PlayAction::RevealAndPlay)
    {
        trump_revealed = true;

        Card card_played = act.played_card;

        cards_played_this_hand.push_back(card_played);
        removeCard(cards_with_player_index.at(index_to_play) , card_played);
        if(cards_played_this_hand.size() == 4)
        {
            index_to_play = get_hand_winner_index(cards_played_this_hand, hand_initiator, trump_revealed, trump_suit);
            hand_initiator = index_to_play;
            if((index_to_play == our_index) || (index_to_play == ((our_index+2)%4)))
                our_points += get_hand_points(cards_played_this_hand);
            else
                opponent_points += get_hand_points(cards_played_this_hand);
            cards_played_this_hand.clear();
        }
        else
        {
            index_to_play = (index_to_play+1)%4;
        }
    }
    else
        std::cerr << "makeMove() : Invalid Action Encountered";
	
}

bool Determinization::isTerminal()
{
    if((cards_with_player_index.at(0).size() == 0)
        && (cards_with_player_index.at(1).size() == 0)
        && (cards_with_player_index.at(2).size() == 0)
        && (cards_with_player_index.at(3).size() == 0))
        return true;
    else
        return false;
}

//resolve avail count issue....avail_count++ done here
Node* Node::insert_child(const PlayAction& play_action)
{
    Node* child = new (std::nothrow) Node;
    if (!child) // handle case where new returned null
    {
        // Do error handling here
        std::cerr << "Node::insert_child() : Could not allocate memory\n";
    }
    //Node::all_nodes.push_back(child);

    child->parent = this;
    child->incoming_action = play_action;
    child->avail_count++;
    children.push_back(child);

    return child;
}

bool Node::no_children_present()
{
    if(children.size() == 0)
        return true;
    else
        return false;
}

bool Node::all_children_present(const Determinization& determinization)
{
    if(get_absent_child(determinization).action == PlayAction::Action::NoAction)
        return true;
    else
        return false;
}

PlayAction Node::get_absent_child(const Determinization& determinization)
{
    if(determinization.legal_actions.size() == 0)
    {
        std::cerr << "Node::get_absent_child() : no legal action in determinization" << std::endl;
    }
  //  debugPoint("get absent child() : before outer loop");
    bool action_present;
    for(int i = 0; i < determinization.legal_actions.size(); i++)
    {
  //      debugPoint("get absent child() : before inner loop ");
  //      std::cout<< " ,,, children.size() = " << children.size();
        action_present = false;
        for(int j = 0; (j+1) < ((int)(children.size())+1); j++)
        {
   //         std::cout << "\nchildren.size() : " << (int)(children.size())+1 << "\tj : "<<j<<"\n";
   //         debugPoint("get absent child() : inner loop top");
            if(children.at(j)->incoming_action == determinization.legal_actions.at(i))
            {
   //             debugPoint("get absent child() : inside inner loop if");
                action_present = true;
                break;
            }
   //         debugPoint("get absent child() : inner loop bottom");
        }
        if(action_present == false)
        {
    //        debugPoint("get absent child() : action not present");
            return determinization.legal_actions.at(i);
        }
    }
    return PlayAction{PlayAction::Action::NoAction , Card{Rank::SEVEN,Suit::NOT_KNOWN}};
}

float Node::compute_ucb1()
{
    if(avail_count == 0 || visit_count == 0)
    {
        std::cout << "Node::assign_ucb1 : Divide by Zero Error.\n";
        return 0;
    }
    else
    {
        UCB1_score = (
            (total_reward / visit_count) + sqrt(2 * (log((float)avail_count)) / visit_count)
        );
    }
    return UCB1_score;
}

float Node::compute_ucb1_tuned()
{
    /*
     C = √( (logN / n) x min(1/4, V(n)) )
     where V(n) is an upper confidence bound on the variance of the bandit, 
     i.e.
     V(n) = Σ(x_i² / n) - (Σ x_i / n)² + √(2log(N) / n)
    and x_i are the rewards we got from the bandit so far.
    */
    if(avail_count == 0 || visit_count == 0)
    {
        std::cout << "Node::assign_ucb1 : Divide by Zero Error.\n";
        return 0;
    }
    
    float V = E_r2_by_n_ - pow((total_reward / visit_count),2) + sqrt((2*log(avail_count))/visit_count);
    float C;
    if(V < (0.25))
    {
        C = sqrt((log(avail_count)/visit_count) * V);
    }
    else
    {
        C = sqrt((log(avail_count)/visit_count) * 0.25);
    }

    UCB1_score = ((total_reward / visit_count) + C);
    return UCB1_score;
}

Node* Node::return_best_child(const Determinization& determinization)
{
    Node* best_child = nullptr;
    float best_ucb;
    float next_ucb;
    for(int i = 0; i < determinization.legal_actions.size(); i++)
    {
        for(int j = 0; j < children.size(); j++)
        {
            if((determinization.legal_actions.at(i) == children.at(j)->incoming_action))
            {
                next_ucb = children.at(j)->compute_ucb1_tuned();
                if(best_child == nullptr)
                {
                    best_child = children.at(j);
                    best_ucb = best_child->compute_ucb1_tuned();
                }
                else if(next_ucb > best_ucb) 
                {
                    best_child = children.at(j);
                    best_ucb = next_ucb;
                }
            }
        }
    }
    return best_child;
}

float Determinization::result()
{
    if(isTerminal())
    {
        if(trump_revealed == true)
        {            
            if (((our_bid == 0) && (opponent_points < opponent_bid)) || ((opponent_bid == 0) && (our_points >= our_bid)))
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
            return 0.5;
    }
    else
    {
        std::cerr << "Determinization::result : Terminal determinization not reached yet\n";
    }
    return 0;
}

void Node::increase_avail_count_of_compatible_children(const Determinization& determinization)
{
    for(int i = 0; i < determinization.legal_actions.size(); i++)
    {
        for(int j = 0; j < (int)children.size(); j++)
        {
            if(determinization.legal_actions.at(i) == children.at(j)->incoming_action)
            {
                children.at(j)->avail_count++;
            }
        }
    }
}

Node* Node::return_child_with_most_visits()
{
    Node* child_with_most_visits = children.at(0);
    for(int i = 1 ; i < children.size() ; i++)
    {
        if(children.at(i)->visit_count > child_with_most_visits->visit_count)
        {
            child_with_most_visits = children.at(i);
        }
    }
    return child_with_most_visits;
}

void Determinization::makeRandomMove()
{
    if(legal_actions.size() == 0)
    {
        std::cerr << "Determinization::makeRandomMove : no moves to make\n";
        return;
    }
    int rand_num = get_random_number(0 , legal_actions.size());
    makeMove(legal_actions.at(rand_num));
}

PlayAction ismcts(const PlayPayload& payload, timeDataType starting_time)
{   

    timeDataType finishing_time = get_finishing_time(payload, starting_time);
    Inference inference{payload};
	Determinization::staticParameters static_param{payload};

    Node root_node;
    Node* current_node_ptr;
    int reward;
    while(finishing_time > std::chrono::high_resolution_clock::now())
    {   
        Determinization determinization{payload , inference , static_param};
        /*
        Node* current_node_ptr == &root_node;
        current_node_ptr = Node* ismctsSelect(Node* current_node_ptr, Determinization& determinization)
        {
            do
            {
                if(!det.isTerminal())
                {
                    det.fillLegalMoves();
                    if(det.all_children_present())
                    {   
                        //increase avail_count of all children
                        current_node_ptr = return_best_child();
                        det.makeMove(current_node_ptr->incoming_action);
                    }
                    else
                    {
                        //increase avail_count of all children
                        current_node_ptr = current_node_ptr->insert_child(get_absent_child());
                        det.makeMove(current_node_ptr->incoming_action);
                        break;
                    }
                }
                else
                {
                    break;
                }
            }while(1);
            return current_node_ptr;
        }
        int reward = ismctsSimulate(Determinization& det)
        {
            if(!det.isTerminal())
            {
                det.fillLegalMoves();
                //det.playRandomMove();
            }
            return det.result()//0 for loss and 1 for win 
        }
        ismctsBackpropagate(Node* current_node , int reward)
        {
            while(current_node != &root_node)
            {
                current_node->visit_count++;
                current_node->total_reward += reward;
                current_node->compute_ucb1();
                current_node = current_node->parent;
            }
        }
        */
        current_node_ptr = &root_node;
        current_node_ptr = ismctsSelect(current_node_ptr, determinization);

        //
        if(determinization.isTerminal() && current_node_ptr->visit_count > 0)
        {
            reward = 0;
            ismctsBackpropagate(current_node_ptr , root_node , reward);
            continue;
        }
        //

        reward = ismctsSimulate(determinization);
        ismctsBackpropagate(current_node_ptr , root_node , reward);
    }
    
    return root_node.return_child_with_most_visits()->incoming_action;
}
Node* ismctsSelect(Node* current_node_ptr, Determinization& det)
{
    while(!det.isTerminal())
    {
        det.fillLegalMoves();
        if(current_node_ptr->all_children_present(det))
        {   
            current_node_ptr->increase_avail_count_of_compatible_children(det);
            current_node_ptr = current_node_ptr->return_best_child(det);
            det.makeMove(current_node_ptr->incoming_action);
        }
        else
        {
            current_node_ptr->increase_avail_count_of_compatible_children(det);
            current_node_ptr = current_node_ptr->insert_child(current_node_ptr->get_absent_child(det));
            det.makeMove(current_node_ptr->incoming_action);
            break;
        }
    
    }
    return current_node_ptr;
}
int ismctsSimulate(Determinization& det)
{
    while(!det.isTerminal())
    {
        det.fillLegalMoves();
        det.makeRandomMove();
    }
	
    return det.result();//0 for loss and 1 for win 
}
void ismctsBackpropagate(Node* current_node, Node& root_node , int reward)
{
    while(current_node != &root_node)
    {
        current_node->visit_count++;
        current_node->total_reward += reward;
        current_node->E_r2_by_n_ += (reward * reward) / current_node->visit_count;
        //current_node->compute_ucb1();

        current_node = current_node->parent;
    }
}

#endif