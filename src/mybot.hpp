#ifndef MYBOT_HPP
#define MYBOT_HPP

#include<array>
#include<chrono>
#include<ctime>
#include<algorithm>
#include<cmath>

#include"bot.hpp"

using timeDataType = std::chrono::high_resolution_clock::time_point;


struct CardDistProb{
        Card card;
        std::array<unsigned int,4> distProb = {100,100,100,100};
        CardDistProb(Card c){card = c;} 
    };
class Inference{
public:
    std::vector<CardDistProb> unknown_cards;
    std::array<unsigned int,4> trump = {100,100,100,100};
    Inference() = default;
    Inference(const PlayPayload&);
    void sortBySumOfProbs();
    void display();
};

class Determinization
{
public:
    struct staticParameters
    {
        short our_index;
        short our_bid;
        short opponent_bid;
        short index_to_play;
        short hand_initiator;
        short bid_winner_index;
        short our_points;
        short opponent_points;
        std::vector<Card> cards_played_this_hand;
        bool trump_revealed = false;
        bool trump_just_revealed = false;
        Suit trump_suit = Suit::NOT_KNOWN;
        std::array<int,4> card_cap;

        staticParameters() = default;
        staticParameters(const PlayPayload& payload);
    };
    short our_index;
    short index_to_play;
    short hand_initiator;
    short bid_winner_index;
    short our_bid;
    short opponent_bid;
    short our_points;
    short opponent_points;
    std::array<std::vector<Card>,4> cards_with_player_index;
    std::vector<Card> cards_played_this_hand;
    std::vector<PlayAction> legal_actions;
    bool trump_revealed = false;
    bool trump_just_revealed = false;
    Suit trump_suit = Suit::NOT_KNOWN;

    Determinization() = default;
    Determinization(const PlayPayload& payload, const Inference& inference, const staticParameters& param){determinizeState(payload,inference,param);}
    //void determinizeStaticParameters();
    void determinizeState(const PlayPayload&, const Inference&, const staticParameters&);
    void fillLegalMoves();
    void makeMove(const PlayAction&);
    void makeRandomMove();
    bool isTerminal();
    void display();
    int result();
    
};

class Node
{
public:
    Node* parent;
    PlayAction incoming_action;
    std::vector<Node*> children;
    unsigned int avail_count = 0;
    unsigned int visit_count = 0;
    float total_reward = 0;
    float UCB1_score = 0;

    Node* insert_child(const PlayAction&);
    Node* return_best_child(const Determinization&);
    bool no_children_present();
    bool all_children_present(const Determinization&);
    void compute_ucb1();
    void increase_avail_count_of_compatible_children(const Determinization&);
    PlayAction get_absent_child(const Determinization&);
    Node* return_child_with_most_visits();

    // static std::vector<Node*> all_nodes;

    // static void deleteAllNodes();
};


class InformationSet
{
public:
    PlayPayload& payload;
    Inference inference;
    Node root_node;
    Determinization::staticParameters static_param;
    Determinization current_determinization;
    std::vector<Determinization> saved_determinizations;
};


PlayAction ismcts(const PlayPayload& payload, timeDataType starting_time);

Node* ismctsSelect(Node* current_node_ptr, Determinization& det);

int ismctsSimulate(Determinization& det);

void ismctsBackpropagate(Node* current_node, Node& root_node , int reward);



#endif
