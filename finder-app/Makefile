all: writer

writer.o: writer.c
	$(CROSS_COMPILE)gcc -g -Wall -c writer.c

writer: writer.o
	$(CROSS_COMIPLE)gcc -g -Wall writer.o -o writer

clean:
	rm -f writer.o writer


