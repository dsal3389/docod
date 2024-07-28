
CC=clang
C_FLAGS=-Wall -g -O3

build:
	$(CC) $(C_FLAGS) src/*.c

