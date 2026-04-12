#ifndef EVALUATION_H
#define EVALUATION_H
#include <iostream>
#include <libchess/position.hpp>
#include <libchess/movegen.hpp>
#include <fstream>

#include "insufficient_material.h"
#include "params.h"
#include "random.h"
inline float phase(const libchess::Position &pos) {
    libchess::Bitboard occ=pos.occupied();
    occ^=pos.occupancy(libchess::Pawn); //pawns dont count towards the phase of the game, endgames can be all pawns.
    occ^=pos.occupancy(libchess::King); //kings dont count towards the phase, they cannot be removed.
    return static_cast<float>(occ.count())/14.f;
}
inline int flip(const int &idx) {return idx^56;}
inline int other(const int &side){return side^1;}
inline float distance(const libchess::Square a,const libchess::Square b) {return static_cast<float>(std::max(abs(a.file()-b.file()),abs(a.rank()-b.rank())));}

inline int material_eval(const libchess::Position &pos) {
    int value=0.f;
    for (int side=0;side<2;side++) {
        for (int p=0;p<5;p++) {
            value+=mat[p]*pos.pieces(libchess::sides[side],libchess::pieces[p]).count()*side_multiplier[side];
        }
    }
    return value;
}

inline int eval(const libchess::Position &pos) {
    if (insufficient_material(pos)){return DRAW_SCORE;}
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
            uint64_t occ=pos.occupancy(libchess::pieces[p]).operator&(side_occ).value();
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
    w_sqrs_attacked|=libchess::movegen::king_moves(pos.king_position(libchess::White));
    libchess::Bitboard b_sqrs_attacked;
    b_sqrs_attacked|=b_pawns.south().east();
    b_sqrs_attacked|=b_pawns.south().west();
    libchess::Bitboard b_pawn_att=b_sqrs_attacked;
    b_sqrs_attacked|=libchess::movegen::king_moves(pos.king_position(libchess::Black));
    for (const auto &fr:w_knights) {
        libchess::Bitboard temp=libchess::movegen::knight_moves(fr);
        w_knight_att|=temp;
        w_sqrs_attacked|=w_knight_att;
    }
    for (const auto &fr:w_bishops) {
        libchess::Bitboard temp=libchess::movegen::bishop_moves(fr,~pos.empty());
        w_bishop_att|=temp;
        w_sqrs_attacked|=w_bishop_att;
    }
    for (const auto &fr:w_rooks) {
        libchess::Bitboard temp=libchess::movegen::rook_moves(fr,~pos.empty());
        w_rook_att|=temp;
        w_sqrs_attacked|=w_rook_att;
    }
    for (const auto &fr:b_knights) {
        libchess::Bitboard temp=libchess::movegen::knight_moves(fr);
        b_knight_att|=temp;
        b_sqrs_attacked|=b_knight_att;
    }
    for (const auto &fr:b_bishops) {
        libchess::Bitboard temp=libchess::movegen::bishop_moves(fr,~pos.empty());
        b_bishop_att|=temp;
        b_sqrs_attacked|=b_bishop_att;
    }
    for (const auto &fr:b_rooks) {
        libchess::Bitboard temp=libchess::movegen::rook_moves(fr,~pos.empty());
        b_rook_att|=temp;
        b_sqrs_attacked|=b_rook_att;
    }
    for (const auto &fr:b_queens) {
        b_queen_att|=libchess::movegen::queen_moves(fr,~pos.empty());
        b_sqrs_attacked|=b_queen_att;
    }
    libchess::Bitboard w_greater[5]={
        w_knights|w_bishops|w_rooks|w_queens,
        w_rooks|w_queens,
        w_rooks|w_queens,
        w_queens,
        libchess::Bitboard(0ull)
    };
    libchess::Bitboard b_greater[5]={
        b_knights|b_bishops|b_rooks|b_queens,
        b_rooks|b_queens,
        b_rooks|b_queens,
        b_queens,
        libchess::Bitboard(0ull)
    };
    libchess::Bitboard b_lesser_att[6]={libchess::Bitboard(0),b_pawn_att,b_pawn_att,(b_pawn_att|b_knight_att|b_bishop_att),(b_pawn_att|b_knight_att|b_bishop_att|b_rook_att),libchess::Bitboard(0)};
    libchess::Bitboard w_lesser_att[6]={libchess::Bitboard(0),w_pawn_att,w_pawn_att,(w_pawn_att|w_knight_att|w_bishop_att),(w_pawn_att|w_knight_att|w_bishop_att|w_rook_att),libchess::Bitboard(0)};
    //bonus for mobility -not including king or pawns:
    float mobility=0.f;
    mobility+=knight_mobility[std::min((w_knight_att&~b_lesser_att[1]&~w_occ).count(),8)];
    mobility+=bishop_mobility[std::min((w_bishop_att&~b_lesser_att[1]&~w_occ).count(),26)];
    mobility+=rook_mobility[std::min((w_rook_att&~b_lesser_att[1]&~w_occ).count(),28)];
    mobility+=queen_mobility[std::min((w_queen_att&~b_lesser_att[1]&~w_occ).count(),27)];
    mobility-=knight_mobility[std::min((b_knight_att&~w_lesser_att[1]&~b_occ).count(),8)];
    mobility-=bishop_mobility[std::min((b_bishop_att&~w_lesser_att[1]&~b_occ).count(),26)];
    mobility-=rook_mobility[std::min((b_rook_att&~w_lesser_att[1]&~b_occ).count(),28)];
    mobility-=queen_mobility[std::min((b_queen_att&~w_lesser_att[1]&~b_occ).count(),27)];
    float bishop_pairs=0.f;
    if (w_bishops.count()>=2){bishop_pairs+=50.f;}
    if (b_bishops.count()>=2){bishop_pairs-=50.f;}
    float outposts=0.f;
    auto potential_outposts=w_pawn_att;
    while (potential_outposts) {
        libchess::Square s=potential_outposts.lsb();
        auto sqr=libchess::Bitboard(s);
        potential_outposts^=sqr;
        if (s.rank()<4||(sqr&b_occ)){continue;} //on the opponents side and not occupied by the opponent
        libchess::Bitboard mask=sqr.north().east()|sqr.north().west();
        mask|=mask.north();
        mask|=mask.north();
        mask|=mask.north();
        if (mask&b_pawn_att){continue;} //not defendable by pawns
        outposts+=10.f;
        if (sqr&w_knights){outposts+=145.f;}
        if (sqr&w_bishops){outposts+=145.f;}
        if (sqr&w_rooks){outposts+=45.f;}
        if (sqr&w_queens){outposts+=45.f;}
    }
    potential_outposts=b_pawn_att;
    while (potential_outposts) {
        libchess::Square s=potential_outposts.lsb();
        auto sqr=libchess::Bitboard(s);
        potential_outposts^=sqr;
        if (s.rank()>=4||(sqr&b_occ)){continue;} //on the opponents side and not occupied by the opponent
        libchess::Bitboard mask=sqr.north().east()|sqr.north().west();
        mask|=mask.north();
        mask|=mask.north();
        mask|=mask.north();
        if (sqr&w_pawn_att){continue;} //not defendable by pawns
        outposts-=10.f;
        if (sqr&b_knights){outposts-=145.f;}
        if (sqr&b_bishops){outposts-=145.f;}
        if (sqr&b_rooks){outposts-=45.f;}
        if (sqr&b_queens){outposts-=45.f;}
    }
    float rook_on_open_files=0.f;
    libchess::Bitboard open_files;
    libchess::Bitboard w_semi_open_files;
    libchess::Bitboard b_semi_open_files;
    for (auto file:libchess::bitboards::files) {
        if (!(file&(w_pawns|b_pawns))){open_files|=file;}
        if (file&b_pawns&&!(file&w_pawns)) {
            w_semi_open_files|=file;
        }
        if (file&w_pawns&&!(file&b_pawns)) {
            b_semi_open_files|=file;
        }
    }
    if ((w_rooks&open_files)){rook_on_open_files+=80.f;}
    if ((w_rooks&w_semi_open_files)){rook_on_open_files+=50.f;}
    if ((b_rooks&open_files)){rook_on_open_files-=80.f;}
    if ((b_rooks&b_semi_open_files)){rook_on_open_files-=50.f;}
    float unstoppable_pawn=0.f;
    auto w_passed=pos.passed_pawns(libchess::White);
    auto b_passed=pos.passed_pawns(libchess::Black);
    if (!b_knights&&!b_bishops&&!b_rooks&&!b_queens&&w_passed) {
        auto temp=w_passed;
        while (temp) {
            libchess::Square s=temp.lsb();
            auto sqr=libchess::Bitboard(s);
            temp^=sqr;
            auto mask=sqr.north();
            mask|=mask.north();
            mask|=mask.north();
            mask|=mask.north();
            mask|=mask.north();
            mask|=mask.north();
            if (mask&w_occ){continue;}
            auto promo_sqr=sqr.north();
            auto last=promo_sqr;
            while (promo_sqr) {
                last=promo_sqr;
                promo_sqr=promo_sqr.north();
            }
            promo_sqr=last;
            float moves_to_promo=distance(s,promo_sqr.lsb());
            float moves_to_capture=distance(promo_sqr.lsb(),pos.king_position(libchess::Black));
            if (moves_to_promo<moves_to_capture){unstoppable_pawn+=mg_value[4]*(1.f/moves_to_promo);}
        }
    }
    if (!w_knights&&!w_bishops&&!w_rooks&&!w_queens&&b_passed) {
        auto temp=b_passed;
        while (temp) {
            libchess::Square s=temp.lsb();
            auto sqr=libchess::Bitboard(s);
            temp^=sqr;
            auto mask=sqr.south();
            mask|=mask.south();
            mask|=mask.south();
            mask|=mask.south();
            mask|=mask.south();
            mask|=mask.south();
            if (mask&b_occ){continue;}
            auto promo_sqr=sqr.south();
            auto last=promo_sqr;
            while (promo_sqr) {
                last=promo_sqr;
                promo_sqr=promo_sqr.south();
            }
            promo_sqr=last;
            float moves_to_promo=distance(s,promo_sqr.lsb());
            float moves_to_capture=distance(promo_sqr.lsb(),pos.king_position(libchess::White));
            if (moves_to_promo<moves_to_capture){unstoppable_pawn-=mg_value[4]*(1.f/moves_to_promo);}
        }
    }
    float king_attack=0.f;
    auto w_king_area_immediate=libchess::Bitboard(pos.king_position(libchess::White)).adjacent();
    auto b_king_area_immediate=libchess::Bitboard(pos.king_position(libchess::Black)).adjacent();
    float w_king_immediate_attack=static_cast<float>((b_king_area_immediate&w_sqrs_attacked).count())/static_cast<float>(b_king_area_immediate.count());
    float b_king_immediate_attack=static_cast<float>((w_king_area_immediate&b_sqrs_attacked).count())/static_cast<float>(w_king_area_immediate.count());
    king_attack+=expf(w_king_immediate_attack*3)*10;
    king_attack-=expf(b_king_immediate_attack*3)*10;
    float passed_pawn=0.f;
    passed_pawn+=static_cast<float>(pos.passed_pawns(libchess::White).count())*50.f;
    passed_pawn-=static_cast<float>(pos.passed_pawns(libchess::Black).count())*50.f;
    value+=material*1.0f;
    value+=psqt*0.5f;
    value+=mobility*0.8f;
    value+=bishop_pairs*1.8f;
    value+=outposts*0.5f;
    value+=rook_on_open_files*0.8f;
    value+=unstoppable_pawn*0.6f;
    value+=king_attack*1.0f;
    value+=passed_pawn*1.0f;
    return static_cast<int>(value)*side_multiplier[pos.turn()];
}





#endif //EVALUATION_H
