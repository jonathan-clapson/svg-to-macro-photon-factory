all:
	g++ -o convert main.cpp macrowriter.cpp shape_math.cpp `pkg-config --libs --cflags libxml-2.0`
