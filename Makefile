#////////////////////////////////////////////////////////////
#//
#// File:  Makefile
#// Authors:  G. Fotiades, A. Koutmos
#// Contributors:
#// Last modified: 2/2/11
#//
#// Description:  Makefile, targets:
#//               Solaris Build:
#//                 - make
#//                 - make all
#//                 - make tessellation
#//               OS X Build:
#//                 - make mac
#//               Cleanup:
#//                 - make clean
#//                 - make realclean
#//
#////////////////////////////////////////////////////////////

LDLIBS = -lglut -lGLU -lGL
MACLDLIBS = -framework GLUT -framework OpenGL -framework Cocoa

SOURCE_C = project3.cpp
SOURCE_H = 
OBJ_FILES = project3.o
BINARY = project3

CC =       gcc
CXX =      g++

COMPILER_FLAGS = -g

all:	project3

project3: $(OBJFILES) $(SOURCE_H) $(SOURCE_C)
	$(CXX) $(COMPILER_FLAGS) -c $(SOURCE_C)
	$(CXX) $(LDLIBS) -o $(BINARY) $(OBJ_FILES)

mac: $(SOURCE_H) $(SOURCE_C)
	$(CXX) $(COMPILER_FLAGS) -D__MAC_BUILD__ -c $(SOURCE_C)
	$(CXX) $(MACLDLIBS) -o $(BINARY) $(OBJ_FILES)

clean:
	rm -rf $(OBJ_FILES) $(BINARY) ptrepository SunWS_cache .sb ii_files core 2> /dev/null

realclean:       clean
