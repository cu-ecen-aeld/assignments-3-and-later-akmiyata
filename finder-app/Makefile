GCC=$(CROSS_COMPILE)gcc
CFLAGS= -g -Wall
FL=writer

all: writer

writer: writer.c
	arch64-none-linux-gnu-$(GCC) $(CFLAGS) -o writer writer.c

clean:
	rm -f writer *.o


