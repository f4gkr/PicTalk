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


include( qwt/qwt.pri )
include( httpserver/httpserver.pri)

LIBS +=  -lusb-1.0 -lpthread -lfftw3f -lm -lzmq

win32 {
    INCLUDEPATH += C:/msys64/usr/include
    LIBS += -lhidapi
}

linux {
    LIBS += -lgps -lhidapi-hidraw
    contains( QMAKE_HOST.arch, arm.* ):{
        #specific instructions for RPiCompile

    } else {
        # generic Linux compile...

    }
}


SOURCES += \
    main.cpp\
    mainwindow.cpp \
    ui/freqctrl.cpp \
    hardware/rtlsdr.cpp \
    common/QLogger.cpp \
    common/samplefifo.cpp \
    ui/spectrumplot.cpp \
    core/controller.cpp \
    dsp/overlapsave.cpp \
    ui/indicatorwidget.cpp \
    ui/qcustomplot.cpp \
    hardware/gpdsd.cpp \
    hardware/windows/tinygps.cpp \
    hardware/windows/rs232.c \
    ui/gkdial.cpp \
    dsp/frameprocessor.cpp \
    core/sampleblock.cpp \
    common/constants.cpp \
    ui/plotter.cpp \
    ui/bookmarks.cpp \
    webinterface/webservice.cpp \
    common/tuningpolicy.cpp \
    dsp/zmqserver.cpp \
    hardware/funcube/fcdwidget.cpp \
    hardware/funcube/funcube.cpp \
    hardware/rxdevice.cpp \
    hardware/audio/audioinput.cpp \
    hardware/rxhardwareselector.cpp \
    hardware/rtlsdr/librtlsdr.c \
    hardware/rtlsdr/tuner_e4k.c \
    hardware/rtlsdr/tuner_fc0012.c \
    hardware/rtlsdr/tuner_fc0013.c \
    hardware/rtlsdr/tuner_fc2580.c \
    hardware/rtlsdr/tuner_r82xx.c \
    hardware/mirisdr/src/adc.c \
    hardware/mirisdr/src/async.c \
    hardware/mirisdr/src/devices.c \
    hardware/mirisdr/src/gain.c \
    hardware/mirisdr/src/hard.c \
    hardware/mirisdr/src/libmirisdr.c \
    hardware/mirisdr/src/reg.c \
    hardware/mirisdr/src/soft.c \
    hardware/mirisdr/src/streaming.c \
    hardware/mirisdr/src/sync.c \
    hardware/mirisdr/src/convert/252_s16.c \
    hardware/mirisdr/src/convert/336_s16.c \
    hardware/mirisdr/src/convert/384_s16.c \
    hardware/mirisdr/src/convert/504_s8.c \
    hardware/mirisdr/src/convert/504_s16.c \
    hardware/mirisdr/src/convert/base.c \
    hardware/miricscpp.cpp \
    ui/gkpushbutton.cpp \
    ui/ledindicator.cpp

HEADERS  += \
    mainwindow.h \
    ui/freqctrl.h \
    hardware/rtlsdr.h \
    common/QLogger.h \
    common/constants.h \
    common/samplefifo.h \
    ui/spectrumplot.h \
    core/controller.h \
    dsp/overlapsave.h \
    ui/indicatorwidget.h \
    ui/qcustomplot.h \
    hardware/gpdsd.h \
    hardware/windows/rs232.h \
    hardware/windows/tinygps.h \
    ui/gkdial.h \
    dsp/frameprocessor.h \
    core/sampleblock.h \
    ui/plotter.h \
    ui/bookmarks.h \
    webinterface/webservice.h \
    common/datatypes.h \
    common/tuningpolicy.h \
    dsp/zmqserver.h \
    hardware/funcube/fcdhidcmd.h \
    hardware/funcube/fcdwidget.h \
    hardware/funcube/funcube.h \
    hardware/rxdevice.h \
    hardware/audio/audioinput.h \
    hardware/rxhardwareselector.h \
    hardware/rtlsdr/reg_field.h \
    hardware/rtlsdr/rtl-sdr.h \
    hardware/rtlsdr/rtlsdr_i2c.h \
    hardware/rtlsdr/tuner_e4k.h \
    hardware/rtlsdr/tuner_fc0012.h \
    hardware/rtlsdr/tuner_fc0013.h \
    hardware/rtlsdr/tuner_fc2580.h \
    hardware/rtlsdr/tuner_r82xx.h \
    hardware/mirisdr/src/async.h \
    hardware/mirisdr/src/constants.h \
    hardware/mirisdr/src/gain.h \
    hardware/mirisdr/src/hard.h \
    hardware/mirisdr/src/soft.h \
    hardware/mirisdr/src/structs.h \
    hardware/mirisdr/include/mirisdr.h \
    hardware/mirisdr/include/mirisdr_export.h \
    hardware/miricscpp.h \
    ui/gkpushbutton.h \
    ui/ledindicator.h

RESOURCES += \
    resources/picsat.qrc
