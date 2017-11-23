#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "bidding_system.h"


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

void hostPool_init(struct HostPool *hostPool, int host_num) {
	hostPool->hostCount = host_num;
	FD_ZERO(&hostPool->read_pipes);
	hostPool->maxfd = 0;
	for (int i = 0; i < host_num; i++) {
		hostPool->available = hostPool->available | ((uint16_t)1 << i);
		hostPool_forkHost(i, hostPool->host[i].pipefd);
		FD_SET(hostPool->host[i].pipefd[PIPE_RD], &hostPool->read_pipes);
		if (hostPool->host[i].pipefd[PIPE_RD] > hostPool->maxfd) {
			hostPool->maxfd = hostPool->host[i].pipefd[PIPE_RD];
		}
	}
}

void hostPool_forkHost(int index, int pipefd[]) {
	pipe(pipefd);
	if (fork() == 0) {
		// child
		char host_id[3];
		sprintf(host_id, "%d", index+1);
		dup2(pipefd[PIPE_RD], STDIN_FILENO);
		dup2(pipefd[PIPE_WR], STDOUT_FILENO);
		execlp("host", "host", host_id, NULL);
	}
}

int hostPool_checkAvailable(struct HostPool *hostPool) {
	return (hostPool->available > 0)? 1:0;
}

void hostPool_receiveTask(struct HostPool *hostPool, int task[]) {
	char buffer[16];
	sprintf(buffer, "%d %d %d %d\n", task[0], task[1], task[2], task[3]);

	uint16_t probe = 1;
	for (int i = 0; i < hostPool->hostCount; i++, probe = probe << 1) {
		if ((hostPool->available & probe) > 0) {
			write(hostPool->host[i].pipefd[PIPE_WR], buffer, strlen(buffer)+1);
			return;
		}
	}

	perror("hostPool: no available host to receiveTask");
	exit(1);
}

int hostPool_checkAllDone(struct HostPool *hostPool) {
	uint16_t probe = 1;
	for (int i = 0; i < hostPool->hostCount; i++, probe = probe << 1) {
		if ((hostPool->available && probe) == 0)
			return 0;
	}
	return 1;
}

void hostPool_cleanUp(struct HostPool *hostPool) {
	char buffer[] = "-1 -1 -1 -1\n";
	for (int i = 0; i < hostPool->hostCount; i++) {
		write(hostPool->host[i].pipefd[PIPE_RD], buffer, sizeof(buffer));
	}
}

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
		printf("%d\t%d\t%d\n", i+1, scoreBoard->player[i].rank, scoreBoard->player[i].score);
	}
}
