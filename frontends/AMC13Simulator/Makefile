#####################################################################
#
#  Name:         Makefile
#  Created by:   Stefan Ritt
#
#  Contents:     Makefile for MIDAS example frontend and analyzer
#
#  $Id: Makefile 4647 2009-12-08 07:52:56Z ritt $
#
#####################################################################

USE_CALO_SIMULATOR = 1
USE_GPU            = 0

#
#--------------------------------------------------------------------
# The GM2DAQ_DIR should be defined prior the use of this Makefile
ifndef GM2DAQ_DIR
missdaq::
	@echo "...";
	@echo "Missing definition of environment variable 'GM2DAQ_DIR' !";
	@echo "...";
endif

#
#--------------------------------------------------------------------
# The MIDASSYS should be defined prior the use of this Makefile
ifndef MIDASSYS
missmidas::
	@echo "...";
	@echo "Missing definition of environment variable 'MIDASSYS' !";
	@echo "...";
endif

VPATH = ../common


#
# Use STRUCK driver
#

#STRUCK_DRIVER_PATH  := $(HOME)/DAQ/drivers/struck/sis1100-2.13-5

#CXXFLAGS += -I$(STRUCK_DRIVER_PATH) -I$(STRUCK_DRIVER_PATH)/sis3100_calls  -I../common 
#CCFLAGS  += -I$(STRUCK_DRIVER_PATH) -I$(STRUCK_DRIVER_PATH)/sis3100_calls  -I../common 
# LDFLAGS  += -L$(STRUCK_DRIVER_PATH)/sis3100_calls/$(OS_DIR)/lib -ldl -lstdc++  -lrt -l_sis3100 -Wl,"-rpath=$(STRUCK_DRIVER_PATH)/sis3100_calls"


CXXFLAGS += -I../common -pthread -D_REENTRANT
CCFLAGS  += -I../common -pthread -D_REENTRANT

# activate debug print statements
DEBUG=0
ifeq ($(DEBUG),1)
CXXFLAGS += -DDEBUG
CCFLAGS += -DDEBUG
endif

# get OS type from shell
OSTYPE = $(shell uname)


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
CXXFLAGS += -g -O2 -Wall
CCFLAGS  += -g -O2 -Wall
# add to compile in 32-bit mode
OSFLAGS += -m32
LIBS = -lm -lz -lutil -lnsl -lpthread -lrt
endif

ifeq ($(OSTYPE),linux64)
OS_DIR = linux
OSFLAGS = -DOS_LINUX -Dextname
CXXFLAGS += -g -O2 -Wall
CCFLAGS  += -g -O2 -Wall
# add to compile in 32-bit mode
# OSFLAGS += -m32
LIBS = -lm -lz -lutil -lnsl -lpthread -lrt
endif

#-----------------------
# MacOSX/Darwin is just a funny Linux
#
ifeq ($(OSTYPE),Darwin)
OSTYPE = darwin
endif

ifeq ($(OSTYPE),darwin)
OS_DIR = darwin
FF = cc
OSFLAGS = -DOS_LINUX -DOS_DARWIN -DHAVE_STRLCPY -DAbsoftUNIXFortran -fPIC -Wno-unused-function
LIBS = -lpthread
SPECIFIC_OS_PRG = $(BIN_DIR)/mlxspeaker
NEED_STRLCPY=
NEED_RANLIB=1
NEED_SHLIB=
NEED_RPATH=

endif

#-------------------------------------------------------------------
# The following lines define directories. Adjust if necessary
#                 
DRV_DIR   = $(MIDASSYS)/drivers/camac
INC_DIR   = $(MIDASSYS)/include
LIB_DIR   = $(MIDASSYS)/$(OS_DIR)/lib
SRC_DIR   = $(MIDASSYS)/src

#-------------------------------------------------------------------
# List of modules
#
# gpu_thread.o

MODULES = frontend_aux.o \
	frontend_rpc.o \
	timetool.o \
	amc13simulator_odb.o \
	tcp_server.o \
	../CaloReadoutAMC13/amc13_odb.o \
	pack_amc13_data.o

# DAQ drivers
CCFLAGS += -I$(GM2DAQ_DIR)/drivers
CXXFLAGS += -I$(GM2DAQ_DIR)/drivers


#
# Calorimeter data simulator
#
ifeq ($(USE_CALO_SIMULATOR),1)
CXXFLAGS += -DUSE_CALO_SIMULATOR
CCFLAGS += -DUSE_CALO_SIMULATOR
MODULES  += simulator.o

ROOTCFLAGS := $(shell  $(ROOTSYS)/bin/root-config --cflags)
ROOTCFLAGS += -DHAVE_ROOT -DUSE_ROOT
ROOTLIBS   := $(shell  $(ROOTSYS)/bin/root-config --libs) -Wl,-rpath,$(ROOTSYS)/lib
ROOTLIBS   += -lThread

#LIB      += $(ROOTLIBS)
LDFLAGS  += $(ROOTLIBS)
CXXFLAGS += $(ROOTCFLAGS)

endif

#
# GPU
#
ifeq ($(USE_GPU),1)
CXXFLAGS += -DUSE_GPU
MODULES  += gpu_thread.o \
	cuda_tools_g2.o \
	kernel.cu.o
include cuda.mk
endif


# sis3350_odb.o sis3100_tools.o sis3350_tools.o udp_thread.o \
#	eeprom_ds2430.o \
#	kernel.cu.o \
#	cuda_tools_g2.o \
#sis3350_tools.o
#adccalib.o adcsum.o scaler.o

#-------------------------------------------------------------------
# Hardware driver can be (camacnul, kcs2926, kcs2927, hyt1331)
#
DRIVER = camacnul

#-------------------------------------------------------------------
# Frontend code name defaulted to frontend in this example.
# comment out the line and run your own frontend as follow:
# gmake UFE=my_frontend
#
UFE = frontend
#UFE1 = frontend_vme

####################################################################
# Lines below here should not be edited
####################################################################

# MIDAS library
LIB += $(LIB_DIR)/libmidas.a

# compiler
CC = gcc
CXX = g++
COMMONFLAGS += -g -I$(INC_DIR) -I$(DRV_DIR) -I$(GM2DAQ_DIR)/frontends/common
CXXFLAGS += $(COMMONFLAGS)
CCFLAGS  += $(COMMONFLAGS)
LDFLAGS +=


all: $(UFE) $(UFE1) 
#analyzer
#include $(UFE).d
#include $(TARGET).mk

#$(TARGET).d: $(TARGET).cxx
#        set -e; $(CXX) -M $(CPPFLAGS) $< \
#                | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
#                [ -s $@ ] || rm -f $@

blank:
	touch blank

include blank $(wildcard *.d)


$(UFE): $(LIB) $(LIB_DIR)/mfe.o $(DRIVER).o $(SRC_DIR)/cnaf_callback.c $(MODULES) $(UFE).cpp Makefile
	$(CXX) $(CXXFLAGS) $(OSFLAGS) -o $(UFE) \
	$(SRC_DIR)/cnaf_callback.c $(DRIVER).o  $(UFE).cpp $(MODULES) $(LIB_DIR)/mfe.o $(LIB) \
	$(LDFLAGS) $(LIBS)

#$(UFE1): $(LIB) $(LIB_DIR)/mfe.o $(DRIVER).o $(SRC_DIR)/cnaf_callback.c $(MODULES) $(UFE1).cpp Makefile
#	$(CXX) $(CXXFLAGS) $(OSFLAGS) -o $(UFE1) \
#	$(SRC_DIR)/cnaf_callback.c $(DRIVER).o  $(UFE1).cpp $(MODULES) $(LIB_DIR)/mfe.o $(LIB) \
#	$(LDFLAGS) $(LIBS)


$(DRIVER).o: $(DRV_DIR)/$(DRIVER).c
	$(CC) $(CCFLAGS) $(OSFLAGS) -c -o $(DRIVER).o $(DRV_DIR)/$(DRIVER).c

#analyzer: $(LIB) $(LIB_DIR)/rmana.o analyzer.o $(MODULES)
#	$(CXX) $(CFLAGS) -o $@ $(LIB_DIR)/rmana.o analyzer.o $(MODULES) \
#	$(LIB) $(LDFLAGS) $(ROOTLIBS) $(LIBS)

%.o: %.cxx
	$(CXX) $(USERFLAGS) $(CXXFLAGS) $(OSFLAGS) -o $@ -c $<
	$(CXX) -MM $(CXXFLAGS) $< > $(notdir $(patsubst %.cxx,%.d,$<))

%.o: %.c 
	$(CC)  $(USERFLAGS) $(CCFLAGS) $(OSFLAGS) -o $@ -c $<
	$(CC)  -MM $(CXXFLAGS) $< > $(notdir $(patsubst %.c,%.d,$<)) 

clean::
	rm -f *.o *.d  *~ \#*

#end file
