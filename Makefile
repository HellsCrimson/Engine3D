CC=g++
SRC=$(shell find src -name "*.cpp")
OBJ=$(SRC:.cpp=.o)
CPPFLAGS=-std=c++17 -Iinclude -DUNICODE -I/usr/include/SDL2 -lSDL2 -lX11 -lGL -lpthread -lpng -lstdc++fs

all: engine

engine: $(OBJ)
	$(CC) -o Engine3D $^ $(CPPFLAGS)

clean:
	$(RM) $(OBJ) Engine3D