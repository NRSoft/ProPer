#!/bin/bash

# change major version number HERE if needed
#  and reset the minor number in "version.h" to 0
major=0

# check if there are any changes at all
status=stat.diff
git status -s > $status
if [ -s $status ]
then
    # check if version.h has already been changed
    header="src/version.h"
    git diff $header > $status
    if [ ! -s $status ]
    then
        # increment minor version
        read -r firstline < $header
        substr="$(echo -e "${firstline:22}" | tr -d '[:space:]')" # remove whitespaces
        minor=$((substr + 1))
        echo '#define VERSION_MINOR' $minor > $header
        echo '#define VERSION_MAJOR' $major >> $header
        builddate="$(date +%d/%m/%Y)"
        echo '#define VERSION_DATE "'$builddate'"' >> $header
    fi
fi
