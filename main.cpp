#include <iostream>
#include "uci.h"
#include "eval_tuner.h"
#include "move_ordering_debugging.h"
#include "tuner.h"


int main() {
    pos=libchess::Position("startpos");
    //auto result=root(pos,1000,MAX_DEPTH,true);
    tune(50);



    //move_ordering_testing();

    uci();
    return 0;
}
