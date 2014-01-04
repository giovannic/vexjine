include ../../Makefile.inc

CFLAGS+=-I../ -I../tools/ -I../network/

all: MM1 SupermarketSim

MM1: MM1Sim.cpp MM1Sim.h
	$(CC) $(CFLAGS) -o MM1 MM1Sim.cpp -L../ -lcinqs

SupermarketSim: Supermarket.cpp Supermarket.h
	$(CC) $(CFLAGS) -o SupermarketSim Supermarket.cpp -L../ -lcinqs

clean:
	rm -f MM1 SupermarketSim
