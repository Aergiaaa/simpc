CC = cc
CFLAGS = -Wall -g

.PHONY: build run clean

build:
	@$(CC) ./src/*.c  $(CFLAGS) -o ./build/simpc

run: build
	@./build/simpc

clean:
	@rm ./build/simpc
