#ifndef CHESSBOT_INSUFFICIENT_MATERIAL_H
#define CHESSBOT_INSUFFICIENT_MATERIAL_H
#include "libchess/position.hpp"

inline bool insufficient_material(const libchess::Position &pos) {
    if (pos.occupied().count()>4||
        !pos.occupancy(libchess::Pawn).empty()||
        !pos.occupancy(libchess::Rook).empty()||
        !pos.occupancy(libchess::Queen).empty()) {
        return false;
    }
    if (pos.occupied().count()<=3){return true;}
    if (pos.pieces(libchess::White,libchess::Knight).count()==2||
        pos.pieces(libchess::Black,libchess::Knight).count()==2) {
        return true;
    }
    if ((libchess::Square(pos.pieces(libchess::White,libchess::Bishop).lsb()).dark()&&
        libchess::Square(pos.pieces(libchess::Black,libchess::Bishop).lsb()).dark())||(
        libchess::Square(pos.pieces(libchess::White,libchess::Bishop).lsb()).light()&&
        libchess::Square(pos.pieces(libchess::Black,libchess::Bishop).lsb()).light())) {
        return true;
    }
    return false;
}


#endif //CHESSBOT_INSUFFICIENT_MATERIAL_H