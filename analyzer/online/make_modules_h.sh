#!/bin/bash

outfile="MODULES.h"
modules=""
out=""

while read name; do
    case $name in
	\#*)	    
	    ;;
	*)
	    modules=$modules" "$name
	    ;;
    esac
done < MODULES

echo "" > $outfile
for module in $modules; do
    echo "extern ANA_MODULE "$module"_module;" >> $outfile
done

echo "" >> $outfile
echo "ANA_MODULE *offline_modules[] = {" >> $outfile
for module in $modules; do
    echo "    &"$module"_module," >> $outfile
done
echo "    NULL" >> $outfile
echo "};" >> $outfile


# build the list of file for make
echo "MODULES = \\" > MODULES.inc
for module in $modules; do
    echo -e "\t\$(OBJ_DIR)/"$module".o \\" >> MODULES.inc
done
echo "" >> MODULES.inc