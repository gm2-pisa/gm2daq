#####################################################################
#
#  Name:         Makefile
#
#  Contents:     Makefile for MIDAS analyzer
#
#####################################################################
#
#--------------------------------------------------------------------
# The MIDASSYS should be defined prior the use of this Makefile
ifndef MIDASSYS
missmidas::
	@echo "...";
	@echo "Missing definition of environment variable 'MIDASSYS' !";
	@echo "...";
endif

# get OS type from shell
OSTYPE = $(shell uname)

# list of module directories
MPATHLIST = $(shell ls -d ../modules/*)

#--------------------------------------------------------------------
# The following lines contain specific switches for different UNIX
# systems. Find the one which matches your OS and outcomment the 
# lines below.

#-----------------------------------------
# This is for Linux
ifeq ($(OSTYPE),Linux)
OSTYPE = linux
ifeq ($(shell uname -p),x86_64)
  OSTYPE = linux64
endif
endif

ifeq ($(OSTYPE),linux)
OS_DIR = linux
OSFLAGS = -DOS_LINUX -Dextname
CFLAGS = -g -O2 -Wall
# add to compile in 32-bit mode
# OSFLAGS += -m32
LIBS = -lm -lz -lutil -lnsl -lpthread -lrt -lstdc++
endif

ifeq ($(OSTYPE),linux64)
OS_DIR = linux
OSFLAGS = -DOS_LINUX -Dextname
CXXFLAGS += -g -O2 -Wall
CFLAGS  += -g -O2 -Wall
# add to compile in 32-bit mode
# OSFLAGS += -m32
LIBS = -lm -lz -lutil -lnsl -lpthread -lrt
endif

#-----------------------------------------
# ROOT flags and libs
#
ifdef ROOTSYS
ROOTCFLAGS := $(shell  $(ROOTSYS)/bin/root-config --cflags)
ROOTCFLAGS += -DHAVE_ROOT -DUSE_ROOT
ROOTLIBS   := $(shell  $(ROOTSYS)/bin/root-config --libs) -Wl,-rpath,$(ROOTSYS)/lib
ROOTLIBS   += -lThread
else
missroot:
	@echo "...";
	@echo "Missing definition of environment variable 'ROOTSYS' !";
	@echo "...";
endif
#-------------------------------------------------------------------
# The following lines define directories. Adjust if necessary
#                 
INC_DIR   = $(MIDASSYS)/include
LIB_DIR   = $(MIDASSYS)/$(OS_DIR)/lib
BIN_DIR   = $(OS_DIR)/bin
OBJ_DIR   = $(OS_DIR)/lib

VPATH = $(LIB_DIR):$(INC_DIR):../modules/:../src:$(MPATHLIST)



# ------------------------------------------------------------------
# AlazarTech (for ATS9870 boards)
ATS9870SDK=$(HOME)/DAQ/ATS9870sdk
CXXFLAGS += -I$(ATS9870SDK) -I$(ATS9870SDK)/include -I../common
CCFLAGS  += -I$(ATS9870SDK) -I$(ATS9870SDK)/include -I../common
# LDFLAGS  += -L$(ATS9870SDK)/lib -lPlxApi



#-------------------------------------------------------------------
# List of analyzer modules
#
#MODULES = sis3350.o sis3350_wf.o sis3350_islands.o sis3350_packet.o sis3350_hist.o  sis3350_hist_raw.o   run_number.o sis3350_defna.o sis3350_defna_hist.o 

#MODULES =  $(OBJ_DIR)/sis3350_wf.o

####################################################################
# Lines below here should not be edited
####################################################################

# MIDAS library
LIB = $(LIB_DIR)/libmidas.a

# compiler
CC = gcc
CXX = g++
CFLAGS += -g -I$(INC_DIR) 
CXXFLAGS += -g -I$(INC_DIR) -I../src -I../modules/common -I.
LDFLAGS +=

all:	$(BIN_DIR)/analyzer 

MODULES.inc: MODULES
	./make_modules_h.sh

include MODULES.inc

MODULES.h: MODULES
	./make_modules_h.sh

$(BIN_DIR)/analyzer: $(LIB) $(OBJ_DIR)/rmana.o $(OBJ_DIR)/analyzer.o $(MODULES) MODULES.h MODULES.inc
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_DIR)/rmana.o $(OBJ_DIR)/analyzer.o $(MODULES) \
	$(LIB) $(LDFLAGS) $(ROOTLIBS) $(LIBS)

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(USERFLAGS) $(ROOTCFLAGS) $(CXXFLAGS) $(OSFLAGS) -o $@ -c $<

$(OBJ_DIR)/analyzer.o: MODULES.h

clean::
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/* *~ \#* MODULES.inc MODULES.h



#end file
