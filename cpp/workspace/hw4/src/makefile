CC=gcc
maindir=$(shell pwd)
mainc=$(shell basename $(main) |sed 's/\.cpp//g')

all:
	$(CC) -o $(mainc).o $(mainc).cpp

run:
	$(CC) -o $(mainc).o -g $(mainc).cpp && $(maindir)/$(mainc).o

debug:
	$(CC) -o $(mainc).o -g $(mainc).cpp &&gdb $(maindir)/$(mainc).o

clean:
	@rm *.o *~
