CFLAGS  = -Wall -Wextra -Werror
SRC     = $(wildcard *.c)

kontakt-converter: $(SRC)
	gcc -o $@ $^ $(CFLAGS)
