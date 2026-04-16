# Tuna: a weak chess engine
built on Libchess

### Features:
- -negamax a-b
- -transposition tables
- -principal variation search
- -iterative deepening
- -null move pruning
- -late move reductions
- -reverse futility pruning
- -quiesence with see<0 pruning
- -odd forward pruning method
### Roadmap:
- -**eval_tuner.h** : broken, don't bother using it
- -**evaluation.h** : evaluation function
- -**insufficient_material.h** : function to determine if theres insufficient material for a checkmate
- -**move_ordering_debugging.h** : measures the avg  move/number_moves where move caused a cutoff as well as the search speed and total nodes across 40 positions
- -**params.h** : parameters for the engine such as eval params and bound/abort values
- -**perft.h** : a simple perft function
- -**random.h** : handles all random functions
- -**root.h** : handles search at the root, when an abort value is recieved, the best move of the last iteration of iterative deepening is used
- -**search.h** : when the atomic search variable is changed, the search aborts with the abort value
- -**tt.h** : a simple tt implementation
- -**uci.h** :handles basic uci compatibility, this was vibe coded for the most part. a seperate thread handles the time controls to ensure moves are always returned on time.






```


Made with <https://dillinger.io/>
