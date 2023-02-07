CC=aarch64-none-linux-gnu-gcc
#CC=gcc
CFLAGS= -g -Wall
FL=writer writer.c

all: writer

writer: writer.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(FL) $(LDFLAGS)

clean:
	rm -f writer *.o


