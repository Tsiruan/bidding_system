test: test.c
	gcc -std=c11 test.c -o test
	./test

sys: bidding_system.c bidding_system.h
	gcc -std=c11 bidding_system.c -o bidding_system

rm:
	rm -f test bidding_system