#ifndef EVALUATION_H
#define EVALUATION_H
#include <iostream>
#include <libchess/position.hpp>
#include <libchess/movegen.hpp>
#include "params.h"

inline int flip(const int &idx) {return idx^56;}
inline int other(const int &side){return side^1;}
inline float phase(const libchess::Position &pos) {
    libchess::Bitboard occ=pos.occupied();
    occ^=pos.occupancy(libchess::Pawn); //pawns dont count towards the phase of the game, endgames can be all pawns.
    occ^=pos.occupancy(libchess::King); //kings dont count towards the phase, they cannot be removed.
    return static_cast<float>(occ.count())/14.f;
}

inline int eval(const libchess::Position &pos) {
    float value=0.f;
    const float ph=phase(pos);
    //material:
    float material=0.f;
    for (int side=0;side<2;side++) {
        for (int p=0;p<5;p++) {
            material+=mg_value[p]*static_cast<float>(pos.pieces(libchess::sides[side],libchess::pieces[p]).count())*static_cast<float>(side_multiplier[side])*ph;
            material+=eg_value[p]*static_cast<float>(pos.pieces(libchess::sides[side],libchess::pieces[p]).count())*static_cast<float>(side_multiplier[side])*(1-ph);
        }
    }
    //psqt:
    float psqt=0.f;
    for (int side=0;side<2;side++) {
        libchess::Bitboard side_occ=pos.occupancy(libchess::sides[side]);
        for (int p=0;p<6;p++) {
            uint64_t occ=pos.occupancy(libchess::pieces[p]).operator&=(side_occ).value();
            while (occ) {
                int sqr=__builtin_ctzll(occ);
                occ^=1ull<<sqr;
                int idx=sqr;
                if (side){idx=flip(sqr);}
                psqt+=mg_tables[p][idx]*static_cast<float>(side_multiplier[side])*ph;
                psqt+=eg_tables[p][idx]*static_cast<float>(side_multiplier[side])*(1-ph);
            }
        }
    }
    //piece calcs:
    const auto w_pawns=pos.pieces(libchess::White,libchess::Pawn);
    const auto w_knights=pos.pieces(libchess::White,libchess::Knight);
    const auto w_bishops=pos.pieces(libchess::White,libchess::Bishop);
    const auto w_rooks=pos.pieces(libchess::White,libchess::Rook);
    const auto w_queens=pos.pieces(libchess::White,libchess::Queen);
    const auto w_occ=w_pawns|w_knights|w_bishops|w_rooks|w_queens|pos.king_position(libchess::White);
    const auto b_pawns=pos.pieces(libchess::Black,libchess::Pawn);
    const auto b_knights=pos.pieces(libchess::Black,libchess::Knight);
    const auto b_bishops=pos.pieces(libchess::Black,libchess::Bishop);
    const auto b_rooks=pos.pieces(libchess::Black,libchess::Rook);
    const auto b_queens=pos.pieces(libchess::Black,libchess::Queen);
    const auto b_occ=b_pawns|b_knights|b_bishops|b_rooks|b_queens|pos.king_position(libchess::Black);
    libchess::Bitboard w_knight_att;
    libchess::Bitboard w_bishop_att;
    libchess::Bitboard w_rook_att;
    libchess::Bitboard w_queen_att;
    libchess::Bitboard b_knight_att;
    libchess::Bitboard b_bishop_att;
    libchess::Bitboard b_rook_att;
    libchess::Bitboard b_queen_att;
    libchess::Bitboard w_sqrs_attacked;
    w_sqrs_attacked|=w_pawns.north().east();
    w_sqrs_attacked|=w_pawns.north().west();
    libchess::Bitboard w_pawn_att=w_sqrs_attacked;
    for (const auto &fr:w_knights) {
        w_knight_att|=libchess::movegen::knight_moves(fr);
        w_sqrs_attacked|=w_knight_att;
    }
    for (const auto &fr:w_bishops) {
        w_bishop_att|=libchess::movegen::bishop_moves(fr,~pos.empty());
        w_sqrs_attacked|=w_bishop_att;
    }
    for (const auto &fr:w_rooks) {
        w_rook_att|=libchess::movegen::rook_moves(fr,~pos.empty());
        w_sqrs_attacked|=w_rook_att;
    }
    for (const auto &fr:w_queens) {
        w_queen_att|=libchess::movegen::queen_moves(fr,~pos.empty());
        w_sqrs_attacked|=w_queen_att;
    }
    w_sqrs_attacked|=libchess::movegen::king_moves(pos.king_position(libchess::White));
    libchess::Bitboard b_sqrs_attacked;
    b_sqrs_attacked|=b_pawns.south().east();
    b_sqrs_attacked|=b_pawns.south().west();
    libchess::Bitboard b_pawn_att=b_sqrs_attacked;
    for (const auto &fr:b_knights) {
        b_knight_att|=libchess::movegen::knight_moves(fr);
        b_sqrs_attacked|=b_knight_att;
    }
    for (const auto &fr:b_bishops) {
        b_bishop_att|=libchess::movegen::bishop_moves(fr,~pos.empty());
        b_sqrs_attacked|=b_bishop_att;
    }
    for (const auto &fr:b_rooks) {
        b_rook_att|=libchess::movegen::rook_moves(fr,~pos.empty());
        b_sqrs_attacked|=b_rook_att;
    }
    for (const auto &fr:b_queens) {
        b_queen_att|=libchess::movegen::queen_moves(fr,~pos.empty());
        b_sqrs_attacked|=b_queen_att;
    }
    b_sqrs_attacked|=libchess::movegen::king_moves(pos.king_position(libchess::Black));
    //bonus for mobility -not including king or pawns:
    float mobility=0.f;
    mobility+=static_cast<float>((w_knight_att&~b_sqrs_attacked&~w_occ).count())*mg_value[1]/100;
    mobility+=static_cast<float>((w_bishop_att&~b_sqrs_attacked&~w_occ).count())*mg_value[2]/100;
    mobility+=static_cast<float>((w_rook_att&~b_sqrs_attacked&~w_occ).count())*mg_value[3]/100;
    mobility+=static_cast<float>((w_queen_att&~b_sqrs_attacked&~w_occ).count())*mg_value[4]/100;
    mobility-=static_cast<float>((b_knight_att&~w_sqrs_attacked&~b_occ).count())*mg_value[1]/100;
    mobility-=static_cast<float>((b_bishop_att&~w_sqrs_attacked&~b_occ).count())*mg_value[2]/100;
    mobility-=static_cast<float>((b_rook_att&~w_sqrs_attacked&~b_occ).count())*mg_value[3]/100;
    mobility-=static_cast<float>((b_queen_att&~w_sqrs_attacked&~b_occ).count())*mg_value[4]/100;
    //malus for hanging pieces:
    float hanging=0.f;
    hanging-=static_cast<float>((w_pawns&b_sqrs_attacked&~w_sqrs_attacked).count())*mg_value[0]/10;
    hanging-=static_cast<float>((w_knights&b_sqrs_attacked&~w_sqrs_attacked).count())*mg_value[1]/10;
    hanging-=static_cast<float>((w_bishops&b_sqrs_attacked&~w_sqrs_attacked).count())*mg_value[2]/10;
    hanging-=static_cast<float>((w_rooks&b_sqrs_attacked&~w_sqrs_attacked).count())*mg_value[3]/10;
    hanging-=static_cast<float>((w_queens&b_sqrs_attacked&~w_sqrs_attacked).count())*mg_value[4]/10;
    hanging+=static_cast<float>((b_pawns&w_sqrs_attacked&~b_sqrs_attacked).count())*mg_value[0]/10;
    hanging+=static_cast<float>((b_knights&w_sqrs_attacked&~b_sqrs_attacked).count())*mg_value[1]/10;
    hanging+=static_cast<float>((b_bishops&w_sqrs_attacked&~b_sqrs_attacked).count())*mg_value[2]/10;
    hanging+=static_cast<float>((b_rooks&w_sqrs_attacked&~b_sqrs_attacked).count())*mg_value[3]/10;
    hanging+=static_cast<float>((b_queens&w_sqrs_attacked&~b_sqrs_attacked).count())*mg_value[4]/10;
    auto w_king_adj=libchess::Bitboard(pos.king_position(libchess::White)).adjacent();
    auto b_king_adj=libchess::Bitboard(pos.king_position(libchess::Black)).adjacent();
    //bonus for attacking the enemy king:
    float king_safety=0.f;
    king_safety+=expf(static_cast<float>((w_sqrs_attacked&b_king_adj).count())/static_cast<float>(b_king_adj.count()))*8-8;
    king_safety-=expf(static_cast<float>((b_sqrs_attacked&w_king_adj).count())/static_cast<float>(w_king_adj.count()))*8-8;
    king_safety+=expf(static_cast<float>((w_knight_att&b_king_adj).count())/static_cast<float>(b_king_adj.count()))*3-3;
    king_safety-=expf(static_cast<float>((b_knight_att&w_king_adj).count())/static_cast<float>(w_king_adj.count()))*3-3;
    king_safety+=expf(static_cast<float>((w_bishop_att&b_king_adj).count())/static_cast<float>(b_king_adj.count()))*3-3;
    king_safety-=expf(static_cast<float>((b_bishop_att&w_king_adj).count())/static_cast<float>(w_king_adj.count()))*3-3;
    king_safety+=expf(static_cast<float>((w_rook_att&b_king_adj).count())/static_cast<float>(b_king_adj.count()))*5-5;
    king_safety-=expf(static_cast<float>((b_rook_att&w_king_adj).count())/static_cast<float>(w_king_adj.count()))*5-5;
    king_safety+=expf(static_cast<float>((w_queen_att&b_king_adj).count())/static_cast<float>(b_king_adj.count()))*10-10;
    king_safety-=expf(static_cast<float>((b_queen_att&w_king_adj).count())/static_cast<float>(w_king_adj.count()))*10-10;
    float tempo=45*ph;
    //rooks and queens on the 7th rank
    float seventh=0.f;
    seventh+=(w_rooks&libchess::bitboards::ranks[6]).count()*50;
    seventh+=(w_queens&libchess::bitboards::ranks[6]).count()*65;
    seventh-=(b_rooks&libchess::bitboards::ranks[1]).count()*50;
    seventh-=(b_queens&libchess::bitboards::ranks[1]).count()*65;
    //bonus for protected pieces
    float protect=0.f;
    protect+=(w_pawn_att&w_pawns).count()*1.f;
    protect+=(w_pawn_att&w_knights).count()*3.2f;
    protect+=(w_pawn_att&w_bishops).count()*3.3f;
    protect+=(w_pawn_att&w_rooks).count()*1.f;
    protect+=(w_pawn_att&w_queens).count()*1.f;
    protect+=(w_knight_att&w_pawns).count()*1.f;
    protect+=(w_knight_att&w_knights).count()*3.2f;
    protect+=(w_knight_att&w_bishops).count()*3.3f;
    protect+=(w_knight_att&w_rooks).count()*1.f;
    protect+=(w_knight_att&w_queens).count()*1.f;
    protect+=(w_bishop_att&w_pawns).count()*1.f;
    protect+=(w_bishop_att&w_knights).count()*3.2f;
    protect+=(w_bishop_att&w_bishops).count()*3.3f;
    protect+=(w_bishop_att&w_rooks).count()*1.f;
    protect+=(w_bishop_att&w_queens).count()*1.f;
    protect+=(w_rook_att&w_pawns).count()*1.f;
    protect+=(w_rook_att&w_knights).count()*3.2f;
    protect+=(w_rook_att&w_bishops).count()*3.3f;
    protect+=(w_rook_att&w_rooks).count()*5.f;
    protect+=(w_rook_att&w_queens).count()*9.f;
    protect+=(w_queen_att&w_pawns).count()*1.f;
    protect+=(w_queen_att&w_knights).count()*3.2f;
    protect+=(w_queen_att&w_bishops).count()*3.3f;
    protect+=(w_queen_att&w_rooks).count()*5.f;
    protect+=(w_queen_att&w_queens).count()*9.f;
    protect-=(b_pawn_att&b_pawns).count()*1.f;
    protect-=(b_pawn_att&b_knights).count()*3.2f;
    protect-=(b_pawn_att&b_bishops).count()*3.3f;
    protect-=(b_pawn_att&b_rooks).count()*1.f;
    protect-=(b_pawn_att&b_queens).count()*1.f;
    protect-=(b_knight_att&b_pawns).count()*1.f;
    protect-=(b_knight_att&b_knights).count()*3.2f;
    protect-=(b_knight_att&b_bishops).count()*3.3f;
    protect-=(b_knight_att&b_rooks).count()*1.f;
    protect-=(b_knight_att&b_queens).count()*1.f;
    protect-=(b_bishop_att&b_pawns).count()*1.f;
    protect-=(b_bishop_att&b_knights).count()*3.2f;
    protect-=(b_bishop_att&b_bishops).count()*3.3f;
    protect-=(b_bishop_att&b_rooks).count()*1.f;
    protect-=(b_bishop_att&b_queens).count()*1.f;
    protect-=(b_rook_att&b_pawns).count()*1.f;
    protect-=(b_rook_att&b_knights).count()*3.2f;
    protect-=(b_rook_att&b_bishops).count()*3.3f;
    protect-=(b_rook_att&b_rooks).count()*5.f;
    protect-=(b_rook_att&b_queens).count()*9.f;
    protect-=(b_queen_att&b_pawns).count()*1.f;
    protect-=(b_queen_att&b_knights).count()*3.2f;
    protect-=(b_queen_att&b_bishops).count()*3.3f;
    protect-=(b_queen_att&b_rooks).count()*5.f;
    protect-=(b_queen_att&b_queens).count()*9.f;

    //entropy

    //draws from material insufficiency
    //unstoppable pawn

    value+=material*1.f;
    value+=psqt*1.0f;
    value+=mobility*2.f;
    value+=hanging*2.f;
    value+=king_safety*1.f;
    value+=tempo*1.f;
    value+=seventh*1.f;
    value+=protect*1.f;
    return static_cast<int>(value)*side_multiplier[pos.turn()];
}





#endif //EVALUATION_H
