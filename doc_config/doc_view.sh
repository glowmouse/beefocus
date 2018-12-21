#!/bin/bash

set -e 
doc_config/doc_build.sh
chromium-browser code_docs/index.html

