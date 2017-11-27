#include "bidding_system.h"
#include <stdio.h>

void _debug_scoreBoard_print(struct ScoreBoard *scoreBoard) {
	printf("id\tscore\trank\n");
	for(int i = 0; i < scoreBoard->playerCount; i++) {
		printf("%d\t%d\t%d\n", i+1, scoreBoard->player[i].rank, scoreBoard->player[i].score);
	}
}