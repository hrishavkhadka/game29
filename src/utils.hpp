#ifndef UTILS_CPP
#define UTILS_CPP

#include<map>

#include"./mybot.hpp"
#include"./bot.hpp"

extern std::map<int,int> bid_map;


int get_player_index(const PlayPayload&);
int get_player_index(const PlayPayload& ,const PlayerID&);
int get_random_number(int lower =0,int higher = 100);
unsigned int get_hand_winner_index(const std::vector<Card>& played , unsigned int initiator_index , bool trump_revealed , Suit trump_suit);
unsigned int get_hand_points(const std::vector<Card>& played);
unsigned int removeCard(std::vector<Card>& card_list , Card card);
void debugPoint(std::string);
void debugPoint();
// inline timeDataType get_present_time();
// inline timeDataType get_finishing_time(int32_t);
timeDataType get_finishing_time(const PlayPayload& payload,const timeDataType& starting_time);
int32_t get_bid(unsigned int key);

#endif