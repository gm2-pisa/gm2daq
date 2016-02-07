if [[ $_ == $0 ]]; then  
  echo "$0 is meant to be sourced:"
  echo "  source $0"
  exit 0
fi

# --------------------------------------
# set up cactus
# --------------------------------------

#CACTUS_ROOT=/mnt/sw/ipbus/cactus/cactuscore
CACTUS_ROOT=/wildcat/gohn/gm2daq-gm2ipbus/build/trunk/cactuscore

LD_LIBRARY_PATH=$CACTUS_ROOT/uhal/uhal/lib:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=$CACTUS_ROOT/uhal/grammars/lib:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=$CACTUS_ROOT/uhal/log/lib:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=$CACTUS_ROOT/extern/pugixml/RPMBUILD/SOURCES/lib:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=$CACTUS_ROOT/extern/boost/RPMBUILD/SOURCES/lib:$LD_LIBRARY_PATH

# --------------------------------------
# set up Cornell WFD Tool
# --------------------------------------

CORNELL_WFD_TOOL_ROOT=$( readlink -f $(dirname $BASH_SOURCE)/ )

# add CornellWFDTool library to the LD_LIBRARY_PATH
LD_LIBRARY_PATH=$CORNELL_WFD_TOOL_ROOT/lib:${LD_LIBRARY_PATH}

# add CornellWFDTool executables to the PATH
PATH=${CORNELL_WFD_TOOL_ROOT}/bin:${PATH}

export CACTUS_ROOT
export LD_LIBRARY_PATH
export PATH
