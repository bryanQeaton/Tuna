#ifndef CHESSBOT_ROOT_H
#define CHESSBOT_ROOT_H
#include <random>
#include "params.h"
#include "search.h"
#include "libchess/position.hpp"

struct root_return {
    std::string best_move;
    int score;
    int depth;
    uint64_t nodes;
    std::vector<libchess::Move> pv;
};
inline root_return root(libchess::Position pos,const int time_limit,const int depth_max=MAX_DEPTH,const bool verbose=false) {
    if (pos.is_terminal()){return {"0000",-BOUND,0,0ull,{libchess::Move()}};}
    const auto t0=std::chrono::high_resolution_clock::now();
    int stand_pat=eval(pos);
    std::vector<libchess::Move> legal_moves=pos.legal_moves();
    std::vector scores(legal_moves.size(),stand_pat);
    std::vector<size_t> indices(legal_moves.size());
    nodeData best={-BOUND,0,{}};
    nodeData last;
    libchess::Move best_move=legal_moves[0];
    libchess::Move last_move=best_move;
    uint64_t nodes=0;
    std::vector<libchess::Move> pv;
    for (int d=0;d<=depth_max;d++) {
        best={-BOUND,0,0,{legal_moves[0]}};
        last.depth=d;
        bool _=false;
        if (verbose) {
            std::cout<<"=============best:"<<last_move<<"|nodes:"<<nodes<<"\npv:";
            for (auto &n:pv) {std::cout<<n<<" ";}
            std::cout<<"\n";
        }
        int alph=-BOUND;
        int beta=BOUND;
        for (size_t m=0;m<legal_moves.size();m++) {
            indices[m]=m;
            pos.makemove(legal_moves[m]);
            std::vector move_history={legal_moves[m]};
            nodeData child;
            if (m==0) {
                child=negamax(pos,-beta,-alph,d,0,move_history,false);
            }
            else {
                child=negamax(pos,-(alph+1),-alph,d,0,move_history,false);
                if (-child.value>alph&&-child.value<beta) {
                    child=negamax(pos,-beta,-alph,d,0,move_history,false);
                }
            }
            if (-child.value>alph){alph=-child.value;}
            nodes+=child.nodes;
            pos.undomove();
            const auto t1=std::chrono::high_resolution_clock::now();
            if (child.value==ABORT_SCORE||std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count()>=time_limit) {
                _=true;
                break;
            }
            if (-child.value>best.value) {
                best=child;
                best.value=-child.value;
                best_move=legal_moves[m];
                if (d==0) {
                    last=best;
                    last_move=best_move;
                }
                pv=child.pv;
            }
            scores[m]=-child.value;
            if (scores[m]>BOUND-MAX_DEPTH) {
                _=true;
                break;
            }
            if (verbose){std::cout<<"depth:"<<d<<"|move:"<<legal_moves[m]<<"|value:"<<-child.value<<"|nodes:"<<child.nodes<<"\n";}
        }
        if (_){break;}
        std::ranges::sort(indices,[&](const size_t a,const size_t b){return scores[a]>scores[b];});
        std::vector<libchess::Move> sorted_moves;
        std::vector<int> sorted_scores;
        sorted_moves.reserve(indices.size());
        sorted_scores.reserve(indices.size());
        for (size_t i:indices) {
            sorted_moves.push_back(legal_moves[i]);
            sorted_scores.push_back(scores[i]);
        }
        legal_moves=std::move(sorted_moves);
        scores=std::move(sorted_scores);
        last=best;
        last_move=best_move;
    }

    //always return the last best move rather than the best move from an incomplete search
    //ensure castling is handled in uci format
    const libchess::Move move=last_move;
    const int value=last.value;
    if (move.type()==libchess::ksc&&pos.turn()==libchess::White){return {"e1g1",value,last.depth,nodes,last.pv};}
    if (move.type()==libchess::qsc&&pos.turn()==libchess::White){return {"e1c1",value,last.depth,nodes,last.pv};}
    if (move.type()==libchess::ksc&&pos.turn()==libchess::Black){return {"e8g8",value,last.depth,nodes,last.pv};}
    if (move.type()==libchess::qsc&&pos.turn()==libchess::Black){return {"e8c8",value,last.depth,nodes,last.pv};}
    return {move,value,last.depth,nodes,last.pv};
}

#endif //CHESSBOT_ROOT_H