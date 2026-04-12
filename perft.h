#ifndef CHESSBOT_PERFT_H
#define CHESSBOT_PERFT_H
#include <algorithm>
#include "libchess/position.hpp"
#include "random.h"

inline uint64_t perft(libchess::Position &pos,const int depth) {
    if (depth<=0){return 1ull;}
    uint64_t nodes=0;
    auto moves=pos.legal_moves();
    std::ranges::shuffle(moves,gen);
    for (const auto &move:moves) {
        pos.makemove(move);
        nodes+=perft(pos,depth-1);
        pos.undomove();
    }
    return nodes;
}

#endif //CHESSBOT_PERFT_H