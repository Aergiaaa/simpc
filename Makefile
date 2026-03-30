CC = cc
CFLAGS = -Wall -g

build:
	@$(CC) *.c  $(CFLAGS) -o simpc

run: build
	@./simpc

clean:
	@rm simpc
