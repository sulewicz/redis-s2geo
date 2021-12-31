#!/bin/bash
set -e
DIR=$1
NAME=$2
echo "Preparing $DIR/build/$NAME.tar.gz..."
cd $DIR/build
echo "`pwd` contains:"
ls -al libredis-s2geo.*so
mkdir $NAME
cp -R libredis-s2geo.*so* $NAME/
tar -czvf $NAME.tar.gz $NAME/
