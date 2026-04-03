#ifndef CHESSBOT_UCI_H
#define CHESSBOT_UCI_H
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include "root.h"
#include "libchess/position.hpp"

inline std::atomic<bool> running=true;
inline std::atomic<int> wtime=0;
inline std::atomic<int> btime=0;
inline std::atomic<int> winc=0;
inline std::atomic<int> binc=0;
inline std::atomic<int> movetime=0;
inline libchess::Position pos("startpos");
inline std::chrono::high_resolution_clock::time_point start_time;
inline std::atomic<int> time_limit=0;
inline void watch_timer(){
    while(running){
        if(search>0&&time_limit){
            if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start_time).count()>=time_limit) {
                search=-1;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
inline void watch_input() {
    std::string line;
    while (running && std::getline(std::cin,line)) {
        if (search&&time_limit!=0) {
            auto time_stop=std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(time_stop-start_time).count()>=time_limit) {
                search=-1;
            }
        }
        try {
            if (line=="quit") {running=false; }
            else if (line=="uci") {
                std::cout<<"id name ChessBot\nid author BryanEquinn\nuciok\n"<<std::flush;
            }
            else if (line=="ucinewgame") {
                // reset TT or other state if needed
                tt.clear();
                // for (int n=0;n<3;n++) {for (int m=0;m<MAX_DEPTH;m++) {killer_moves[n][m]=libchess::Move();}}
                // for (int n=0;n<6;n++) {for (int m=0;m<64;m++) {countermove_history[n][m]=libchess::Move();}}
                // for (int n=0;n<6;n++) {for (int m=0;m<64;m++) {followup_history[n][m]=libchess::Move();}}
                for (int n=0;n<64;n++) {for (int m=0;m<64;m++) {history[n][m]*=.9;}}
            }
            else if (line=="isready") {
                std::cout<<"readyok\n"<<std::flush;
            }
            else if (line.starts_with("position")) {
                std::vector<std::string> words;
                std::stringstream ss(line);
                std::string word;
                while (ss>>word) words.push_back(word);
                if (words[1]=="startpos") {
                    pos.set_fen("startpos");
                } else if (words[1]=="fen") {
                    std::string fen;
                    size_t i=2;
                    while (i<words.size()&&words[i]!="moves") {
                        fen+=words[i]+" ";
                        i++;
                    }
                    pos.set_fen(fen);
                }
                auto it=std::ranges::find(words,"moves");
                if (it!=words.end()) {
                    for (++it;it!=words.end();++it) pos.makemove(*it);
                }
            }
            else if (line.starts_with("go")) {
                std::vector<std::string> words;
                std::stringstream ss(line);
                std::string word;
                while (ss>>word) words.push_back(word);
                movetime=0;
                for (size_t i=1;i+1<words.size();i++) {
                    if (words[i]=="wtime") wtime=std::stoi(words[i+1]);
                    else if (words[i]=="btime") btime=std::stoi(words[i+1]);
                    else if (words[i]=="winc") winc=std::stoi(words[i+1]);
                    else if (words[i]=="binc") binc=std::stoi(words[i+1]);
                    else if (words[i]=="movetime") movetime=std::stoi(words[i+1]);
                }
                start_time=std::chrono::high_resolution_clock::now();
                search=1;
            }
            else if (line=="stop") {search=-1;}
        }
        catch (...) {continue;}
    }
}

inline void uci(const int &depth_limit=MAX_DEPTH) {
    std::thread t(watch_input);
    std::thread l(watch_timer);
    while (running) {
        if (search==1) {
            //time management goes here
            if (movetime==0) {
                int time_left=wtime;
                int inc=winc;
                if (pos.turn()==libchess::Black) {
                    time_left=btime;
                    inc=binc;
                }
                time_limit=(time_left/20)+(inc/2);
            }
            else {
                time_limit=movetime.load();
            }
            auto local_pos =pos;
            auto ret=root(local_pos,time_limit,depth_limit);
            std::cout<<"info depth "<<ret.depth<<" score cp "<<ret.score<<" nodes "<<ret.nodes<<" nps "<<(static_cast<float>(ret.nodes)/static_cast<float>(time_limit))*1000.f<<" ";
            for (const auto &move:ret.pv) {std::cout<<move<<" ";}
            std::cout<<"\n"<<std::flush;;
            std::cout<<"bestmove "<<ret.best_move<<"\n"<<std::flush;
            search=0;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (t.joinable()) t.join();
}

#endif // CHESSBOT_UCI_H
