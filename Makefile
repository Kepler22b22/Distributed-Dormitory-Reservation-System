CC = gcc
CFLAGS = -Wall
OBJ = serverA.o serverB.o serverC.o servermain.o client.o
EXECUTABLES = serverA serverB serverC servermain client

all: $(EXECUTABLES)

serverA.o: serverA.c
	$(CC) $(CFLAGS) -c serverA.c -o serverA.o

serverB.o: serverB.c 
	$(CC) $(CFLAGS) -c serverB.c -o serverB.o

serverC.o: serverC.c 
	$(CC) $(CFLAGS) -c serverC.c -o serverC.o

servermain.o: servermain.c
	$(CC) $(CFLAGS) -c servermain.c -o servermain.o

client.o: client.c
	$(CC) $(CFLAGS) -c client.c -o client.o

serverA: serverA.o
	$(CC) $(CFLAGS) -o serverA serverA.o

serverB: serverB.o
	$(CC) $(CFLAGS) -o serverB serverB.o

serverC: serverC.o
	$(CC) $(CFLAGS) -o serverC serverC.o

servermain: servermain.o
	$(CC) $(CFLAGS) -o servermain servermain.o

client: client.o
	$(CC) $(CFLAGS) -o client client.o

clean:
	rm -f $(EXECUTABLES) *.o
