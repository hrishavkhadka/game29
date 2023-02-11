#include "./utils.hpp"


std::map<int,int> bid_map= {
    {11110,20},
    {11100,19},
    {11101,19},
    {11010,19},
    {11011,19},
    {11000,17},
    {11001,18},
    {11002,18},
    {10110,17},
    {10111,18},
    {10100,16},
    {10101,16},
    {10102,17},
    {10010,16},
    {10011,16},
    {10012,17},
    {10000,16},
    {10001,16},
    {10002,16},
    {10003,17},
    {1110,18},
    {1111,18},
    {1100,16},
    {1101,17},
    {1102,17},
    {1010,16},
    {1011,16},
    {1012,17},
    {1000,0},
    {1001,0},
    {1002,0},
    {1003,16}
};

int get_player_index(const PlayPayload& payload,const PlayerID& player_id)
{
    for(int i = 0 ; i < 4 ; i++)
        if(player_id == payload.player_ids.at(i))
            return i;
    std::cerr << "player_id not in player_ids" << std::endl;
}
int get_player_index(const PlayPayload& payload)
{
    for(int i = 0 ; i < 4 ; i++)
        if(payload.player_id == payload.player_ids.at(i))
            return i;
    std::cerr << "player_id not in player_ids" << std::endl;
}

int get_random_number(int lower,int higher)
{
    return lower + (std::rand() % (higher - lower));
}

// int CardPower(const Card& card, Suit trump_suit = Suit::CLUBS, bool trump_revealed = false)
// {
//     if(trump_revealed)
//     {
//         if(card.suit == trump_suit)
//         {
//             return CardPower(card.rank) + 7;
//         }
//         else
//         {
//             return CardPower(card.rank);
//         }
//     }
//     else
//     {
//         return CardPower(card.rank);
//     }
// }

unsigned int get_hand_winner_index(const std::vector<Card>& played , unsigned int initiator_index , bool trump_revealed , Suit trump_suit)
{
    /*
    lead_suit
    winning card = lead card
    if not trump_revealed
        for cards in played with i 
            if card's suit is lead suit and rank is higher than winning card
                winning card = card
                winning index = initiator + i
    if trump_revealed
        for cards in played with i
            if card is trump suit
                if winning card is not trump suit
                    winning card = card
                    winning index = initiator + i
                if winning card is trump suit but lower than rank
                    change winning ...
            if card is not trump suit and winning card is not trump suit and lower in rank
                change winning...
    */
    Suit lead_suit = played.at(0).suit;
    Card top_card = played.at(0);
    Card card; 
    unsigned int winner_index = initiator_index;
    if(trump_revealed == false)
    {
        for(int i = 1 ; i<4 ; i++)
        {
            card = played.at(i);
            if((card.suit == lead_suit) && (CardPower(card.rank) > CardPower(top_card.rank)))
            {
                top_card = card;
                winner_index = (initiator_index+i)%4;
            }
        }
    }
    else
    {
        for(int i = 1 ; i<4 ; i++)
        {
            card = played.at(i);
            if(card.suit == trump_suit)
            {
                if(top_card.suit != trump_suit)
                {
                    top_card = card;
                    winner_index = (initiator_index+i)%4;
                }
                else if((top_card.suit == trump_suit) && (CardPower(card.rank) > CardPower(top_card.rank)))
                {
                    top_card = card;
                    winner_index = (initiator_index+i)%4;
                }
            }
            else if((top_card.suit != trump_suit) && (card.suit == lead_suit) && (CardPower(top_card.rank) < CardPower(card.rank)))
            {
                top_card = card;
                winner_index = (initiator_index+i)%4;
            }
        }
    }
    return winner_index;
}

unsigned int get_hand_points(const std::vector<Card>& played)
{
    unsigned int hand_points = 0;
    for(auto card : played)
    {
        hand_points += CardValue(card.rank);
    }
    return hand_points;
}

unsigned int removeCard(std::vector<Card>& card_list , Card card)
{
    unsigned int i;
    for(i = 0 ; i < card_list.size() ; i++)
    {
        if(card_list.at(i) == card)
        {
            card_list.erase(card_list.begin() + i);
            break;
        }
    }
    return i;
}

void debugPoint(std::string message)
{
    std::cout << "\n========================";
    std::cout << " debug message : " << message;
    std::cout << " ========================\n";
}

void debugPoint()
{
    unsigned short i;
    return;
}

// inline timeDataType get_present_time()
// {
//     return std::chrono::high_resolution_clock::now();
// }

// inline timeDataType get_finishing_time(int32_t time_remaining)
// {
//     return std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(time_remaining);    
// }

//change this.....................................................
timeDataType get_finishing_time(const PlayPayload& payload, const timeDataType& starting_time)
{
    //for now 1400
    int time_to_take=0;
    int32_t time_remaining;
    if(payload.remaining_time < 300)
        time_remaining = (payload.remaining_time * 2)/3;
    else
        time_remaining  = payload.remaining_time - 100;
    switch(payload.hand_history.size())
    {
        case 0:
            time_to_take = (time_remaining / 5);//350
            break;
        case 1:
            time_to_take = (time_remaining / 5);//210
            break;
        case 2:
            time_to_take = (time_remaining / 4);//168
            break;
        case 3:
            time_to_take = (time_remaining / 4);//168
            break;
        case 4:
            time_to_take = (time_remaining / 3);//126
            break;
        case 5:
            time_to_take = (time_remaining / 3);//126
            break;
        case 6:
            time_to_take = (payload.remaining_time/ 3);//126
            break;
        case 7:
            time_to_take = (payload.remaining_time/3);//126
            break;
        default:
            time_to_take = (payload.remaining_time / 2);
            break;
    }
    if (time_to_take < 7 )
        time_to_take = 7;
    return starting_time + std::chrono::milliseconds(time_to_take);
}

int32_t get_bid(unsigned int key)
{
    return bid_map[key];
}
