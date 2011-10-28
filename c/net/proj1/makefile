CC=gcc
maindir=`pwd`
mainc=`basename $(main) |sed 's/.c//g'`

all:
	$(CC) -o $(mainc).o $(mainc).c

run:
	$(CC) -o $(mainc).o -g $(mainc).c && $(maindir)/$(mainc).o

debug:
	$(CC) -o $(mainc).o -g $(mainc).c &&gdb $(maindir)/$(mainc).o

clean:
	@rm *.o *~
