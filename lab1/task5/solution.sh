#!/bin/bash
# TODO: почему работает без /usr/

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

wc -l wchelp.txt | cut -d ' ' -f 1 > wchelp-lines.txt

tac wchelp.txt > wchelp-reversed.txt

cat wchelp.txt wchelp-reversed.txt me.txt metoo.txt wchelp-lines.txt > all.txt

tar -czf result.tar *.txt # TODO: в чем разница между -z и отдельной командой gzip

gzip result.tar

cd ..

if [ -f result.tar.gz  ]; then
  rm -rf result.tar.gz
fi

mv out/result.tar.gz ./

rm -rf out/*
rmdir out