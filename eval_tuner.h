#ifndef CHESSBOT_EVAL_TUNER_H
#define CHESSBOT_EVAL_TUNER_H
#include <cstring>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <iostream>
#include <iomanip>
#include "random.h"







constexpr float k=1.0f;
inline float sig(const float evaluation) {
    return 1.0f/(1.0f+powf(10.0f,-evaluation*k/400.0f));
}
inline void display_params() {
    std::cout<<"==================================\n";
    std::cout<<"inline float mg_value[6]={";
    for (int i=0;i<6;i++) {
        std::cout<<std::fixed<<std::setprecision(1)<<mg_value[i];
        if (i<5){std::cout<<",";}
        else {std::cout<<"};\n";}
    }
    std::cout<<"inline float eg_value[6]={";
    for (int i=0;i<6;i++) {
        std::cout<<std::fixed<<std::setprecision(1)<<eg_value[i];
        if (i<5){std::cout<<",";}
        else {std::cout<<"};\n";}
    }
    std::cout<<"inline float knight_mobility[9]={";
    for (int i=0;i<9;i++) {
        std::cout<<std::fixed<<std::setprecision(1)<<knight_mobility[i];
        if (i<8){std::cout<<",";}
        else {std::cout<<"};\n";}
    }
    std::cout<<"inline float bishop_mobility[27]={";
    for (int i=0;i<27;i++) {
        std::cout<<std::fixed<<std::setprecision(1)<<bishop_mobility[i];
        if (i<26){std::cout<<",";}
        else {std::cout<<"};\n";}
    }
    std::cout<<"inline float rook_mobility[29]={";
    for (int i=0;i<29;i++) {
        std::cout<<std::fixed<<std::setprecision(1)<<rook_mobility[i];
        if (i<28){std::cout<<",";}
        else {std::cout<<"};\n";}
    }
    std::cout<<"inline float queen_mobility[28]={";
    for (int i=0;i<28;i++) {
        std::cout<<std::fixed<<std::setprecision(1)<<queen_mobility[i];
        if (i<27){std::cout<<",";}
        else {std::cout<<"};\n";}
    }
    std::cout<<"inline float mg_pawn_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<mg_pawn_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout<<"inline float eg_pawn_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<eg_pawn_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout<<"inline float mg_knight_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<mg_knight_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout<<"inline float eg_knight_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<eg_knight_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout<<"inline float mg_bishop_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<mg_bishop_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout<<"inline float eg_bishop_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<eg_bishop_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout<<"inline float mg_rook_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<mg_rook_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout<<"inline float eg_rook_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<eg_rook_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout<<"inline float mg_queen_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<mg_queen_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout<<"inline float eg_queen_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<eg_queen_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout<<"inline float mg_king_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<mg_king_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout<<"inline float eg_king_table[64]={";
    for (int i=0;i<64;i++) {
        if (i%8==0){std::cout<<"\n";}
        std::cout<<std::fixed<<std::setprecision(1)<<eg_king_table[i];
        if (i<63){std::cout<<",";}
        else {std::cout<<"\n};\n";}
    }
    std::cout.unsetf(std::ios::floatfield);  // removes fixed/scientific
    std::cout<<std::setprecision(6);       // default precision
    std::cout<<"==================================\n";

}
inline float match(const int games,const int move_time,const float *nudges) {
    float wr=0.f;
    for (int i=0;i<games;i++) {
        auto position=libchess::Position("startpos");
        tt.clear();
        int c=0;
        while (!position.is_terminal()&&!insufficient_material(position)&&position.fullmoves()<150) {
            if (abs(material_eval(position))>600){c++;}
            if (c>=6) {break;}
            bool is_pertrubed=false;
            if (position.turn()==libchess::White) {
                is_pertrubed=true;
                for (int i=0;i<64;i++) {mg_rook_table[i]+=nudges[i];}
            }
            auto move=root(position,move_time,MAX_DEPTH).best_move;
            position.makemove(move);
            if (is_pertrubed) {
                for (int i=0;i<64;i++) {mg_rook_table[i]-=nudges[i];}

            }
        }
        if ((position.is_checkmate()&&position.turn()==libchess::Black)||material_eval(position)>6) {wr+=1.f;}
        else if ((position.is_checkmate()&&position.turn()==libchess::White)||material_eval(position)<-6) {wr+=0.f;}
        else {wr+=.5f;}
    }
    return wr/=static_cast<float>(games);
}

inline void tune(const int samples) {
    for (int i=0;i<samples;i++) {
        float nudges[64]{};
        for (auto &n:nudges){n=real_dist(gen)*10.f;}
        float result=match(3,25,nudges);
        std::cout<<result<<"\n";
        if (result>.5f) {for (int i=0;i<64;i++) {mg_rook_table[i]+=nudges[i];}}
    }
    display_params();
}




#endif //CHESSBOT_EVAL_TUNER_H