CC = g++
CXXFLAGS = -g

all: sim

sim: main.o matchmaker.o rtree.o
	$(CC) $(CXXFLAGS) main.o matchmaker.o rtree.o -o sim -lpthread

main.o: src/main.cc
	$(CC) $(CXXFLAGS) src/main.cc -c -lpthread

matchmaker.o: src/matchmaker.cc
	$(CC) $(CXXFLAGS) src/matchmaker.cc -c -lpthread

rtree.o: src/rtree.cc
	$(CC) $(CXXFLAGS) src/rtree.cc -c -lpthread

.PHONY: clean

clean:
	rm *.o sim
