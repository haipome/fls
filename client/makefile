
client:clientest.c flsclient.h flsc.o
	cc -Wall -o client clientest.c flsc.o
	cp client ../../tools/client

flsc.o:flsclient.c flsc.h
	cc -Wall -c -o flsc.o flsclient.c

.PHONY:clean
clean:
	rm -f client flsc.o

