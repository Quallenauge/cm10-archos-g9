#!/bin/bash

# Args
# $1 - Base path
# $2 - base directory of the untarred component
# $3 - name of the component archive


if [ -d $1/$2 ]; then
  echo "Removing $1/$2"
  rm -rf $1/$2
fi

[ ! -d $1 ] && mkdir -p $1
tar -C $1 -zxf $3
