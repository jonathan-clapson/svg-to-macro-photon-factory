CC=g++

INCLUDE_DIRS=../macroio
LIBDIRS=../macroio

CONVERT_SRC=main.cpp
CONVERT_HDR=

all: convert

convert: $(CONVERT_SRC) $(CONVERT_HDR)
	$(CC) -g -L$(LIBDIRS) -o convert -I$(INCLUDE_DIRS) $(CONVERT_SRC) -lmacroio `pkg-config --libs --cflags libxml-2.0`

clean:
	rm convert
