#include <iostream>
#include "uci.h"
#include "eval_tuner.h"
#include "move_ordering_debugging.h"
#include "tuner.h"


int main() {
    pos=libchess::Position("startpos");
    //auto result=root(pos,100,MAX_DEPTH,true);
    //std::cout<<result.depth;
    //tune(5);



    //move_ordering_testing();

    uci();
    return 0;
}
