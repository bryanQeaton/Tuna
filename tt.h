#ifndef CHESSBOT_TT_H
#define CHESSBOT_TT_H
#include "libchess/position.hpp"

//transposition table
struct ENTRY {
    uint64_t hash=0;
    int flag=-1;
    int value=-BOUND;
    int depth=0;
    int depth_to_root=0;
    bool quies_node=false;
    libchess::Move best_move{};
};
class TT {
    std::vector<ENTRY> cells{};
    uint64_t size=0;
public:
    explicit TT(const int hash_size_mb=8) {
        constexpr auto entry_size=sizeof(ENTRY);
        const uint64_t hash_size_byte=hash_size_mb*1000000;
        size=hash_size_byte/entry_size;
        cells.resize(size);
    }
    void clear() {std::ranges::fill(cells,ENTRY());}
    ENTRY &operator[](const uint64_t idx) {return cells[idx%size];}

};





#endif //CHESSBOT_TT_H