CFLAGS = -Wall -g
CC	= gcc $(CFLAGS)

commando: cmd.o cmdcol.o commando.o util.o
	$(CC) -o commando cmdcol.o commando.o util.o cmd.o

commando.o: commando.c commando.h
	$(CC) -c commando.c

cmd.o: cmd.c commando.h

cmdcol.o: cmdcol.c commando.h

clean:
	rm -f commando*.o

include test_Makefile
