.PHONY: sample

# arguments passed to bidding_system: host_num, player_num
host = 4
player = 6

BINPATH = ./bin
SRCPATH = ./src
INCPATH = ./include
DEBPATH = ./src/debug
SAMPATH = ./sample
TESTPATH = ./test

BIN = bidding_system

OBJ_bidding_system = bidding_system.o hostPool.o scoreBoard.o
OBJ_host = host.o
OBJ_player = player.o

# debug flag for gcc
DDEBUG = -DDEBUG -g
CFLAGS = -std=c11 -Wall -Iinclude


# fast command
all: debug

# duplicate from project folder to workstation
push:
	scp -i ~/.ssh/id_rsa -r . b03902079@linux7.csie.ntu.edu.tw:~/SP/hw2/bidding_system

pull:
	scp -l 8192 -r tsiruan@140.112.249.109:/mnt/e/ntu/106-1/System\ Programming/hw2/bidding_system .


# run sample binary from TA
sample:
	$(SAMPATH)/bidding_system $(host) $(player)
	find . -name '$(SAMPATH)*.FIFO' -print0 | xargs -0 rm -f

# overrides functions to print debug message
# modifications to project source is avoided
debug: CFLAGS += $(DDEBUG)
debug: $(eval OBJ_bidding_system += debug_bidding_system.o)
debug: test


# test every part of the program
test: init compile cpnrun clear

init:
	@mkdir -p $(TESTPATH) $(BINPATH)

compile: $(BIN)

cpnrun:
	@echo setting up testing environment
	@cp -t $(TESTPATH) $(BINPATH)/bidding_system
	@cp -t $(TESTPATH) $(SAMPATH)/host $(SAMPATH)/player
	@echo ==================================
	@echo ./bidding_system $(host) $(player)
	@$(TESTPATH)/bidding_system $(host) $(player)

clear:
	@rm -r -f $(TESTPATH) $(BINPATH)


## compiling ##
.SECONDEXPANSION:

# link .o files
$(BIN): $$(addprefix $(BINPATH)/, $$(OBJ_$$@))
	@echo [linking] $^
	@gcc $^ -o $(BINPATH)/$@

# compile .c files
$(BINPATH)/%.o: $(SRCPATH)/%.c $(INCPATH)/*.h
	@echo [compiling] $<
	@gcc $(CFLAGS) $< -c -o $@

# compile .c files for debugging
$(BINPATH)/debug_%.o: $(DEBPATH)/debug_%.c $(DEBPATH)/debug_%.h
	@echo [compiling] $<
	@gcc $(CFLAGS) $< -c -o $@
