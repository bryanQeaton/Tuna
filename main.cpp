#include <iostream>
#include "uci.h"
#include "eval_tuner.h"
#include "iccplayer.h"
#include "move_ordering_debugging.h"
#include "tuner.h"


int main() {
    pos=libchess::Position("startpos");
    //auto result=root(pos,100,MAX_DEPTH,true);
    //std::cout<<result.depth;
    //move_ordering_testing();
    //tune(5);



    uci();
    return 0;
}
