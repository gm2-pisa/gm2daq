#!/bin/bash

#Determine the absolute directory of this script
export GM2DAQ_DIR=$(dirname ${BASH_SOURCE[0]} | xargs readlink -e)

#ROOT
export ROOTSYS=/home/daq/root

#MIDAS
export MIDASSYS=/home/daq/gm2midas/midas
export MIDAS_EXPTAB=/etc/exptab

#ROME
export ROMESYS=/home/daq/gm2midas/rome

#IPBus
export CACTUS_ROOT=/opt/cactus

#AMC13
export AMC13_STANDALONE_ROOT=$GM2DAQ_DIR/amc13

#CUDA
export CUDASYS=/usr/local/cuda

#Update PATH and LD_LIBRARY_PATH
export PATH=$PATH:$ROOTSYS/bin:$MIDASSYS/linux/bin:$CACTUS_ROOT/bin:$AMC13_STANDALONE_ROOT/tools/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ROOTSYS/lib:$MIDASSYS/linux/lib:$CACTUS_ROOT/lib:$AMC13_STANDALONE_ROOT/amc13/lib:$AMC13_STANDALONE_ROOT/tools/lib

echo "Done!"

