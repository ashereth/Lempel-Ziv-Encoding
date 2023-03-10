SHELL := /bin/sh
CC = clang
CFLAGS = -Wall -Wpedantic -Werror -Wextra 

all: encode decode
	
encode: encode.o
	$(CC) -o encode -lm encode.o
	
encode.o: encode.c  
	$(CC) $(CFLAGS) -c encode.c

decode: decode.o
	$(CC) -o decode -lm decode.o
	
decode.o: decode.c  
	$(CC) $(CFLAGS) -c decode.c

clean:
	rm -f *.o encode decode

format:
	clang-format -i -style=file *.[ch]

