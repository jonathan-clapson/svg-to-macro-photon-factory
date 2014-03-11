all:
	g++ -o convert main.cpp `pkg-config --libs --cflags libxml-2.0`
