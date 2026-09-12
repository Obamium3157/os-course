#!/usr/bin/bash

set -e

if [ -d out ]; then
  rm -rf out/*
else
  mkdir out
fi
cd out

whoami > me.txt

cp me.txt metoo.txt

man wc > wchelp.txt