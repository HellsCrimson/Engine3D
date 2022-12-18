CC=g++
SRC=$(shell find src -name "*.cpp")
OBJ=$(SRC:.cpp=.o)
CPPFLAGS=-std=c++17 -Iinclude -lX11 -lGL -lpthread -lpng -lstdc++fs

all: engine

engine: $(OBJ)
	$(CC) -o Engine3D $^ $(CPPFLAGS) -I/include

clean:
	$(RM) $(OBJ) Engine3D