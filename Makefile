CC=g++
SRC=src
OBJ=obj
CFLAGS=-std=c++17
LFLAGS=-lX11 -lGL -lpthread -lpng -lstdc++fs

all: engine

engine: engine.o
	$(CC) -o Engine $(OBJ)/Engine3D.o $(LFLAGS) $(CFLAGS)

engine.o: $(SRC)/Engine3D.cpp
	$(CC) -c $(SRC)/Engine3D.cpp -o $(OBJ)/Engine3D.o

clean:
	$(RM) $(OBJ)/*.o
	$(RM) Engine