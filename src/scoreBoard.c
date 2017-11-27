#include <stdio.h>
#include "bidding_system.h"

void scoreBoard_init(struct ScoreBoard *scoreBoard, int player_num) {
	scoreBoard->playerCount = player_num;
	for(int i = 0; i < player_num; i++) {
		scoreBoard->player[i].score = 0;
	}
}

void scoreBoard_pushScore(struct ScoreBoard *scoreBoard, int playerId, int rank) {
	int rtos[5] = {0, 3, 2, 1, 0};	// rank to score
	scoreBoard->player[playerId-1].score += rtos[rank];
}

void scoreBoard_rankPlayer(struct ScoreBoard *scoreBoard) {
	struct Player *ranking[20];
	for (int i = 0; i < 20; i++) {
		ranking[i] = &scoreBoard->player[i];
	}

	struct Player *temp;
	for (int i = 0; i < scoreBoard->playerCount; i++)
		for (int j = scoreBoard->playerCount-1; j > i; j--) {
			if (ranking[j]->score > ranking[j-1]->score) {
				temp = ranking[j];
				ranking[j] = ranking[j-1];
				ranking[j-1] = temp;
			}
		}

	int rank;
	for (int i = 0; i < scoreBoard->playerCount; i++) {
		if (i == 0) {
			rank = 1;
		} else if (ranking[i]->score == ranking[i-1]->score) {
			// rank remains the same
		} else {
			rank = i+1;
		}
		ranking[i]->rank = rank;
	}
}

void scoreBoard_print(struct ScoreBoard *scoreBoard) {
	for(int i = 0; i < scoreBoard->playerCount; i++) {
		printf("%d %d\n", i+1, scoreBoard->player[i].rank);
	}
}
