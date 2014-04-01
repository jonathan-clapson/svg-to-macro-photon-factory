CC 		= g++
CFLAGS 	= -O3 -g
RM		= rm -f

INCLUDE_DIRS=../macroio
LIBDIRS=../macroio

VIEWER_SRC=viewer.cpp
VIEWER_HDR=

#CPP_FILES=gnuplot_i.cpp main.cpp shape_math.cpp
#HDR_FILES=gnuplot_i.h shape_math.h

viewer: $(VIEWER_SRC) $(VIEWER_HDR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIRS) -L$(LIBDIRS) -o viewer $(VIEWER_SRC) `pkg-config --libs --cflags allegro-5 allegro_primitives-5  ` -lmacroio

clean: clean_tmp
	$(RM) viewer

clean_tmp:
	$(RM) gnuplot_tmpdatafile_*
