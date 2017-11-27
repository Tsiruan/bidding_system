#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "bidding_system.h"


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
