CC=gcc
maindir=$(shell pwd)
mainc=$(shell basename $(main) |sed 's/.c//g')

all:
	$(CC) -o $(mainc).o $(mainc).c

run:
	$(CC) -o $(mainc).o -g $(mainc).c && $(maindir)/$(mainc).o

clean:
	@rm *.o *~
