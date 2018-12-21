#!/bin/bash

set -e

rm -rf code_docs
git clone -b gh-pages https://github.com/glowmouse/beefocus.git code_docs
git config --global push.default simple

echo "" > .nojekyll

