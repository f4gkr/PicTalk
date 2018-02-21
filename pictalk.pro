#==========================================================================================
# + + +   This Software is released under the "Simplified BSD License"  + + +
# Copyright F4GKR Sylvain AZARIAN . All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without modification, are
#   permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright notice, this list of
#	  conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright notice, this list
#	  of conditions and the following disclaimer in the documentation and/or other materials
#	  provided with the distribution.
#
#   THIS SOFTWARE IS PROVIDED BY Sylvain AZARIAN F4GKR ``AS IS'' AND ANY EXPRESS OR IMPLIED
#   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
#   FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Sylvain AZARIAN OR
#   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
#   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
#==========================================================================================
#
TARGET = pictalk
TEMPLATE = app

QT       += core gui multimedia
DEFINES += BUILD_DATE='"\\\"$(shell  date +\"%Y%m%d\")\\\""'

include( httpserver/httpserver.pri)
include( qwt/qwt.pri )

LIBS +=  -lusb-1.0 -lpthread -lfftw3f -lm -lzmq
QMAKE_CXXFLAGS += -std=c++11


win32 {
    # pkg-config --cflags --libs python3
    INCLUDEPATH += C:/msys64/usr/include
    LIBS += -lhidapi

    # for python embed, see https://docs.python.org/3/extending/embedding.html
    # paragraph 1.6. Compiling and Linking under Unix-like systems
    INCLUDEPATH += C:/msys64/mingw64/include/python3.6m
    QMAKE_LFLAGS += -LC:/msys64/mingw64/lib
    LIBS += -lpython3.6m  -lversion -lm
    DESTDIR = /msys64/home/sylvain/code/pictalk_bindist
}

linux {
    LIBS += $$system("python3.5-config --libs")
    QMAKE_CFLAGS += $$system("python3.5-config --cflags")
    INCLUDEPATH += $$system("python3.5-config --includes |cut -c 3-")
    DESTDIR = $$PWD/bin
    contains( QMAKE_HOST.arch, arm.* ):{
        #specific instructions for RPiCompile

    } else {
        # generic Linux compile...

    }
    LIBS += -lhidapi-hidraw
}
#https://stackoverflow.com/questions/42620074/gprof-produces-empty-output
#    QMAKE_CXXFLAGS += -pg -no-pie
#    QMAKE_CFLAGS += -pg -no-pie
#    QMAKE_LFLAGS += -pg -no-pie

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    ui/spectrumplot.cpp \
    ui/qcustomplot.cpp \
    ui/plotter.cpp \
    ui/ledindicator.cpp \
    ui/indicatorwidget.cpp \
    ui/gkpushbutton.cpp \
    ui/gkdial.cpp \
    ui/freqctrl.cpp \
    ui/bookmarks.cpp \
    dsp/zmqserver.cpp \
    dsp/pythondecoder.cpp \
    dsp/overlapsave.cpp \
    dsp/frameprocessor.cpp \
    core/sampleblock.cpp \
    core/controller.cpp \
    common/tuningpolicy.cpp \
    common/samplefifo.cpp \
    common/QLogger.cpp \
    common/constants.cpp \
    hardware/rxhardwareselector.cpp \
    hardware/rxdevice.cpp \
    hardware/rtlsdr.cpp \
    hardware/miricscpp.cpp \ 
    hardware/rtlsdr/tuner_r82xx.c \
    hardware/rtlsdr/tuner_fc2580.c \
    hardware/rtlsdr/tuner_fc0013.c \
    hardware/rtlsdr/tuner_fc0012.c \
    hardware/rtlsdr/tuner_e4k.c \
    hardware/rtlsdr/librtlsdr.c \
    hardware/mirisdr/src/convert/base.c \
    hardware/mirisdr/src/convert/504_s16.c \
    hardware/mirisdr/src/convert/504_s8.c \
    hardware/mirisdr/src/convert/384_s16.c \
    hardware/mirisdr/src/convert/336_s16.c \
    hardware/mirisdr/src/convert/252_s16.c \
    hardware/mirisdr/src/sync.c \
    hardware/mirisdr/src/streaming.c \
    hardware/mirisdr/src/soft.c \
    hardware/mirisdr/src/reg.c \
    hardware/mirisdr/src/libmirisdr.c \
    hardware/mirisdr/src/hard.c \
    hardware/mirisdr/src/gain.c \
    hardware/mirisdr/src/devices.c \
    hardware/mirisdr/src/async.c \
    hardware/mirisdr/src/adc.c \
    hardware/funcube/funcube.cpp \
    hardware/funcube/fcdwidget.cpp \
    hardware/audio/audioinput.cpp \
    webinterface/webservice.cpp

HEADERS  += \
    mainwindow.h \
    ui/spectrumplot.h \
    ui/qcustomplot.h \
    ui/plotter.h \
    ui/ledindicator.h \
    ui/indicatorwidget.h \
    ui/gkpushbutton.h \
    ui/gkdial.h \
    ui/freqctrl.h \
    ui/bookmarks.h \
    dsp/zmqserver.h \
    dsp/pythondecoder.h \
    dsp/overlapsave.h \
    dsp/frameprocessor.h \
    core/sampleblock.h \
    core/controller.h \
    common/tuningpolicy.h \
    common/samplefifo.h \
    common/QLogger.h \
    common/datatypes.h \
    common/constants.h \
    hardware/rxhardwareselector.h \
    hardware/rxdevice.h \
    hardware/rtlsdr.h \
    hardware/miricscpp.h \
    hardware/rtlsdr/tuner_r82xx.h \
    hardware/rtlsdr/tuner_fc2580.h \
    hardware/rtlsdr/tuner_fc0013.h \
    hardware/rtlsdr/tuner_fc0012.h \
    hardware/rtlsdr/tuner_e4k.h \
    hardware/rtlsdr/rtlsdr_i2c.h \
    hardware/rtlsdr/rtl-sdr.h \
    hardware/rtlsdr/reg_field.h \
    hardware/mirisdr/src/structs.h \
    hardware/mirisdr/src/soft.h \
    hardware/mirisdr/src/hard.h \
    hardware/mirisdr/src/gain.h \
    hardware/mirisdr/src/constants.h \
    hardware/mirisdr/src/async.h \
    hardware/mirisdr/include/mirisdr_export.h \
    hardware/mirisdr/include/mirisdr.h \
    hardware/funcube/funcube.h \
    hardware/funcube/fcdwidget.h \
    hardware/funcube/fcdhidcmd.h \
    hardware/audio/audioinput.h \
    webinterface/webservice.h


RESOURCES += \
    resources/picsat.qrc

OTHER_FILES +=

DISTFILES += \
    bin/python/decodeZ3.py \
    PackagingNotes.txt

