#!/bin/bash
set -e

echo "Downloading from github..."
git clone https://github.com/TheMediocritist/beepy_gnuboy || { echo "Error: Failed to clone display driver repository."; exit 1; }
cd beepy_gnuboy
./configure
make || { echo "Error: Failed to compile."; exit 1; }

echo "Installing binary and config file"
sudo make install
cp fbgnuboy /usr/local/bin
mkdir ~/.gnuboy
cp gnuboy.rc ~/.gnuboy/gnuboy.rc

echo "Loading..."
./fbgnuboy
