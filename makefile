.PHONY: sample

# arguments passed to bidding_system: host_num, player_num
-h = 4
-p = 6

BINPATH = ./bin
SRCPATH = ./src
SAMPATH = ./sample
TESTPATH = ./test

CFLAGS = -std=c11 -Wall

# run sample binary from TA
sample:
	$(SAMPATH)/bidding_system $(-h) $(-p)
	find . -name '$(SAMPATH)*.FIFO' -print0 | xargs -0 rm -f


# test every part of the program with TA's binary
test: init compile cpnrun clear

init:
	mkdir -p $(TESTPATH)

compile: bidding_system

cpnrun:
	cp -t $(TESTPATH)/ $(BINPATH)/bidding_system
	cp -t $(TESTPATH)/ $(SAMPATH)/host $(SAMPATH)/player
	$(TESTPATH)/bidding_system $(-h) $(-p)

clear:
	rm -r -f $(TESTPATH)

bidding_system: $(SRCPATH)/bidding_system.c $(SRCPATH)/bidding_system.h
	gcc $(CFLAGS) $(SRCPATH)/bidding_system.c -o $(BINPATH)/bidding_system
