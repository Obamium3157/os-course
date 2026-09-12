#!/usr/bin/bash

tar -czf proj.tar.gz proj
if [ -d out ]; then
  # TODO: уточнить применение флагов
  read -r -p "Directory out already exists. Recreate? (y/N) " ans
  if [[ $ans =~ ^[Yy]$ ]]; then
    rm -rf out
  else
    exit 1
  fi
fi
mkdir out
cp proj.tar.gz out/
cd out
tar -xzf proj.tar.gz --strip-components=1 # TODO: что будет, если написать 2. Показать примеры
rm -rf proj.tar.gz
mkdir include src build
mv lib.h include/
mv lib.cpp main.cpp src/
g++ src/main.cpp src/lib.cpp -Iinclude -o build/proj # TODO: доуточнить про Iinclude
echo "30 12" | ./build/proj > stdout.txt