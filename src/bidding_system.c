#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "bidding_system.h"

#ifdef DEBUG
#include "debug/debug_bidding_system.h"
#endif


int main(int argc, char const *argv[]) {
	int host_num, player_num;
	struct HostPool hostPool;
	struct ScoreBoard scoreBoard;

	system_argParse(argc, argv, &host_num, &player_num);

	hostPool_init(&hostPool, host_num);
	scoreBoard_init(&scoreBoard, player_num);

	int task[4] = {1, 2, 3, 4};
	do {
		if (!hostPool_checkAvailable(&hostPool)) {
			system_updateScore(&hostPool, &scoreBoard);
		}
		hostPool_receiveTask(&hostPool, task);
	} while (system_combine(task, player_num)) ;

	while (!hostPool_checkAllDone(&hostPool)) {
		system_updateScore(&hostPool, &scoreBoard);
	}

	hostPool_cleanUp(&hostPool);
	scoreBoard_rankPlayer(&scoreBoard);
	scoreBoard_print(&scoreBoard);

	return 0;
}


void system_argParse(int argc, char const *argv[], int *host_num, int *player_num) {
	if (argc != 3) {
		perror("[invalid arguments]\nUsage: ./bidding_system [host number] [player number]\n");
		exit(0);
	}

	*host_num = atoi(argv[1]);
	*player_num = atoi(argv[2]);

	if (*host_num < 0 || *host_num > 12) {
		perror("[invalid arguments] host number shall be an interger between 1 to 12\n");
		exit(0);
	}
	if (*player_num < 4 || *player_num > 20) {
		perror("[invalid arguments] player number shall be an interger between 4 to 20\n");
		exit(0);
	}
}

int system_combine(int task[], const int player_num) {
	for(int i = 3; i >= 0; i--) {
		task[i]++;
		for(int j = i+1; j < 4; j++) {
			task[j] = task[j-1]+1;
		}
		if (task[3] <= player_num) return 1;
	}
	return 0;
}

void system_updateScore(struct HostPool *hostPool, struct ScoreBoard *scoreBoard) {
	char buffer[32];
	int playerId, rank;

	fd_set working_fdset;
	memcpy(&working_fdset, &hostPool->read_pipes, sizeof(fd_set));

	select(hostPool->maxfd+1, &working_fdset, NULL, NULL, NULL);

	for (int i = 0; i < hostPool->hostCount; i++) {
		if (FD_ISSET(hostPool->host[i].pipefd[PIPE_RD], &working_fdset)) {
			read(hostPool->host[i].pipefd[PIPE_RD], buffer, 32);
			for (int j = 0; j < 4; j++) {
				sscanf(buffer + j*4, "%d %d\n", &playerId, &rank);
				scoreBoard_pushScore(scoreBoard, playerId, rank);
			}
		}
	}
}
