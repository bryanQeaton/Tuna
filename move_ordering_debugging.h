#ifndef CHESSBOT_MOVE_ORDERING_DEBUGGING_H
#define CHESSBOT_MOVE_ORDERING_DEBUGGING_H
#include <chrono>

#include "root.h"

inline std::string test_fens[42] = {
    // Tactical middlegames
    "r2q1rk1/pp1nbppp/2p1pn2/3p4/3P4/2NBPN2/PPQ2PPP/R1B2RK1 w - -",
    "r1bq1rk1/pp2bppp/2n1pn2/2pp4/3P4/2NBPN2/PPQ2PPP/R1B2RK1 w - -",
    "r2q1rk1/ppp1bppp/2n1pn2/3p4/3P4/2NBPN2/PPQ2PPP/R1B2RK1 b - -",
    "r1bq1rk1/pp3ppp/2n1pn2/2bp4/3P4/2NBPN2/PPQ2PPP/R1B2RK1 w - -",
    "r2q1rk1/pp1nbppp/2p1pn2/8/2BP4/2N1PN2/PPQ2PPP/R1B2RK1 b - -",
    "r1bq1rk1/pp1n1ppp/2pbpn2/8/2BPP3/2N1BN2/PP3PPP/R2Q1RK1 w - -",
    "r2q1rk1/pp1n1ppp/2pbpn2/8/2BPP3/2N1BN2/PP3PPP/R2Q1RK1 b - -",
    "r1bq1rk1/pp3ppp/2nbpn2/2pp4/2P5/2NPBN2/PP2QPPP/R3KB1R w KQ -",
    "r2q1rk1/pp3ppp/2nbpn2/2pp4/2P5/2NPBN2/PP2QPPP/R3KB1R b KQ -",
    "r1bq1rk1/ppp1bppp/2n1pn2/3p4/2PP4/2N1PN2/PP2BPPP/R1BQ1RK1 w - -",

    // Sharp middlegames
    "r4rk1/1pp1qppp/p1np1n2/4p3/2BPP3/2N1BN2/PP3PPP/R2Q1RK1 w - -",
    "r4rk1/1pp1qppp/p1np1n2/4p3/2BPP3/2N1BN2/PP3PPP/R2Q1RK1 b - -",
    "2r2rk1/pp1n1ppp/2pbpn2/q7/3PP3/2N1BN2/PPQ2PPP/2RR2K1 w - -",
    "2r2rk1/pp1n1ppp/2pbpn2/q7/3PP3/2N1BN2/PPQ2PPP/2RR2K1 b - -",

    // Checking / forcing positions
    "6k1/5ppp/4p3/3rP3/3P4/2R3P1/5P1P/6K1 w - -",
    "6k1/5ppp/4p3/3rP3/3P4/2R3P1/5P1P/6K1 b - -",
    "7k/5Q2/6K1/8/8/8/8/8 b - -",
    "6k1/8/8/8/8/6K1/5Q2/8 w - -",
    "4k3/8/8/8/8/8/4q3/4K3 w - -",
    "r3k2r/pppq1ppp/2n5/3bp3/3P4/2N1PN2/PPP2PPP/R2Q1RK1 b kq -",
    "r3k2r/pppq1ppp/2n5/3bp3/3P4/2N1PN2/PPP2PPP/R2Q1RK1 w kq -",

    // Promotions / pawn races
    "8/P6k/8/8/8/8/6Kp/8 w - -",
    "8/P6k/8/8/8/8/6Kp/8 b - -",
    "2k5/P7/1K6/8/8/8/7p/8 w - -",
    "2k5/P7/1K6/8/8/8/7p/8 b - -",
    "8/3k4/2p5/2Pp4/3P4/2K5/8/8 w - -",
    "8/3k4/2p5/2Pp4/3P4/2K5/8/8 b - -",

    // Zugzwang / low mobility
    "8/8/8/2k5/2P5/2K5/8/8 w - -",
    "8/8/8/2k5/2P5/2K5/8/8 b - -",
    "8/8/8/8/3k4/3P4/3K4/8 w - -",
    "8/8/8/8/3k4/3P4/3K4/8 b - -",
    "8/8/2k5/8/2B5/3K4/8/8 w - -",
    "8/8/2k5/8/2B5/3K4/8/8 b - -",

    // Rook endgames (ordering sensitive)
    "8/8/1k6/1p6/1P6/1K6/8/R7 w - -",
    "8/6k1/8/5p2/5P2/6K1/8/R7 w - -",

    // Queen endgames (huge checking trees)
    "6k1/8/8/8/8/6K1/5Q2/8 w - -",
    "7k/5Q2/6K1/8/8/8/8/8 b - -",

    // En-passant / castling edge cases
    "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6",
    "r3k2r/8/8/8/8/8/8/R3K2R w KQkq -",
    "r3k2r/8/8/8/8/8/8/R3K2R b KQkq -",
    "4k2r/8/8/8/8/8/8/R3K2R w KQ -",
    "4k2r/8/8/8/8/8/8/R3K2R b KQ -"

};
inline void move_ordering_testing(const int depth=8) {
    auto pos=libchess::Position("startpos");
    int ms=0;
    for (const auto & test_fen : test_fens) {
        pos.set_fen(test_fen);
        auto t0=std::chrono::high_resolution_clock::now();
        const uint64_t last_nodes=mo_nodes;
        std::vector<libchess::Move> movelist;
        const auto ret=negamax(pos,-BOUND,BOUND,depth,0,movelist,false,0);
        std::cout<<"nodes: "<<mo_nodes-last_nodes<<" | "<<test_fen<<" | eval:"<<ret.value<<"\n";
        auto t1=std::chrono::high_resolution_clock::now();
        ms+=static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count());
    }
    std::cout<<"ms: "<<ms<<" nodes: "<<mo_nodes<<" cutoff ratio: "<<1-(beta_cutoff_ratio/beta_cutoff_count)<<"\n";
}


#endif //CHESSBOT_MOVE_ORDERING_DEBUGGING_H