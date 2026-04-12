#ifndef CHESSBOT_RANDOM_H
#define CHESSBOT_RANDOM_H
#include <random>

inline std::mt19937 gen(std::random_device{}());
inline std::uniform_real_distribution real_dist(-1.f,1.f);
inline float random_sign() {
    if (real_dist(gen)>.5){return 1.f;}
    return -1.f;
}


#endif //CHESSBOT_RANDOM_H