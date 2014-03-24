CPP_FILES := $(wildcard *.cpp)
#OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
OBJ_FILES := $(notdir $(CPP_FILES:.cpp=.o))

all: libmacroio.so

libmacroio.so: $(OBJ_FILES)
	g++ $(LD_FLAGS) -shared -o $@ $^

%.o: %.cpp
	g++ -fpic -c $<

clean:
	rm *.o
	rm libmacroio.so
