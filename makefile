
objects=config.o error.o log.o memory.o flsinit.o \
	accque.o net.o flsmain.o main.o signal.o sync.o limit.o

fls:$(objects)
	cc -Wall -o fls $(objects) -lm
	cp fls ../bin/fls

$(objects):fls.h

.PHONY:clean
clean:
	rm -f $(objects) fls

