#!/bin/bash

# Print all commands to the terminal
set -x

# Exit with an error if anything we call exits with an error.
set -e

# Make a build directory
mkdir -p build/simulator
pushd build/simulator

# Create makefiles and run make
cmake ../..
make

popd
exit 0

