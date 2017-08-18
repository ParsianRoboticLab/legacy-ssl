#!/bin/bash
# These are the packages required for install on Ubuntu systems.
sudo apt-get install g++ libqt4-dev libeigen3-dev libdc1394-22 libdc1394-22-dev cmake libv4l-0  #SSL-VISION
sudo apt-get install build-essential libqjson-dev libode-dev libboost-dev libgl1-mesa-dev freeglut3-dev #PARSIAN AI
sudo apt-get install libprotobuf-dev protobuf-compiler protobuf-c-compiler libprotoc-dev #Protobuf

# INSTALL VARTYPES
mkdir temp
cd temp
git clone https://gitlab.com/ParsianSoftware/vartypes-parsian.git
cd vartypes-parsian/
cd vartypes-0.6/
mkdir build && cd build
cmake ..
make -j 4
sudo make install
cd ..
cd vartypes-0.7/
mkdir build && cd build
cmake ..
make -j 4
sudo make install
cd ..
cd ..
cd ..
pwd
rm -rf temp
