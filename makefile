CFLAGS = -Wall -Wextra
SRC =$(wildcard src/*.c)
LIBS = -lssl -lcrypto

main:
	clang $(CFLAGS) $(SRC) -o main $(LIBS)