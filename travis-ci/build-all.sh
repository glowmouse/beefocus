#!/bin/bash

# Print all commands to the terminal
set -x

# Exit with an error if anything we call exits with an error.
set -e

# Make a build directory
mkdir -p build/simulator
pushd build/simulator 2> /dev/null

# Create makefiles and run make
cmake ../..
make

popd 2> /dev/null
exit 0

