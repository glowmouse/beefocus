#!/bin/bash

set -e
doc_config/doc_git_setup.sh
doc_config/doc_build.sh
doc_config/doc_git_push.sh

