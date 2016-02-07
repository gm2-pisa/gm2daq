# Makefile for building my C++ IPbus tools for the Cornell WFDs

# before using this makefile:
# 1. edit setenv.sh to point to your cactus installation
#     (may also need to edit INCLUDE_PATH and LIBRARY_PATH 
#      below if your installation has a different structure)
# 2. source setenv.sh (to define CACTUS_ROOT) 

# Robin Bjorkquist, March 2015

# I use .cxx extension for executable sources
#       .cc  extension for other sources (which will be made 
#            into a library to be linked into the executables)

LIBRARY = lib/libCornellWFDTool.so
LIBRARY_SOURCES = $(wildcard src/*.cc)
LIBRARY_OBJECT_FILES = $(patsubst src/%.cc,obj/%.o,${LIBRARY_SOURCES})

EXECUTABLE_SOURCES = $(wildcard src/*.cxx)
EXECUTABLE_OBJECT_FILES = $(patsubst src/%.cxx,obj/%.o,${EXECUTABLE_SOURCES})
EXECUTABLES = $(patsubst src/%.cxx,bin/%,${EXECUTABLE_SOURCES})

CC = g++

CFLAGS = -c -MMD -MP -fPIC
#  -c     only make the .o object file (do not run the linker)
#  -MMD   create the .d dependency file at the same time as the .o object file
#  -MP    avoids make errors that would arise if you removed a .hh header 
#         file that was previously listed in the .d dependency file
#  -fPIC  needed to create shared library

LIBRARY_LFLAGS = -shared -fPIC
#  -shared   produce a shared library (.so) object
#  -fPIC     needed to create shared library

EXECUTABLE_LFLAGS = 

INCLUDE_PATH = -Iinclude \
               -I$(CACTUS_ROOT)/uhal/uhal/include \
	       -I$(CACTUS_ROOT)/uhal/grammars/include \
	       -I$(CACTUS_ROOT)/uhal/log/include \
	       -I$(CACTUS_ROOT)/extern/pugixml/RPMBUILD/SOURCES/include \
	       -I$(CACTUS_ROOT)/extern/boost/RPMBUILD/SOURCES/include

LIBRARY_PATH = -Llib \
               -L$(CACTUS_ROOT)/uhal/uhal/lib \
               -L$(CACTUS_ROOT)/uhal/grammars/lib \
               -L$(CACTUS_ROOT)/uhal/log/lib \
               -L$(CACTUS_ROOT)/extern/pugixml/RPMBUILD/SOURCES/lib \
               -L$(CACTUS_ROOT)/extern/boost/RPMBUILD/SOURCES/lib

LIBS = -lcactus_uhal_uhal \
       -lcactus_uhal_grammars \
       -lcactus_uhal_log \
       -lcactus_extern_pugixml \
       -lboost_filesystem \
       -lboost_regex \
       -lboost_system \
       -lboost_thread

EXECUTABLE_LIBS = ${LIBS} -lCornellWFDTool

.PHONY: all clean cleanobjs cleanall # indicates that these targets are not filenames

all: ${LIBRARY} ${EXECUTABLES}

clean: cleanall

cleanobjs:
	\rm -rf obj

cleanall:
	\rm -rf obj
	\rm -rf lib
	\rm -rf bin

# create my own pattern rules

# automatic variables in make:
#	$@ = filename of target
#	$< = filename of first prerequisite
#	$^ = filename of all prerequisites, no duplicates

# automatic dependency generation:
#    The .d files are generated along with the .o files 
#    because of the -MMD compiler option in CFLAGS

# ----------------------------------------
# library
# ----------------------------------------
${LIBRARY}: ${LIBRARY_OBJECT_FILES}
	mkdir -p lib
	$(CC) $(LIBRARY_LFLAGS) $(LIBRARY_PATH) $(LIBS) $(LIBRARY_OBJECT_FILES) -o $@

${LIBRARY_OBJECT_FILES}: obj/%.o: src/%.cc
	mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_PATH) $< -o $@

-include $(LIBRARY_OBJECT_FILES:.o=.d)

# ----------------------------------------
# executables
# ----------------------------------------
${EXECUTABLES}: bin/%: obj/%.o
	mkdir -p bin
	$(CC) $(EXECUTABLE_LFLAGS) $(LIBRARY_PATH) $(EXECUTABLE_LIBS) $< -o $@

${EXECUTABLE_OBJECT_FILES}: obj/%.o: src/%.cxx
	mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE_PATH) $< -o $@

-include $(EXECUTABLE_OBJECT_FILES:.o=.d)

