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

# get OS type from shell
OSTYPE = $(shell uname)
#--------------------------------------------------------------------
# The following lines contain specific switches for different UNIX
# systems. Find the one which matches your OS and outcomment the 
# lines below.

#----------------------------------------
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
OSFLAGS += -m32
LIBS = -lm -lz -lutil -lnsl -lpthread -lrt
endif

ifeq ($(OSTYPE),linux64)
OS_DIR = linux
OSFLAGS = -DOS_LINUX -Dextname
CFLAGS = -g -O2 -Wall
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

DEBUG=1

#-------------------------------------------------------------------
# The following lines define directories. Adjust if necessary
#                 
DRV_DIR   = $(MIDASSYS)/drivers/camac
INC_DIR   = $(MIDASSYS)/include
LIB_DIR   = $(MIDASSYS)/$(OS_DIR)/lib
SRC_DIR   = $(MIDASSYS)/src

#-------------------------------------------------------------------
# List of frpmtend modules
#
MODULES   = rpc_g2.o \
	master_odb.o
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

####################################################################
# Lines below here should not be edited
####################################################################

# MIDAS library
LIB = $(LIB_DIR)/libmidas.a

# compiler
CC = gcc
CXX = g++
CFLAGS += -g -I$(INC_DIR) -I$(DRV_DIR) -D_REENTRANT -pthread
LDFLAGS +=

# DAQ drivers
CFLAGS += -I$(GM2DAQ_DIR)/drivers -I$(GM2DAQ_DIR)/frontends/common

all: $(UFE) 
#analyzer

$(UFE): $(LIB) $(LIB_DIR)/mfe.o $(DRIVER).o $(UFE).c $(SRC_DIR)/cnaf_callback.c $(MODULES)
	$(CC) $(CFLAGS) $(OSFLAGS) -o $(UFE) $(UFE).c \
	$(SRC_DIR)/cnaf_callback.c $(DRIVER).o $(MODULES) $(LIB_DIR)/mfe.o $(LIB) \
	$(LDFEFLAGS) $(LIBS)

$(DRIVER).o: $(DRV_DIR)/$(DRIVER).c
	$(CC) $(CFLAGS) $(OSFLAGS) -c -o $(DRIVER).o $(DRV_DIR)/$(DRIVER).c

%.o: %.c rpc_g2.h 
	$(CC) $(USERFLAGS) $(CFLAGS) $(OSFLAGS) -o $@ -c $<

clean::
	rm -f *.o *~ \#*

#end file
