#!/bin/bash

#
# Copied from https://github.com/indilib/indi
#

set -e

mkdir -p build
pushd build 2> /dev/null
git clone https://github.com/google/googletest.git
if [ ! -d googletest ]; then
echo "Failed to get googletest.git repo"
exit 1
fi
pushd googletest
mkdir build
cd build
cmake .. -DCMAKE_CXX_FLAGS="-fPIC"
sudo make install
popd 2> /dev/null
rm -rf googletest
popd 2> /dev/null

