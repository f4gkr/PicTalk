# Binary Install for Ubuntu x86_64

Installing
- sudo dpkg -i ./pictalk_1.0.0_amd64.deb

this will complain with missing packages, then :

- sudo apt-get -f install

this will add the missing packages. 

Depends: libc6 , libusb-1.0-0, libfftw3-3, qt5-default (>= 5.5), libzmq3-dev, libhidapi-hidraw0, python3, python3-zmq, libqt5svg5, python3-scipy, python3-numpy, libpython3.6-dev



Removing:
- sudo dpkg -r pictalk


