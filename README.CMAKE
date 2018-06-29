In order to build with CMake on Windows, do the following:

NOTE: Only Win64 build is supported

1. Download and install Visual Studio 2015 (only that build is supported)
2. Download and install CMake for the Windows platform
3. Start CMake point it to the location where the source code is located and choose folder where you want to build generated Visual Studio soludions
4. Press "Gonfigurate" inside the cMake
5. Press "Generate" inside the cMake
6. After all that just press "Opne Project" and the CMake will open Visual Studio 2015 with generated solution files for the building
	
In order to build with CMake on Linux, do the following:

NOTE: Only x86-64 builds are supported

1. You need to download all the libraries for the building (this is example for the Ubuntu/Kubuntu)

sudo apt-get install cmake-curses-gui
sudo apt-get install g++
sudo apt-get install libglew-dev
sudo apt-get install libjpeg-dev
sudo apt-get install libsdl1.2-dev
sudo apt-get install libcurl4-openssl-dev
sudo apt-get install opencl-dev
sudo apt-get install mysql-dev
sudo apt-get install ncurses-dev
sudo apt-get install mysql-client
sudo apt-get install libmysqlclient-dev
sudo apt-get install libfreetype6-dev

2. Type "ccmake ." - you will have only one files "empty cache"
3. Press "c" to configure
4. After all that, CMake will produce some output where it will write where your library and include files are located. Press "e" to exit help
5. Press "c" again to configure and "e" to exit help
6. Press "g" to generate the Makefile.
7. type "make" to compile

