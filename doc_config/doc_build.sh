#!/bin/bash

set -e 

mkdir -p code_docs
cd code_docs
rm -rf *

doxygen ../doc_config/Doxyfile 2>doxygen.err | tee doxygen.log
mv html/* .
rmdir html

