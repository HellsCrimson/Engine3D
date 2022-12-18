CC=g++
SRC=$(shell find src -name "*.cpp")
OBJ=$(SRC:.cpp=.o)
CFLAGS=-std=c++17
LFLAGS=-lX11 -lGL -lpthread -lpng -lstdc++fs

all: engine

engine: $(OBJ)
	$(CC) -o Engine3D $^ $(LFLAGS) $(CFLAGS) -I/include

clean:
	$(RM) $(OBJ) Engine3D