CC = gcc

X11 = /opt/X11
INC = -I$(X11)/include
LIB = -L$(X11)/lib -lX11

SRC = main.c raymarch.c
HDR = raymarch.h

default: raymarch

raymarch: $(SRC) $(HDR)
	$(CC) -o $@ $(SRC) $(INC) $(LIB)

clean:
	rm -f raymarch
