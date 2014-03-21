CONVERT_SRC=main.cpp macrowriter.cpp shape_math.cpp laser_types.cpp
CONVERT_HDR=macrowriter.h shape_math.h laser_types.h macroerror.h

all: convert

convert: $(CONVERT_SRC) $(CONVERT_HDR)
	g++ -o convert $(CONVERT_SRC) `pkg-config --libs --cflags libxml-2.0`
reader:
	g++ -o reader reader.cpp macroreader.cpp shape_math.cpp laser_types.cpp `pkg-config --libs --cflags libxml-2.0`

clean:
	rm convert
	rm reader
