# PicTalk

this is the the PicTalk software, an attempt to provide an "all in one" software to enable reception and decoding of PicSat telemetry and scientific data.

Current version (as of 19th of Feb 2018) supports SDRPlay RSP1/A and RTLSDR. Funcube is not working properly (frequency management issue) and will be added soon.


This sotware requires Python 3.5/3.6 to be installed with the following Python Packages :
- Scipy
- Numpy
- ZMQ

Depending on selected python version you want to use, you have to add to your system :
- python3.6-dev or python3.5-dev
- libpython3.6-dev or libpython3.5-dev

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

### Required librairies :
- libusb-1.0-0-dev 
- libfftw3-dev
- libczmq-dev
- libhidapi-dev

Clone the repository, then from the folder :

git clone https://github.com/f4gkr/PicTalk.git
Cloning into 'PicTalk'...

- Then cd PicTalk
- generate makefile : 
   qmake
- compile
   make

the project file pictalk.pro is set to support python3.6. If you want to use a specific version, you have to update the following lines in the project file :

- LIBS += $$system("python3.6-config --libs")
- QMAKE_CFLAGS += $$system("python3.6-config --cflags")
- INCLUDEPATH += $$system("python3.6-config --includes |cut -c 3-")


# Run application:
By default the binary is stored in ./bin folder and requires the ./python folder to be present (the python folder contains the frame decoder)

Binary name : ./bin/pictalk

For more information, please visit https://picsat.obspm.fr/
