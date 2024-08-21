#!/bin/bash

CURRENTDIR=$(pwd)

sudo apt update
sudo apt upgrade
sudo apt install g++ cmake gdb pkg-config autoconf generate-ninja ninja-build curl zip unzip tar

sudo ./netmem.sh

cd ~/
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

echo "export VCPKG_ROOT=$HOME/vcpkg" >> ~/.bashrc
echo "export PATH=\$VCPKG_ROOT:\$PATH" >> ~/.bashrc

./bootstrap-vcpkg.sh

cd $CURRENTDIR