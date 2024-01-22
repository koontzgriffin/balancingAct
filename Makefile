OBJS = src/main.cpp
CC = /usr/bin/g++
INCLUDE_PATHS = -I/usr/local/include
LIBRARY_PATHS = -L/usr/local/lib
COMPILER_FLAGS = -std=c++11 -Wall -O2
LINKER_FLAGS = -lSDL2
OBJ_NAME = BalancingAct

all:
	$(CC) -o $(OBJ_NAME) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) $(OBJS)

bounce:
	$(CC) -o bounce $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) src/bounce.cpp

roll:
	$(CC) -o roll $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) src/roll.cpp