CFLAGS := -Wall -std=c++20 -O3
CC := clang++
OBJS := ./src/*.cpp
INCLUDE := -I/usr/local/include -I../include
LIB := -L../lib

all: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) $(LIB) $(OBJS) -o cftp 

clean:
	rm *.o cftp
