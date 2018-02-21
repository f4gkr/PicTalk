# PicTalk

this is the the PicTalk software, an attempt to provide an "all in one" software to enable reception and decoding of PicSat telemetry and scientific data.

Current version (as of 21st of Feb 2018) supports SDRPlay RSP1/A and RTLSDR. Funcube is not working properly (frequency management issue) and will be added soon.


This sotware requires Python 3.5  to be installed with the following Python Packages :
- Scipy
- Numpy
- ZMQ
- Requets

To have the relevant packages, do:
- sudo apt-get install python3-pip
- pip3 install scipy numpy requests zmq


This application is split in two parts:
- The core Qt C++ program manages the SDR device and extracts the sub-band of interest,
- It scans the RF channel to estimate potential transmission from satellite
- the IQ Samples are sent to the Python code in charge of demodulation and building the KISS AX25 Frame
- The Python code sends back to C++ the decoded frame for local storage and display.

To compile this program you need to check that you have the correct Python installed, and in particular the following packages:
### Required Qt Modules :
- qt5-default
- libqt5svg5-dev
- qtmultimedia5-dev
- libqt5opengl5-dev

### Required librairies :
- libusb-1.0-0-dev 
- libfftw3-dev
- libczmq-dev
- libhidapi-dev

To have all the required packages :
sudo apt-get install libqt5svg5-dev qtmultimedia5-dev libqt5opengl5-dev libusb-1.0-0-dev libfftw3-dev libczmq-dev libhidapi-dev 

Clone the repository, then from the folder :

git clone https://github.com/f4gkr/PicTalk.git
Cloning into 'PicTalk'...

- Then cd PicTalk
- generate makefile : 
   qmake
- compile
   make

the project file pictalk.pro is set to support python3.5. If you want to use a specific version, you have to update the following lines in the project file :

- LIBS += $$system("python3.5-config --libs")
- QMAKE_CFLAGS += $$system("python3.5-config --cflags")
- INCLUDEPATH += $$system("python3.5-config --includes |cut -c 3-")


# Run application:
By default the binary is stored in ./bin folder and requires the ./python folder to be present (the python folder contains the frame decoder)

Binary name : ./bin/pictalk

It will create a folder named "pictalk" in your home. This folder contains several files, and in particular the **pictalkframes.txt** that will contain all the decoded PicSat frames.

For more information, please visit https://picsat.obspm.fr/
