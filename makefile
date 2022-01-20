all:		simulator.o generator.o customer.o stop.o
		gcc -o simulator simulator.o -lX11 -lpthread
		gcc -o generator generator.o
		gcc -o customer customer.o
		gcc -o stop stop.o

simulator.o:	simulator.c display.c dispatchCenter.c taxi.c simulator.h
		gcc -c simulator.c

generator.o:	generator.c simulator.h
		gcc -c generator.c

customer.o: 	customer.c simulator.h
		gcc -c customer.c

stop.o:		stop.c simulator.h
		gcc -c stop.c

clean:
		rm -f simulator.o generator.o customer.o stop.o simulator taxi generator customer stop
