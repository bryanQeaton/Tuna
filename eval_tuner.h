#ifndef CHESSBOT_EVAL_TUNER_H
#define CHESSBOT_EVAL_TUNER_H
#include <cstring>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <iostream>
#include <iomanip>

inline std::uniform_real_distribution real_dist(-5.f,5.f);
inline float random_sign() {
    if (real_dist(gen)>.5){return 1.f;}
    return -1.f;
}
struct Data {
    libchess::Position pos;
    float true_score=0;
};
const std::string DATASET_FILE="C:/Users/bryan/CLionProjects/ChessBot/LumbrasGigaBase_OTB_2020-2024.pgn";

inline std::vector<Data> gen_dataset(const int file_r_low=0,const int file_r_high=19000) {
    std::vector<Data> ds;
    std::ifstream file(DATASET_FILE);
    std::string line;
    int c=0;
    if (file.is_open()) {
        while (std::getline(file,line)) {
            if (line[0]=='1') {
                c++;
                if (c<file_r_low){continue;}
                if (c%1000==0){std::cout<<"games parsed: "<<c<<"|positions evaluated: "<<ds.size()<<"\n";}
                if (c>=file_r_high){break;}
                size_t start=0;
                while ((start=line.find('{',start))!=std::string::npos) {
                    size_t end=line.find('}',start);
                    if (end==std::string::npos) break; // no matching closing brace
                    line.erase(start, end-start+1);
                }
                start=0;
                while ((start=line.find('(',start))!=std::string::npos) {
                    size_t end=line.find(')',start);
                    if (end==std::string::npos) break; // no matching closing brace
                    line.erase(start, end-start+1);
                }
                if (line.find(')')!=std::string::npos||line.find('(')!=std::string::npos){continue;}
                std::stringstream ss(line);
                std::string chunk;
                std::vector<std::string> chunks;
                while (ss>>chunk) {chunks.push_back(chunk);}
                std::string result=chunks.back();
                float true_value=0.5;
                if (result=="1-0"){true_value=1.f;}
                if (result=="0-1"){true_value=0.f;}
                chunks.pop_back();
                for (int i=0;i<static_cast<int>(chunks.size());i++) {
                    if (chunks[i].back()=='.'){chunks.erase(chunks.begin()+i);}
                    if (chunks[i].front()=='$'){chunks.erase(chunks.begin()+i);}
                    if (std::isdigit(chunks[i].front())){chunks.erase(chunks.begin()+i);}
                }
                std::string best_move;
                auto position=libchess::Position("startpos");
                for (auto &raw:chunks) {
                    const auto moves=position.legal_moves();
                    if (raw.back()=='+'||raw.back()=='#'){raw.pop_back();}
                    if (raw.size()==2) {
                        for (const auto &move:moves) {
                            if (move.piece()==0&&raw==libchess::square_strings[sqr_idx(move.to())]) {
                                    best_move=static_cast<std::string>(move);
                                ds.push_back({position,true_value*side_multiplier[position.turn()]});
                            }
                        }
                    }
                    std::vector<std::string> from_sqrs;
                    if (std::isupper(raw.back())) {
                        char from=raw[0];
                        if (std::islower(raw[0])){from=raw[0];}
                        if (std::isdigit(from)) {
                            from_sqrs.push_back({'a',from});
                            from_sqrs.push_back({'b',from});
                            from_sqrs.push_back({'c',from});
                            from_sqrs.push_back({'d',from});
                            from_sqrs.push_back({'e',from});
                            from_sqrs.push_back({'f',from});
                            from_sqrs.push_back({'g',from});
                            from_sqrs.push_back({'h',from});
                        }
                        else {
                            from_sqrs.push_back({from,'1'});
                            from_sqrs.push_back({from,'2'});
                            from_sqrs.push_back({from,'3'});
                            from_sqrs.push_back({from,'4'});
                            from_sqrs.push_back({from,'5'});
                            from_sqrs.push_back({from,'6'});
                            from_sqrs.push_back({from,'7'});
                            from_sqrs.push_back({from,'8'});
                        }
                        if (raw.size()==7) {
                            from_sqrs.push_back({raw[0],raw[1]});
                        }
                        std::string to={raw[raw.size()-4],raw[raw.size()-3]};
                        for (const auto &move:moves) {
                            if (move.piece()==0&&to==libchess::square_strings[sqr_idx(move.to())]) {
                                char p=static_cast<char>(std::tolower(raw.back()));
                                if (p=='q'&&move.promotion()==libchess::Queen) {
                                    best_move=static_cast<std::string>(move);
                                    ds.push_back({position,true_value*side_multiplier[position.turn()]});

                                }
                                if (p=='r'&&move.promotion()==3) {
                                    best_move=static_cast<std::string>(move);
                                    ds.push_back({position,true_value*side_multiplier[position.turn()]});
                                }
                                if (p=='b'&&move.promotion()==2) {
                                    best_move=static_cast<std::string>(move);
                                    ds.push_back({position,true_value*side_multiplier[position.turn()]});
                                }
                                if (p=='n'&&move.promotion()==1) {
                                    best_move=static_cast<std::string>(move);
                                    ds.push_back({position,true_value*side_multiplier[position.turn()]});
                                }
                            }
                        }
                    }
                    if (raw.size()==4&&((std::isupper(raw[0])&&raw[1]!='x')||(std::islower(raw[0])&&raw[1]=='x'))) {
                        char from=raw[1];
                        if (std::islower(raw[0])){from=raw[0];}
                        if (std::isdigit(from)) {
                            from_sqrs.push_back({'a',from});
                            from_sqrs.push_back({'b',from});
                            from_sqrs.push_back({'c',from});
                            from_sqrs.push_back({'d',from});
                            from_sqrs.push_back({'e',from});
                            from_sqrs.push_back({'f',from});
                            from_sqrs.push_back({'g',from});
                            from_sqrs.push_back({'h',from});
                        }
                        else {
                            from_sqrs.push_back({from,'1'});
                            from_sqrs.push_back({from,'2'});
                            from_sqrs.push_back({from,'3'});
                            from_sqrs.push_back({from,'4'});
                            from_sqrs.push_back({from,'5'});
                            from_sqrs.push_back({from,'6'});
                            from_sqrs.push_back({from,'7'});
                            from_sqrs.push_back({from,'8'});
                        }
                    }
                    if (raw.size()==5) {
                        char from=raw[1];
                        if (std::islower(raw[0])){from=raw[0];}
                        if (std::isdigit(from)) {
                            from_sqrs.push_back({'a',from});
                            from_sqrs.push_back({'b',from});
                            from_sqrs.push_back({'c',from});
                            from_sqrs.push_back({'d',from});
                            from_sqrs.push_back({'e',from});
                            from_sqrs.push_back({'f',from});
                            from_sqrs.push_back({'g',from});
                            from_sqrs.push_back({'h',from});
                        }
                        else {
                            from_sqrs.push_back({from,'1'});
                            from_sqrs.push_back({from,'2'});
                            from_sqrs.push_back({from,'3'});
                            from_sqrs.push_back({from,'4'});
                            from_sqrs.push_back({from,'5'});
                            from_sqrs.push_back({from,'6'});
                            from_sqrs.push_back({from,'7'});
                            from_sqrs.push_back({from,'8'});
                        }
                    }
                    if (std::islower(raw.front())&&raw.size()>2) {
                        for (const auto &move:moves) {
                            std::string to={raw[raw.size()-2],raw[raw.size()-1]};
                            if (move.piece()==0&&to==libchess::square_strings[sqr_idx(move.to())]) {
                                if (!from_sqrs.empty()) {
                                    for (auto sqr:from_sqrs) {
                                        if (sqr==libchess::square_strings[sqr_idx(move.from())]) {
                                    best_move=static_cast<std::string>(move);
                                            ds.push_back({position,true_value*side_multiplier[position.turn()]});
                                        }
                                    }
                                }
                                else {
                                    best_move=static_cast<std::string>(move);
                                    ds.push_back({position,true_value*side_multiplier[position.turn()]});

                                }
                            }
                        }
                    }
                    if (raw.front()=='N') {
                        for (const auto &move:moves) {
                            std::string to={raw[raw.size()-2],raw[raw.size()-1]};
                            if (move.piece()==1&&to==libchess::square_strings[sqr_idx(move.to())]) {
                                if (!from_sqrs.empty()) {
                                    for (const auto& sqr:from_sqrs) {
                                        if (sqr==libchess::square_strings[sqr_idx(move.from())]) {
                                    best_move=static_cast<std::string>(move);
                                            ds.push_back({position,true_value*side_multiplier[position.turn()]});
                                        }
                                    }
                                }
                                else {
                                    best_move=static_cast<std::string>(move);
                                    ds.push_back({position,true_value*side_multiplier[position.turn()]});

                                }
                            }
                        }
                    }
                    if (raw.front()=='B') {
                        for (const auto &move:moves) {
                            std::string to={raw[raw.size()-2],raw[raw.size()-1]};
                            if (move.piece()==2&&to==libchess::square_strings[sqr_idx(move.to())]) {
                                if (!from_sqrs.empty()) {
                                    for (const auto& sqr:from_sqrs) {
                                        if (sqr==libchess::square_strings[sqr_idx(move.from())]) {
                                    best_move=static_cast<std::string>(move);
                                            ds.push_back({position,true_value*side_multiplier[position.turn()]});
                                        }
                                    }
                                }
                                else {
                                    best_move=static_cast<std::string>(move);
                                    ds.push_back({position,true_value*side_multiplier[position.turn()]});

                                }
                            }
                        }
                    }
                    if (raw.front()=='R') {
                        for (const auto &move:moves) {
                            std::string to={raw[raw.size()-2],raw[raw.size()-1]};
                            if (move.piece()==3&&to==libchess::square_strings[sqr_idx(move.to())]) {
                                if (!from_sqrs.empty()) {
                                    for (const auto& sqr:from_sqrs) {
                                        if (sqr==libchess::square_strings[sqr_idx(move.from())]) {
                                    best_move=static_cast<std::string>(move);
                                            ds.push_back({position,true_value*side_multiplier[position.turn()]});
                                        }
                                    }
                                }
                                else {
                                    best_move=static_cast<std::string>(move);
                                    ds.push_back({position,true_value*side_multiplier[position.turn()]});

                                }
                            }
                        }
                    }
                    if (raw.front()=='Q') {
                        for (const auto &move:moves) {
                            std::string to={raw[raw.size()-2],raw[raw.size()-1]};
                            if (move.piece()==4&&to==libchess::square_strings[sqr_idx(move.to())]) {
                                if (!from_sqrs.empty()) {
                                    for (const auto& sqr:from_sqrs) {
                                        if (sqr==libchess::square_strings[sqr_idx(move.from())]) {
                                    best_move=static_cast<std::string>(move);
                                            ds.push_back({position,true_value*side_multiplier[position.turn()]});
                                        }
                                    }
                                }
                                else {
                                    best_move=static_cast<std::string>(move);
                                    ds.push_back({position,true_value*side_multiplier[position.turn()]});

                                }
                            }
                        }
                    }
                    if (raw.front()=='K') {
                        for (const auto &move:moves) {
                            std::string to={raw[raw.size()-2],raw[raw.size()-1]};
                            if (move.piece()==5&&to==libchess::square_strings[sqr_idx(move.to())]) {
                                if (!from_sqrs.empty()) {
                                    for (const auto& sqr:from_sqrs) {
                                        if (sqr==libchess::square_strings[sqr_idx(move.from())]) {
                                    best_move=static_cast<std::string>(move);
                                            ds.push_back({position,true_value*side_multiplier[position.turn()]});
                                        }
                                    }
                                }
                                else {
                                    best_move=static_cast<std::string>(move);
                                    ds.push_back({position,true_value*side_multiplier[position.turn()]});

                                }
                            }
                        }
                    }
                    if (raw=="O-O"&&position.turn()==libchess::White) {
                        best_move="e1h1";
                        ds.push_back({position,true_value*side_multiplier[position.turn()]});
                    }
                    if (raw=="O-O-O"&&position.turn()==libchess::White) {
                        best_move="e1a1";
                        ds.push_back({position,true_value*side_multiplier[position.turn()]});
                    }
                    if (raw=="O-O"&&position.turn()==libchess::Black) {
                        best_move="e8h8";
                        ds.push_back({position,true_value*side_multiplier[position.turn()]});
                    }
                    if (raw=="O-O-O"&&position.turn()==libchess::Black) {
                        best_move="e8a8";
                        ds.push_back({position,true_value*side_multiplier[position.turn()]});
                    }
                    position.makemove(best_move);

                }
            }
        }
        file.close();
    }
    return ds;
}

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

inline float get_mse(const std::vector<Data> &dataset) {
    float mse=0.f;
    for (const auto &data:dataset) {
        std::vector<libchess::Move> mh;
        libchess::Position position=data.pos;
        mse+=powf(sig(static_cast<float>(quiescence(position,-BOUND,BOUND,0,mh).value))-data.true_score,2.f);
    }
    mse/=static_cast<float>(dataset.size());
    return mse;
}

/*
system:
-calculate current positions mse
-nudge eval in random direction
-calculate next mse and compare to last mse
-if its better, keep the nudge
-if its worse, flip the nudge
-re evaluate
-if its still worse, revert.
*/
inline void rand_tweak_tuner(const int samples=1000,int file_low=0,int file_high=20000) {
    auto dataset=gen_dataset(file_low,file_high);
    std::cout<<dataset.size()<<" total positions evaluated!\n";
    float mg_value_copy[6];
    float eg_value_copy[6];
    float knight_mobility_copy[9];
    float bishop_mobility_copy[27];
    float rook_mobility_copy[29];
    float queen_mobility_copy[28];
    float mg_pawn_table_copy[64];
    float eg_pawn_table_copy[64];
    float mg_knight_table_copy[64];
    float eg_knight_table_copy[64];
    float mg_bishop_table_copy[64];
    float eg_bishop_table_copy[64];
    float mg_rook_table_copy[64];
    float eg_rook_table_copy[64];
    float mg_queen_table_copy[64];
    float eg_queen_table_copy[64];
    float mg_king_table_copy[64];
    float eg_king_table_copy[64];
    float last_mse=0.f;
    float mg_value_delta[6];
    float eg_value_delta[6];
    float knight_mobility_delta[9];
    float bishop_mobility_delta[27];
    float rook_mobility_delta[29];
    float queen_mobility_delta[28];
    float mg_pawn_table_delta[64];
    float eg_pawn_table_delta[64];
    float mg_knight_table_delta[64];
    float eg_knight_table_delta[64];
    float mg_bishop_table_delta[64];
    float eg_bishop_table_delta[64];
    float mg_rook_table_delta[64];
    float eg_rook_table_delta[64];
    float mg_queen_table_delta[64];
    float eg_queen_table_delta[64];
    float mg_king_table_delta[64];
    float eg_king_table_delta[64];
    for (int n=0;n<samples;n++) {
        float mse=get_mse(dataset);
        if (n!=0&&mse>last_mse) {
            std::ranges::copy(mg_value_copy, mg_value);
            std::ranges::copy(eg_value_copy, eg_value);
            std::copy(std::begin(knight_mobility_copy), std::end(knight_mobility_copy), knight_mobility);
            std::copy(std::begin(bishop_mobility_copy), std::end(bishop_mobility_copy), bishop_mobility);
            std::copy(std::begin(rook_mobility_copy), std::end(rook_mobility_copy), rook_mobility);
            std::copy(std::begin(queen_mobility_copy), std::end(queen_mobility_copy), queen_mobility);
            std::copy(std::begin(mg_pawn_table_copy), std::end(mg_pawn_table_copy), mg_pawn_table);
            std::copy(std::begin(eg_pawn_table_copy), std::end(eg_pawn_table_copy), eg_pawn_table);
            std::copy(std::begin(mg_knight_table_copy), std::end(mg_knight_table_copy), mg_knight_table);
            std::copy(std::begin(eg_knight_table_copy), std::end(eg_knight_table_copy), eg_knight_table);
            std::copy(std::begin(mg_bishop_table_copy), std::end(mg_bishop_table_copy), mg_bishop_table);
            std::copy(std::begin(eg_bishop_table_copy), std::end(eg_bishop_table_copy), eg_bishop_table);
            std::copy(std::begin(mg_rook_table_copy), std::end(mg_rook_table_copy), mg_rook_table);
            std::copy(std::begin(eg_rook_table_copy), std::end(eg_rook_table_copy), eg_rook_table);
            std::copy(std::begin(mg_queen_table_copy), std::end(mg_queen_table_copy), mg_queen_table);
            std::copy(std::begin(eg_queen_table_copy), std::end(eg_queen_table_copy), eg_queen_table);
            std::copy(std::begin(mg_king_table_copy), std::end(mg_king_table_copy), mg_king_table);
            std::copy(std::begin(eg_king_table_copy), std::end(eg_king_table_copy), eg_king_table);
            n-=1;
            continue;
        }
        if (last_mse!=mse) {std::cout<<"loss: "<<mse<<"\n";}
        last_mse=mse;
        std::ranges::copy(mg_value, mg_value_copy);
        std::ranges::copy(eg_value, eg_value_copy);
        std::ranges::copy(knight_mobility, knight_mobility_copy);
        std::ranges::copy(bishop_mobility, bishop_mobility_copy);
        std::ranges::copy(rook_mobility, rook_mobility_copy);
        std::ranges::copy(queen_mobility, queen_mobility_copy);
        std::copy(std::begin(mg_pawn_table), std::end(mg_pawn_table), mg_pawn_table_copy);
        std::copy(std::begin(eg_pawn_table), std::end(eg_pawn_table), eg_pawn_table_copy);
        std::copy(std::begin(mg_knight_table), std::end(mg_knight_table), mg_knight_table_copy);
        std::copy(std::begin(eg_knight_table), std::end(eg_knight_table), eg_knight_table_copy);
        std::copy(std::begin(mg_bishop_table), std::end(mg_bishop_table), mg_bishop_table_copy);
        std::copy(std::begin(eg_bishop_table), std::end(eg_bishop_table), eg_bishop_table_copy);
        std::copy(std::begin(mg_rook_table), std::end(mg_rook_table), mg_rook_table_copy);
        std::copy(std::begin(eg_rook_table), std::end(eg_rook_table), eg_rook_table_copy);
        std::copy(std::begin(mg_queen_table), std::end(mg_queen_table), mg_queen_table_copy);
        std::copy(std::begin(eg_queen_table), std::end(eg_queen_table), eg_queen_table_copy);
        std::copy(std::begin(mg_king_table), std::end(mg_king_table), mg_king_table_copy);
        std::copy(std::begin(eg_king_table), std::end(eg_king_table), eg_king_table_copy);
        // Fill the deltas
        for (int i = 0; i < 6; i++) {
            mg_value_delta[i] = real_dist(gen);
            eg_value_delta[i] = real_dist(gen);
        }
        for (int i = 0; i < 9; i++) knight_mobility_delta[i] = real_dist(gen);
        for (int i = 0; i < 27; i++) bishop_mobility_delta[i] = real_dist(gen);
        for (int i = 0; i < 29; i++) rook_mobility_delta[i] = real_dist(gen);
        for (int i = 0; i < 28; i++) queen_mobility_delta[i] = real_dist(gen);
        for (int i = 0; i < 64; i++) {
            mg_pawn_table_delta[i] = real_dist(gen);
            eg_pawn_table_delta[i] = real_dist(gen);
            mg_knight_table_delta[i] = real_dist(gen);
            eg_knight_table_delta[i] = real_dist(gen);
            mg_bishop_table_delta[i] = real_dist(gen);
            eg_bishop_table_delta[i] = real_dist(gen);
            mg_rook_table_delta[i] = real_dist(gen);
            eg_rook_table_delta[i] = real_dist(gen);
            mg_queen_table_delta[i] = real_dist(gen);
            eg_queen_table_delta[i] = real_dist(gen);
            mg_king_table_delta[i] = real_dist(gen);
            eg_king_table_delta[i] = real_dist(gen);
        }
        // Apply the deltas to the original tables
        for (int i = 0; i < 6; i++) {
            mg_value[i] += mg_value_delta[i];
            eg_value[i] += eg_value_delta[i];
        }
        for (int i = 0; i < 9; i++) knight_mobility[i] += knight_mobility_delta[i];
        for (int i = 0; i < 27; i++) bishop_mobility[i] += bishop_mobility_delta[i];
        for (int i = 0; i < 29; i++) rook_mobility[i] += rook_mobility_delta[i];
        for (int i = 0; i < 28; i++) queen_mobility[i] += queen_mobility_delta[i];
        for (int i = 0; i < 64; i++) {
            mg_pawn_table[i] += mg_pawn_table_delta[i];
            eg_pawn_table[i] += eg_pawn_table_delta[i];
            mg_knight_table[i] += mg_knight_table_delta[i];
            eg_knight_table[i] += eg_knight_table_delta[i];
            mg_bishop_table[i] += mg_bishop_table_delta[i];
            eg_bishop_table[i] += eg_bishop_table_delta[i];
            mg_rook_table[i] += mg_rook_table_delta[i];
            eg_rook_table[i] += eg_rook_table_delta[i];
            mg_queen_table[i] += mg_queen_table_delta[i];
            eg_queen_table[i] += eg_queen_table_delta[i];
            mg_king_table[i] += mg_king_table_delta[i];
            eg_king_table[i] += eg_king_table_delta[i];
        }
        if (n%100==0&&n!=0) {
            const int low=file_low;
            file_low=file_high;
            file_high=file_high+(file_high-low);
            dataset=gen_dataset(file_low,file_high);
            last_mse=get_mse(dataset);
            display_params();
        }
    }
    display_params();
}

/*
system:
maybe:
-randomly set parameters
-do single param tuning 1 pass for randomly set params
-copy as best is better than original
-repeat

*/
inline float best_param(float *param,const std::vector<Data> &dataset) {
    float bound=2000.f; //i dont think a single parameter can be more than 2000 generously.
    float step_size=500.f; //seems optimal
    float best_param=0.f;
    while (step_size>0.01f) {
        float best_step_param=0.f;
        float best_mse=99999.f;
        for (float i=best_param-bound;i<best_param+bound;i+=step_size) {
            *param=i;
            const float step_mse=get_mse(dataset);
            if (step_mse<best_mse) {
                best_step_param=i;
                best_mse=step_mse;
            }
        }
        best_param=best_step_param;
        bound=step_size;
        step_size/=10;
    }
    std::cout<<best_param<<"\n";
    return best_param;
}



#endif //CHESSBOT_EVAL_TUNER_H