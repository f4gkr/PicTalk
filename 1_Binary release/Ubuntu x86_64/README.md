# Binary Install for Ubuntu x86_64

** WARNING ** this is a preliminary release please feedback any issue !

Works with **RTLSDR and SDRPlay RSP1/RSP1A ONLY** (RSP2 not supported !).

the package will add udev rules for RTLSDR and RSP, will blacklist the following modules:

- blacklist dvb_usb_rtl28xxu
- blacklist dvb_usb_v2
- blacklist rtl_2830
- blacklist rtl_2832
- blacklist r820t

The source code contains the LibMirics SDR and RTLSDR libraries to reduce external dependencies.

Installing
- sudo dpkg -i ./pictalk_1.0.0_amd64.deb

this will complain with missing packages, then :

- sudo apt-get -f install

this will add the missing packages. 

Depends: libc6 , libusb-1.0-0, libfftw3-3, qt5-default (>= 5.5), libzmq3-dev, libhidapi-hidraw0, python3, python3-zmq, libqt5svg5, python3-scipy, python3-numpy, libpython3.6-dev



Removing:
- sudo dpkg -r pictalk


