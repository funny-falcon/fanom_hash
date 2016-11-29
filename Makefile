CC= g++
test_fanom: test_fanom.cpp fanom_hash.h fanom_hash32.h Makefile
	$(CC) -O3 -ggdb3 test_fanom.cpp -o test_fanom
