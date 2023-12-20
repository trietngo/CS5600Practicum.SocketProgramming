all: rfs server

rfs: client.o clientHelper.o
	gcc client.o clientHelper.o -o rfs

server: server.o serverHelper.o
	gcc server.o serverHelper.o -o server -lpthread

client.o: client.c
	gcc -c client.c

clientHelper.o: clientHelper.c clientHelper.h
	gcc -c clientHelper.c

server.o: server.c
	gcc -c server.c -lpthread

serverHelper.o: serverHelper.c serverHelper.h
	gcc -c serverHelper.c -lpthread

clean:
	rm -rf *.o server rfs remote localRepo