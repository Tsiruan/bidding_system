#include <stdint.h>
#include <sys/select.h>

#define PIPE_RD 0
#define PIPE_WR 1

struct Host {
	int pipefd[2];
};

struct HostPool {
	int hostCount;
	uint16_t available;
	struct Host host[12];
	fd_set read_pipes;
	int maxfd;
};

struct Player {
	int score;
	int rank;
};

struct ScoreBoard {
	int playerCount;
	struct Player player[20];
};

void system_argParse(int argc, char const *argv[], int *host_num, int *player_num);
int  system_combine(int task[], const int player_num);
void system_updateScore(struct HostPool *hostPool, struct ScoreBoard *scoreBoard);
void hostPool_init(struct HostPool *hostPool, int host_num);
void hostPool_forkHost(int index, int pipefd[]);
int  hostPool_checkAvailable(struct HostPool *hostPool);
void hostPool_receiveTask(struct HostPool *hostPool, int task[]);
int  hostPool_checkAllDone(struct HostPool *hostPool);
void hostPool_cleanUp(struct HostPool *hostPool);
void scoreBoard_init(struct ScoreBoard *scoreBoard, int player_num);
void scoreBoard_pushScore(struct ScoreBoard *scoreBoard, int playerId, int rank);
void scoreBoard_rankPlayer(struct ScoreBoard *scoreBoard);
void scoreBoard_print(struct ScoreBoard *scoreBoard);