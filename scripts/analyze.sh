#!/bin/bash

run=$1

export MIDAS_DIR=$HOME/experiment/offline

$HOME/DAQ/analyzer/analyzer -s 0 -i $HOME/experiment/run$1.mid -o hist$run.root 



