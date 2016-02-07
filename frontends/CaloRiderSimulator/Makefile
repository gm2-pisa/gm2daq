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

AMC13_ROOT  := $(HOME)/gm2daq/amc13
CACTUS_ROOT  := /opt/cactus

#rhf -  I believe this next block can be removed 5-11-2014
#CACTUS_PROJ_HOME = /home/daq/exampleForManuel
#CACTUS_LIB_SHARE = $(CACTUS_PROJ_HOME)/lib/libmyprojectlibname.so
#LIBRARY_PATH  = \
#	-L${CACTUS_ROOT}/lib \
#	-L/lib64 \
#	-L/diskless/SL60/x86_64/root/usr/lib64 \
#	-L/diskless/SL60/x86_64/root/lib64
#CACTUS_LIB = -lboost_filesystem \
#        -lboost_regex \
#        -lboost_system \
#        -lboost_thread \
#        \
#        -lcactus_extern_pugixml \
#        -lcactus_uhal_log \
#        -lcactus_uhal_grammars \
#        -l_uhal_uhal
#LINK_LIBRARY_FLAGS = -shared -fPIC - Wall -02 ${LIBRARY_PATH} ${LIBRARIES}

CXXFLAGS += -I${AMC13_ROOT}/include
CCFLAGS  += -I${AMC13_ROOT}/include
CXXFLAGS += -I${CACTUS_ROOT}/include
CCFLAGS  += -I${CACTUS_ROOT}/include
CXXFLAGS += -I/usr/include/
CCFLAGS  += -I/usr/include/
CXXFLAGS += -I/diskless/SL60/x86_64/root/usr/include
CCFLAGS  += -I/diskless/SL60/x86_64/root/usr/include


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
CXXFLAGS += -pg -O2 -Wall
CCFLAGS  += -pg -O2 -Wall
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
INC_DIR   = \
	-I/usr/include \
	-I${AMC13_ROOT}/include \
	-I$(MIDASSYS)/include \
	-I$(CACTUS_ROOT)/include \
        -I/diskless/SL60/x86_64/root/usr/include
LIB_DIR   = ${MIDASSYS}/${OS_DIR}/lib
SRC_DIR   = $(MIDASSYS)/src

#-------------------------------------------------------------------
# List of modules
#
# gpu_thread.o

MODULES = frontend_aux.o \
	frontend_rpc.o \
	timetool.o \
	ridertrigger_odb.o \
	tcp_server.o \
	amc13_odb.o \
	AMC13_library.o \
	Rider_library.o

# DAQ drivers
CCFLAGS += -I$(HOME)/gm2daq/drivers
CXXFLAGS += -I$(HOME)/gm2daq/drivers


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
#LIB += ${CACTUS_LIB_SHARE}
LIB += ${AMC13_ROOT}/lib/libuhalamc13.so
#LIB += ${AMC13_ROOT}/lib/libcactus_amc13_amc13.so
LIB += /lib64/libreadline.so.6.0
#Before I figured out to explicitly load the libreadline.so.6.0 libraries
#I just excludes the classes that needed those libraries and makemy own 
#shared library. Very hacky. 
#LIB += ${AMC13_ROOT}/lib/libuhalamc13_s.so


# compiler
CC = gcc
CXX = g++
COMMONFLAGS += -g -I$(INC_DIR) -I$(DRV_DIR) 
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


$(UFE): $(LIB) $(LIB_DIR)/mfe.o $(DRIVER).o $(SRC_DIR)/cnaf_callback.c $(MODULES) $(UFE).cpp  Makefile
	$(CXX) $(CXXFLAGS) $(OSFLAGS) -o $(UFE) \
	$(SRC_DIR)/cnaf_callback.c $(DRIVER).o  \
	$(UFE).cpp $(MODULES) $(LIB_DIR)/mfe.o $(LIB) $(LDFLAGS) $(LIBS) 

# decoded by rhf using http://www.gnu.org/software/make/manual/html_node/Simple-Makefile.html#Simple-Makefile
# why are cnaf_callback.c and frontend.cpp prerequisites for frontend target? Because make is smart and writes the object target and prequisites for you!  
# Why is Makefile a prerequisite? I think so that the Makefile is remade. 
# frontend : libmidas.a libmyprojectlibname.so /home/daq/DAQ/midas/linux64/libs/mfe.o camacnul.o /home/daq/DAQ/midas/src/cnaf_callback.c frontend_aux.o frontend_rpc.o timetool.ok tcpsimulator_odb.o tcp_server.o ../CaloReadoutAMC13/tcpreadout_odb.o frontend.cpp Makefile 
#     g++ CXXFLAGS OSFLAGS -o frontend cnaf_callback.c camacnul.o libmidas.a libmyprojectlibname.so  

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
	$(CC) $(USERFLAGS) $(CCFLAGS) $(OSFLAGS) -o $@ -c $<
	$(CC) -MM $(CXXFLAGS) $< > $(notdir $(patsubst %.c,%.d,$<))

clean::
	rm -f *.o *.d  *~ \#*

#end file
