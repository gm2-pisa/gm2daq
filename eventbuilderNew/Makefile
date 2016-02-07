#####################################################################
#
#  Name:         Makefile
#  Created by:   Pierre-Andre Amaudruz
#
#  Contents:     Makefile for mevb MIDAS example 
#
#  $Id: Makefile 5207 2011-10-06 21:11:02Z amaudruz $
#
#####################################################################

#--------------------------------------------------------------------
# The following lines contain specific switches for different UNIX
# systems. Find the one which matches your OS and outcomment the 
# lines below.

#
#--------------------------------------------------------------------
# The MIDASSYS should be defined prior the use of this Makefile
ifndef MIDASSYS
missmidas::
	@echo "...";
	@echo "Missing definition of environment variable 'MIDASSYS' !";
	@echo "...";
endif

#-----------------------------------------
# This is for OSF1
#LIBS = -lbsd -lzlib
#OSFLAGS = -DOS_OSF1 -Dextname
#FF = f77
#FFLAGS = -nofor_main -D 40000000 -T 20000000

#-----------------------------------------
# This is for Linux
OSFLAGS = -DOS_LINUX -Dextname
CFLAGS = 

# Overwrite MIDAS MAX_EVENT_SIZE with environment variable
#
ifdef MIDAS_MAX_EVENT_SIZE
CFLAGS += -DMAX_EVENT_SIZE=$(MIDAS_MAX_EVENT_SIZE)
endif

# for f2c, uncomment following:
#LIBS = -lbsd -lm -lutil /usr/lib/libf2c.a
#FF = cc
# for egcs g77, use this
FF = g77
LIBS = -lm -lz -lutil -lnsl -lpthread -lrt -lstdc++

#-------------------------------------------------------------------
# The following lines define direcories. Adjust it if necessary.
#          
DRV_DIR         = $(MIDASSYS)/drivers
INC_DIR         = $(MIDASSYS)/include
LIB_DIR         = $(MIDASSYS)/linux/lib
MX_DIR          = $(MIDASSYS)/../mxml

#-------------------------------------------------------------------
# Hardware driver can be (camacnul, kcs2926, kcs2927, hyt1331)
#
DRIVER = camacnul

####################################################################
# Lines below here should not be edited
####################################################################

# MIDAS library
LIB = $(LIB_DIR)/libmidas.a

# compiler
CC = gcc
CXX = g++
CFLAGS += -g -O2 -Wall -Wno-strict-aliasing -Wuninitialized -I$(INC_DIR) -I$(DRV_DIR) -I$(MX_DIR) -DHAVE_FTPLIB
LDFLAGS +=

all: mevb ebfe

ebfe: $(LIB) $(LIB_DIR)/mfe.o $(DRV_DIR)/camac/$(DRIVER).o ebfe.c
	$(CC) $(CFLAGS) $(OSFLAGS) -o ebfe ebfe.c $(DRIVER).o \
	$(LIB_DIR)/mfe.o $(LIB) $(LDFEFLAGS) $(LIBS)

$(DRIVER).o: $(DRV_DIR)/camac/$(DRIVER).c
	$(CC) $(CFLAGS) $(OSFLAGS) -c -o $(DRIVER).o $(DRV_DIR)/bus/$(DRIVER).c

mdsupport.o: $(MIDASSYS)/src/mdsupport.cxx
	$(CXX) $(CFLAGS) $(OSFLAGS) -o $@ -c $<

mevb:   $(LIB) mevb.c ebuser.o mdsupport.o
	$(CC) $(CFLAGS) $(OSFLAGS) -o mevb mevb.c ebuser.o mdsupport.o $(LIB) $(LDFEFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

clean:
	rm -f *.o *~ mevb \#*


