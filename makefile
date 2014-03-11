all:
	g++ -o convert main.cpp macrowriter.cpp `pkg-config --libs --cflags libxml-2.0`
