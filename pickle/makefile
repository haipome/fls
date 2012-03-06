
obj=main.o config.o name.o dumper.o reload.o

pickle:$(obj)
	gcc -Wall -o pickle $(obj)
	cp pickle ../../bin/pickle

$(obj):pickle.h

.PHONY:clean
clean:
	rm -f $(obj) pickle

