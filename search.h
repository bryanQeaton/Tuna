#ifndef CHESSBOT_SEARCH_H
#define CHESSBOT_SEARCH_H
#include <atomic>
#include <algorithm>
#include <complex>

#include "evaluation.h"
#include "params.h"
#include "tt.h"
#include "libchess/position.hpp"
//search termination var
inline std::atomic search=0; //0=default,1=search,-1=stop search
//move ordering debug info
inline uint64_t mo_nodes=0;
inline float beta_cutoff_ratio=0.f;
inline float beta_cutoff_count=0.f;
//tables and data
inline libchess::Move killer_moves[3][MAX_DEPTH];
inline libchess::Move countermove_history[6][64];
inline int history[64][64]={};
inline TT tt(HASH_SIZE);
//static exchange evaluator
inline int sqr_idx(const libchess::Square &sqr) {return sqr.rank()*8+sqr.file();}
inline int see_internal(libchess::Position &pos, const libchess::Square &sqr) {
    int value=0;
    const auto legal_captures=pos.legal_captures();
    libchess::Move smallest_attacker;
    int smallest=7;
    for (auto move:legal_captures) {
        if (move.piece()<smallest&&move.to()==sqr) {
            smallest=move.piece();
            smallest_attacker=move;
        }
    }
    if (smallest_attacker!=libchess::Move()) {
        pos.makemove(smallest_attacker);
        value=std::max(0,static_cast<int>(mat[smallest_attacker.captured()])-see_internal(pos,sqr));
        pos.undomove();
    }
    return value;
}
inline int see(libchess::Position &pos,const libchess::Square &sqr,const libchess::Move &move) {
    int value=0;
    pos.makemove(move);
    value=static_cast<int>(mat[move.captured()])-see_internal(pos,sqr);
    pos.undomove();
    return value;
}
//return struct
struct nodeData {
    int value=-BOUND;
    uint64_t nodes=0ull;
    int depth=0;
    std::vector<libchess::Move> pv{};
};

inline bool use_nn=false;
inline nodeData quiescence(libchess::Position &pos,int alpha,int beta, const int depth_to_root,std::vector<libchess::Move> &move_history) {
    mo_nodes++;
    if (pos.threefold()||pos.fiftymoves()){return {DRAW_SCORE+depth_to_root,1ull,depth_to_root,move_history};} //return because draw
    const int stand_pat=eval(pos);
    if (depth_to_root>=MAX_DEPTH){return {stand_pat,1ull,depth_to_root,move_history};} //return eval
    if (search.load(std::memory_order_relaxed)==-1) return {ABORT_SCORE,1ull,depth_to_root,move_history};
    //tt
    const bool is_pv=(beta-alpha)>1;
    const int alph_orig=alpha;
    libchess::Move best_move={};
    const uint64_t hash=pos.hash();
    ENTRY entry;
    entry=tt[hash];
    if (entry.hash==hash) {
        best_move=entry.best_move;
    }
    //mate distance pruning
    alpha=std::max(-BOUND+depth_to_root, alpha);
    beta=std::min(BOUND-depth_to_root-1,beta);
    if (alpha>=beta){return {alpha,1ull,depth_to_root,move_history};}
    //standing pat
    int value=-BOUND;
    std::vector<libchess::Move>legal_moves;
    if (!pos.in_check()) {
        value=stand_pat;
        if (value>alpha) {
            if (value>=beta) {
                return {value,1ull,depth_to_root,move_history};
            }
            alpha=value;
        }
        legal_moves=pos.legal_captures();
    }
    else {
        legal_moves=pos.legal_moves();
        if (pos.in_check()&&legal_moves.empty()){return {-(BOUND-depth_to_root),1ull,depth_to_root,move_history};}
    }
    nodeData child;
    uint64_t nodes=0ull;
    std::vector<libchess::Move> pv;
    std::vector<int> scores(legal_moves.size());
    for (int m=0;m<static_cast<int>(legal_moves.size());m++) {
        if (legal_moves[m]==best_move){scores[m]+=1000000;}
        const int s=see(pos,legal_moves[m].to(),legal_moves[m]);
        scores[m]+=s;
    }
    for (int m=0;m<static_cast<int>(legal_moves.size());m++) { //iterate over moves
        int best_value=-BOUND;
        int best_idx=0;
        for (int n=m;n<static_cast<int>(legal_moves.size());n++) {
            if (scores[n]>best_value) {
                best_value=scores[n];
                best_idx=n;
            }
        }
        if (best_idx!=m) {
            std::swap(legal_moves[m],legal_moves[best_idx]);
            std::swap(scores[m],scores[best_idx]);
        }
        pos.makemove(legal_moves[m]); //make move
        move_history.push_back(legal_moves[m]);

        //pvs
        if (m==0) {child=quiescence(pos,-beta,-alpha,depth_to_root+1,move_history);}
        else {
            if (scores[m]<-100) { //prune bad see values
                pos.undomove();
                move_history.pop_back();
                continue;
            }
            child=quiescence(pos,-(alpha+1),-alpha,depth_to_root+1,move_history);
            if (-child.value>alpha&&-child.value<beta) {child=quiescence(pos,-beta,-alpha,depth_to_root+1,move_history);}
        }
        nodes+=child.nodes;
        if (-child.value>value) {
            value=-child.value;
            best_move=legal_moves[m];
            pv=child.pv;

        }
        pos.undomove();
        move_history.pop_back();
        if (value>alpha) {
            if (value>=beta) {
                break;
            }
            alpha=value;
        }
    }
    if (legal_moves.empty()){pv=move_history;}
    //tt
    entry.value=value;
    entry.hash=hash;
    entry.best_move=best_move;
    entry.depth_to_root=depth_to_root;
    entry.quies_node=true;
    auto stored_pv=pv;
    int remove = std::min(depth_to_root, static_cast<int>(stored_pv.size()));
    stored_pv.erase(stored_pv.begin(), stored_pv.begin() + remove);
    if (value<=alph_orig) {entry.flag=1;}
    else if (value>=beta) {entry.flag=-1;}
    else{entry.flag=0;}
    tt[hash]=entry;
    return {value,nodes,depth_to_root,pv};
}

inline nodeData negamax(libchess::Position &pos,int alpha,int beta,int depth, const int depth_to_root,std::vector<libchess::Move> &move_history,const bool is_null,int reductions=0) {
    if (pos.threefold()||pos.fiftymoves()){return {DRAW_SCORE+depth_to_root,1ull,depth_to_root,move_history};}
    if (depth<=0) {
        //return {eval(pos),1ull,depth_to_root,move_history};
        const nodeData node=quiescence(pos,alpha,beta,depth_to_root,move_history);
        return {node.value,node.nodes,node.depth,node.pv};
    }
    mo_nodes++;
    if (search.load(std::memory_order_relaxed)==-1) return {ABORT_SCORE,1ull,depth_to_root,move_history};
    //tt
    const bool is_pv=(beta-alpha)>1;
    const int alph_orig=alpha;
    libchess::Move best_move={};
    const uint64_t hash=pos.hash();
    ENTRY entry=tt[hash];
    if (entry.hash==hash) {
        if (entry.depth>=depth&&!entry.quies_node&&depth_to_root!=0) {
            //when calling tt
            //return move_history+stored_pv
            auto returned_pv=move_history;
            returned_pv.insert(returned_pv.end(),entry.pv.begin(),entry.pv.end());
            if (entry.flag==0){return {entry.value,1ull,entry.depth_to_root,returned_pv};}
            if (entry.flag==-1&&entry.value>=beta){return {entry.value,1ull,entry.depth_to_root,returned_pv};}
            if (entry.flag==1&&entry.value<=alpha){return {entry.value,1ull,entry.depth_to_root,returned_pv};}
        }
        best_move=entry.best_move;
    }
    //mate distance pruning
    alpha=std::max(-BOUND+depth_to_root, alpha);
    beta=std::min(BOUND-depth_to_root-1,beta);
    if (alpha>=beta){return {alpha,1ull,depth_to_root,move_history};}
    // //rfp
    if (!is_pv&&!pos.in_check()&&eval(pos)>=beta+150*depth) {
        //return {eval(pos),1ull,depth_to_root,move_history};
        const nodeData node=quiescence(pos,alpha,beta,depth_to_root,move_history);
        return {node.value,node.nodes,node.depth,node.pv};
    }
    //nmp
    if (!pos.in_check()&&!is_null&&eval(pos)>=beta&&phase(pos)>.2) {
        pos.makenull();
        move_history.emplace_back();
        auto child=negamax(pos,-beta,-beta+1,depth-(depth/3)-2,depth_to_root+1,move_history,true,reductions);
        pos.undonull();
        move_history.pop_back();
        if (-child.value>=beta) {
            return {-child.value,1ull,depth_to_root,move_history};
        }
    }
    //try best_move
    if (best_move!=libchess::Move()) {
        pos.makemove(best_move);
        move_history.push_back(best_move);
        auto child=negamax(pos,-beta,-alpha,depth-1,depth_to_root+1,move_history,is_null,reductions);
        pos.undomove();
        move_history.pop_back();
        if (-child.value>alpha) {
            if (-child.value>=beta) {
                return {-child.value,child.nodes,child.depth,child.pv};
            }
        }
    }
    auto legal_moves=pos.legal_moves(); //generate legal moves
    const bool check=pos.in_check();
    if (legal_moves.empty()) { //if no moves then
        if (check){return {-(BOUND-depth_to_root),1ull,depth_to_root,move_history};} //return because checkmate
        return {DRAW_SCORE+depth_to_root,1ull,depth_to_root,move_history}; //return because draw
    }
    std::vector<libchess::Move> pv;
    int value=-BOUND;
    nodeData child;
    uint64_t nodes=0ull;
    std::vector scores(legal_moves.size(),0);
    for (int m=0;m<static_cast<int>(legal_moves.size());m++) {
        //tt
        if (legal_moves[m]==best_move){scores[m]+=1000000;}
        //history
        scores[m]+=history[sqr_idx(legal_moves[m].from())][sqr_idx(legal_moves[m].to())];
        if (!move_history.empty()) {
            //countermove
            scores[m]+=(countermove_history[move_history.back().piece()][sqr_idx(move_history.back().to())]==legal_moves[m])*1000;
        }
        //pawns
        if (legal_moves[m].piece()==0&&legal_moves[m].to().rank()>=4&&pos.turn()==libchess::White) {
            auto mask= libchess::Bitboard(legal_moves[m].to().north());
            mask|=mask.north();
            mask|=mask.north();
            mask|=mask.north();
            mask|=mask.north();
            mask|=mask.north();
            scores[m]+=100;
            if (!(mask&pos.occupied())) {
                auto promo_sqr=libchess::Bitboard(legal_moves[m].to().north());
                auto last=promo_sqr;
                while (promo_sqr) {
                    last=promo_sqr;
                    promo_sqr=promo_sqr.north();
                }
                promo_sqr=last;
                const float moves_to_promo=distance(legal_moves[m].to(),promo_sqr.lsb());
                const float moves_to_capture=distance(promo_sqr.lsb(),pos.king_position(libchess::Black));
                if (moves_to_promo<moves_to_capture){scores[m]+=mg_value[4]*(1.f/moves_to_promo)*.25;}
            }
        }
        if (legal_moves[m].piece()==0&&legal_moves[m].to().rank()>=4&&pos.turn()==libchess::Black) {
            auto mask= libchess::Bitboard(legal_moves[m].to().south());
            mask|=mask.south();
            mask|=mask.south();
            mask|=mask.south();
            mask|=mask.south();
            mask|=mask.south();
            scores[m]+=100;
            if (!(mask&pos.occupied())) {
                auto promo_sqr=libchess::Bitboard(legal_moves[m].to().south());
                auto last=promo_sqr;
                while (promo_sqr) {
                    last=promo_sqr;
                    promo_sqr=promo_sqr.south();
                }
                promo_sqr=last;
                const float moves_to_promo=distance(legal_moves[m].to(),promo_sqr.lsb());
                const float moves_to_capture=distance(promo_sqr.lsb(),pos.king_position(libchess::White));
                if (moves_to_promo<moves_to_capture){scores[m]+=mg_value[4]*(1.f/moves_to_promo)*.25;}
            }
        }
        //psqt
        int from=sqr_idx(legal_moves[m].from());
        int to=sqr_idx(legal_moves[m].to());
        if (pos.turn()==libchess::Black) {
            from=flip(from);
            to=flip(to);
        }
        scores[m]+=mg_tables[legal_moves[m].piece()][to]-mg_tables[legal_moves[m].piece()][from]*phase(pos)*2;
        scores[m]+=eg_tables[legal_moves[m].piece()][to]-eg_tables[legal_moves[m].piece()][from]*(1-phase(pos))*2;
        //killers
        scores[m]+=(killer_moves[0][depth_to_root]==legal_moves[m])*500;
        scores[m]+=(killer_moves[1][depth_to_root]==legal_moves[m])*500;
        scores[m]+=(killer_moves[2][depth_to_root]==legal_moves[m])*500;
        //mvv-lva
        if (legal_moves[m].is_capturing()) {
            scores[m]+=(mat[legal_moves[m].captured()]*10-mat[legal_moves[m].piece()])*100000;
        }
        scores[m]+=(legal_moves[m].piece()==libchess::Pawn)*100;
        scores[m]+=100000*(legal_moves[m].type()==libchess::MoveType::promo);
        scores[m]+=5000*(legal_moves[m].type()==libchess::MoveType::ksc);
        scores[m]+=5000*(legal_moves[m].type()==libchess::MoveType::qsc);
    }
    for (int m=0;m<static_cast<int>(legal_moves.size());m++) { //iterate over moves
        //swap in place
        int best_value=-BOUND;
        int best_idx=0;
        for (int n=m;n<static_cast<int>(legal_moves.size());n++) {
            if (scores[n]>best_value) {
                best_value=scores[n];
                best_idx=n;
            }
        }
        if (best_idx!=m) {
            std::swap(legal_moves[m],legal_moves[best_idx]);
            std::swap(scores[m],scores[best_idx]);
        }
        pos.makemove(legal_moves[m]); //make move
        move_history.push_back(legal_moves[m]);
        //pvs
        int ext=0;

        //extension for:
        //being in check
        //pv and castling
        //pv and promoting
        libchess::MoveType type=legal_moves[m].type();
        if (check||
            (is_pv&&
                (type==4||type==5||
                    legal_moves[m].is_promoting()||
                    (legal_moves[m].is_capturing()&&legal_moves[m].captured()>legal_moves[m].piece())
                )
            )
            ) {
            ext=1;
        }
        if (m==0) {
            child=negamax(pos,-beta,-alpha,depth-1+ext,depth_to_root+1,move_history,is_null,reductions);
        }
        else {
            int lmr=0;
            lmr=1+static_cast<int>((log(m)*log(depth)));
            if (m>6){lmr+=1;}
            if (m>12){lmr+=1;}
            if (is_pv||legal_moves[m].is_capturing()||
                legal_moves[m].is_promoting()||
                killer_moves[0][depth_to_root]==legal_moves[m]||
                killer_moves[1][depth_to_root]==legal_moves[m]||
                killer_moves[2][depth_to_root]==legal_moves[m]||
                legal_moves[m].piece()==0){
                lmr=1;
                }
            if (ext){lmr=0;}
            child=negamax(pos,-(alpha+1),-alpha,depth-(1+lmr),depth_to_root+1,move_history,is_null,reductions);
            if (-child.value>alpha&&-child.value<beta) {
                child=negamax(pos,-beta,-alpha,depth-1+ext,depth_to_root+1,move_history,is_null,reductions);
            }
        }
        nodes+=child.nodes;
        if (-child.value>value) {
            value=-child.value;
            best_move=legal_moves[m];
            pv=child.pv;
        }
        pos.undomove(); //undo move
        move_history.pop_back();
        const bool is_capture=legal_moves[m].is_capturing();
        if (value>alpha) { //a-b
            best_move=legal_moves[m];
            if (value>=beta) { //beta cutoff
                beta_cutoff_ratio+=(static_cast<float>(m)/static_cast<float>(legal_moves.size()));
                beta_cutoff_count++;
                if (!is_capture) {
                    //history
                    history[sqr_idx(legal_moves[m].from())][sqr_idx(legal_moves[m].to())]+=depth*depth;
                    //countermove history
                    if (!move_history.empty()) {countermove_history[move_history.back().piece()][sqr_idx(move_history.back().to())]=legal_moves[m];}
                    //killers
                    if (killer_moves[0][depth_to_root]!=legal_moves[m]){killer_moves[0][depth_to_root]=legal_moves[m];}
                    else if (killer_moves[1][depth_to_root]!=legal_moves[m]){killer_moves[1][depth_to_root]=legal_moves[m];}
                    else if (killer_moves[2][depth_to_root]!=legal_moves[m]){killer_moves[2][depth_to_root]=legal_moves[m];}
                }
                break;
            }
            if (!is_capture) {history[sqr_idx(legal_moves[m].from())][sqr_idx(legal_moves[m].to())]-=depth*2;}
            alpha=value; //update alpha
        }
    }
    //tt
    entry.value=value;
    entry.hash=hash;
    entry.best_move=best_move;
    entry.depth_to_root=depth_to_root;
    auto stored_pv=pv;
    int remove=std::min(depth_to_root,static_cast<int>(stored_pv.size()));
    stored_pv.erase(stored_pv.begin(),stored_pv.begin()+remove);
    if (value<=alph_orig) {entry.flag=1;}
    else if (value>=beta) {entry.flag=-1;}
    else{entry.flag=0;}
    if (depth>entry.depth||entry.quies_node) { //depth based replacement scheme
        entry.depth=depth;
        entry.quies_node=false;
        tt[hash]=entry;
    }
    return {value,nodes,depth_to_root,pv};
}



#endif //CHESSBOT_SEARCH_H