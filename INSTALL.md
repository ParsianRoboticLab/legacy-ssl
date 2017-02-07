# Parsian SSL AI - INSTALL

## Overveiw

We developed AI on Ubuntu OS. (Ubuntu 14.04+ tested and is recommended). It is  important that the graphics card driver is installed properly (the official Ubuntu packages for nVidia and AMD(ATI) graphics cards are available). AI will compile and run in both 32 and 64 bits Linux.

AI is written in C++, in order to compile it, you will need a working toolchain and a c++ compiler.


## Dependencies

AI depends on:

- [OpenGL](https://www.opengl.org)
- [Qt4 Development Libraries](https://www.qt.io) version 4.8+
- [VarTypes Library](https://github.com/szi/vartypes)
- [Google Protobuf](https://github.com/google/protobuf)
- [Boost development libraries](http://www.boost.org/) (needed by VarTypes)


### Linux/Unix Installation
-------------

Firstly we update our current packages by :
```bash
sudo apt-get update
```

Secondly we install the dependencies that are in Ubuntu servers by default :
``` bash
sudo apt-get install g++ libqt4-dev libeigen3-dev libdc1394-22 libdc1394-22-dev cmake libv4l-0 build-essential libqjson-dev libode-dev libboost-dev libgl1-mesa-dev freeglut3-dev git libprotobuf-dev libprotoc-dev
```
The we can start installing **Vartypes**

1.  Download [Vartypes0.6](/uploads/cd33c97c934ee345540870336678f032/vartypes-0.6.tar.gz)
 and [Vartypes0.7](/uploads/bfe2878333b4bc8e319209aaff8c7613/vartypes-0.7.tar.gz)
2.  Extract them into your Desktop Folder
3.  Open Terminal <kbd>Ctrl + Alt + t </kbd>
4.  Follow below code : (It builds and installs VarTypes libraries)
``` bash 

cd ~/Desktop
cd vartypes-0.6
sudo make -j 8
sudo make install
cd ..
cd vartypes-0.7
sudo make
sudo make install
cd ..
```

Install Qt
-------------------

As  **A.I.** is build in Qt4 so you should download and install Qt4. 

> **Tip:** It's recommended to install Qt4.8.6 or at least 4.7.4 version.

you can download Qt4.8.6 from here or in Qt official website.

1 . Give the installer application permission to run.
``` bash
sudo chmod +x Qt4.8.6_Offline.run
```
2 . Run Installer and just press next, next  ... Finish ;-) 

``` bash
sudo ./Qt4.8.6_Offline.run

```
3 . Enjoy !

> **Tip:** 

> - Also it's recommended to install latest version Qt5.
> - Install [Oh-My-Zsh](https://github.com/robbyrussell/oh-my-zsh) for better git access 

Finish :raised_hand:

## Notes on the performance

When running AI, check the **Command** in the status bar. If it is running at **60 FPS** or higher, everything is ok. Otherwise check the graphics card's driver installation and OpenGL settings.
**Also check CPU lack**

Other Applications
-------------


> **Note:** This application complete the project for stimulating and making plans. 
> 
> - GrSim (Graphical Simulator)
> - ssl-visual-planner (Make Plan for Offensive situations)
> - ssl-fedit


