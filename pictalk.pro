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



win32 {
    # pkg-config --cflags --libs python3
    INCLUDEPATH += C:/msys64/usr/include
    LIBS += -lhidapi

    # for python embed, see https://docs.python.org/3/extending/embedding.html
    # paragraph 1.6. Compiling and Linking under Unix-like systems
    #INCLUDEPATH += C:/Python36/include
    #QMAKE_LFLAGS += -LC:/Python36/libs
    #QMAKE_CFLAGS += -fwrapv -D__USE_MINGW_ANSI_STDIO=1  -DNDEBUG  -DNDEBUG
    #LIBS += -lpython36  -lversion -lm
    #DEFINES += D_hypot=hypot
    DESTDIR = /msys64/home/sylvain/code/pictalk_bindist
}

linux {
    LIBS += -lgps -lhidapi-hidraw -L/usr/lib/python3.6/config-3.6m-x86_64-linux-gnu -L/usr/lib -lpython3.6m -lpthread -ldl  -lutil -lm  -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions
    QMAKE_CFLAGS += -fdebug-prefix-map=/build/python3.6-fWqO4P/python3.6-3.6.1=. -fstack-protector-strong
    INCLUDEPATH += /usr/include/python3.6m
    DESTDIR = $$PWD/bin
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
    ui/ledindicator.cpp \
    dsp/pythondecoder.cpp \
    webinterface/webservice.cpp \
    common/constants.cpp \
    common/QLogger.cpp \
    common/samplefifo.cpp \
    common/tuningpolicy.cpp \
    core/controller.cpp \
    core/sampleblock.cpp \
    dsp/frameprocessor.cpp \
    dsp/overlapsave.cpp \
    dsp/pythondecoder.cpp \
    dsp/zmqserver.cpp \
    hardware/audio/audioinput.cpp \
    hardware/funcube/fcdwidget.cpp \
    hardware/funcube/funcube.cpp \
    hardware/windows/tinygps.cpp \
    hardware/gpdsd.cpp \
    hardware/miricscpp.cpp \
    hardware/rtlsdr.cpp \
    hardware/rxdevice.cpp \
    hardware/rxhardwareselector.cpp \
    httpserver/httpconnectionhandler.cpp \
    httpserver/httpconnectionhandlerpool.cpp \
    httpserver/httpcookie.cpp \
    httpserver/httpglobal.cpp \
    httpserver/httplistener.cpp \
    httpserver/httprequest.cpp \
    httpserver/httprequesthandler.cpp \
    httpserver/httpresponse.cpp \
    httpserver/httpsession.cpp \
    httpserver/httpsessionstore.cpp \
    httpserver/staticfilecontroller.cpp \
    qwt/qwt_abstract_legend.cpp \
    qwt/qwt_abstract_scale.cpp \
    qwt/qwt_abstract_scale_draw.cpp \
    qwt/qwt_abstract_slider.cpp \
    qwt/qwt_analog_clock.cpp \
    qwt/qwt_arrow_button.cpp \
    qwt/qwt_clipper.cpp \
    qwt/qwt_color_map.cpp \
    qwt/qwt_column_symbol.cpp \
    qwt/qwt_compass.cpp \
    qwt/qwt_compass_rose.cpp \
    qwt/qwt_counter.cpp \
    qwt/qwt_curve_fitter.cpp \
    qwt/qwt_date.cpp \
    qwt/qwt_date_scale_draw.cpp \
    qwt/qwt_date_scale_engine.cpp \
    qwt/qwt_dial.cpp \
    qwt/qwt_dial_needle.cpp \
    qwt/qwt_dyngrid_layout.cpp \
    qwt/qwt_event_pattern.cpp \
    qwt/qwt_graphic.cpp \
    qwt/qwt_interval.cpp \
    qwt/qwt_interval_symbol.cpp \
    qwt/qwt_knob.cpp \
    qwt/qwt_legend.cpp \
    qwt/qwt_legend_data.cpp \
    qwt/qwt_legend_label.cpp \
    qwt/qwt_magnifier.cpp \
    qwt/qwt_math.cpp \
    qwt/qwt_matrix_raster_data.cpp \
    qwt/qwt_null_paintdevice.cpp \
    qwt/qwt_painter.cpp \
    qwt/qwt_painter_command.cpp \
    qwt/qwt_panner.cpp \
    qwt/qwt_picker.cpp \
    qwt/qwt_picker_machine.cpp \
    qwt/qwt_pixel_matrix.cpp \
    qwt/qwt_plot.cpp \
    qwt/qwt_plot_abstract_barchart.cpp \
    qwt/qwt_plot_axis.cpp \
    qwt/qwt_plot_barchart.cpp \
    qwt/qwt_plot_canvas.cpp \
    qwt/qwt_plot_curve.cpp \
    qwt/qwt_plot_dict.cpp \
    qwt/qwt_plot_directpainter.cpp \
    qwt/qwt_plot_grid.cpp \
    qwt/qwt_plot_histogram.cpp \
    qwt/qwt_plot_intervalcurve.cpp \
    qwt/qwt_plot_item.cpp \
    qwt/qwt_plot_layout.cpp \
    qwt/qwt_plot_legenditem.cpp \
    qwt/qwt_plot_magnifier.cpp \
    qwt/qwt_plot_marker.cpp \
    qwt/qwt_plot_multi_barchart.cpp \
    qwt/qwt_plot_panner.cpp \
    qwt/qwt_plot_picker.cpp \
    qwt/qwt_plot_rasteritem.cpp \
    qwt/qwt_plot_renderer.cpp \
    qwt/qwt_plot_rescaler.cpp \
    qwt/qwt_plot_scaleitem.cpp \
    qwt/qwt_plot_seriesitem.cpp \
    qwt/qwt_plot_shapeitem.cpp \
    qwt/qwt_plot_spectrocurve.cpp \
    qwt/qwt_plot_spectrogram.cpp \
    qwt/qwt_plot_svgitem.cpp \
    qwt/qwt_plot_textlabel.cpp \
    qwt/qwt_plot_tradingcurve.cpp \
    qwt/qwt_plot_xml.cpp \
    qwt/qwt_plot_zoneitem.cpp \
    qwt/qwt_plot_zoomer.cpp \
    qwt/qwt_point_3d.cpp \
    qwt/qwt_point_data.cpp \
    qwt/qwt_point_mapper.cpp \
    qwt/qwt_point_polar.cpp \
    qwt/qwt_raster_data.cpp \
    qwt/qwt_round_scale_draw.cpp \
    qwt/qwt_sampling_thread.cpp \
    qwt/qwt_scale_div.cpp \
    qwt/qwt_scale_draw.cpp \
    qwt/qwt_scale_engine.cpp \
    qwt/qwt_scale_map.cpp \
    qwt/qwt_scale_widget.cpp \
    qwt/qwt_series_data.cpp \
    qwt/qwt_slider.cpp \
    qwt/qwt_spline.cpp \
    qwt/qwt_symbol.cpp \
    qwt/qwt_system_clock.cpp \
    qwt/qwt_text.cpp \
    qwt/qwt_text_engine.cpp \
    qwt/qwt_text_label.cpp \
    qwt/qwt_thermo.cpp \
    qwt/qwt_transform.cpp \
    qwt/qwt_wheel.cpp \
    qwt/qwt_widget_overlay.cpp \
    ui/bookmarks.cpp \
    ui/freqctrl.cpp \
    ui/gkdial.cpp \
    ui/gkpushbutton.cpp \
    ui/indicatorwidget.cpp \
    ui/ledindicator.cpp \
    ui/plotter.cpp \
    ui/qcustomplot.cpp \
    ui/spectrumplot.cpp \
    webinterface/webservice.cpp \
    main.cpp \
    mainwindow.cpp \
    hardware/mirisdr/src/convert/252_s16.c \
    hardware/mirisdr/src/convert/336_s16.c \
    hardware/mirisdr/src/convert/384_s16.c \
    hardware/mirisdr/src/convert/504_s16.c \
    hardware/mirisdr/src/convert/504_s8.c \
    hardware/mirisdr/src/convert/base.c \
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
    hardware/rtlsdr/librtlsdr.c \
    hardware/rtlsdr/tuner_e4k.c \
    hardware/rtlsdr/tuner_fc0012.c \
    hardware/rtlsdr/tuner_fc0013.c \
    hardware/rtlsdr/tuner_fc2580.c \
    hardware/rtlsdr/tuner_r82xx.c \
    hardware/windows/rs232.c

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
    ui/ledindicator.h \
    dsp/pythondecoder.h \
    webinterface/webservice.h \
    common/constants.h \
    common/datatypes.h \
    common/QLogger.h \
    common/samplefifo.h \
    common/tuningpolicy.h \
    core/controller.h \
    core/sampleblock.h \
    dsp/frameprocessor.h \
    dsp/overlapsave.h \
    dsp/pythondecoder.h \
    dsp/zmqserver.h \
    hardware/audio/audioinput.h \
    hardware/funcube/fcdhidcmd.h \
    hardware/funcube/fcdwidget.h \
    hardware/funcube/funcube.h \
    hardware/mirisdr/include/mirisdr.h \
    hardware/mirisdr/include/mirisdr_export.h \
    hardware/mirisdr/src/async.h \
    hardware/mirisdr/src/constants.h \
    hardware/mirisdr/src/gain.h \
    hardware/mirisdr/src/hard.h \
    hardware/mirisdr/src/soft.h \
    hardware/mirisdr/src/structs.h \
    hardware/rtlsdr/reg_field.h \
    hardware/rtlsdr/rtl-sdr.h \
    hardware/rtlsdr/rtlsdr_i2c.h \
    hardware/rtlsdr/tuner_e4k.h \
    hardware/rtlsdr/tuner_fc0012.h \
    hardware/rtlsdr/tuner_fc0013.h \
    hardware/rtlsdr/tuner_fc2580.h \
    hardware/rtlsdr/tuner_r82xx.h \
    hardware/windows/rs232.h \
    hardware/windows/tinygps.h \
    hardware/gpdsd.h \
    hardware/miricscpp.h \
    hardware/rtlsdr.h \
    hardware/rxdevice.h \
    hardware/rxhardwareselector.h \
    httpserver/httpconnectionhandler.h \
    httpserver/httpconnectionhandlerpool.h \
    httpserver/httpcookie.h \
    httpserver/httpglobal.h \
    httpserver/httplistener.h \
    httpserver/httprequest.h \
    httpserver/httprequesthandler.h \
    httpserver/httpresponse.h \
    httpserver/httpsession.h \
    httpserver/httpsessionstore.h \
    httpserver/staticfilecontroller.h \
    qwt/qwt.h \
    qwt/qwt_abstract_legend.h \
    qwt/qwt_abstract_scale.h \
    qwt/qwt_abstract_scale_draw.h \
    qwt/qwt_abstract_slider.h \
    qwt/qwt_analog_clock.h \
    qwt/qwt_arrow_button.h \
    qwt/qwt_clipper.h \
    qwt/qwt_color_map.h \
    qwt/qwt_column_symbol.h \
    qwt/qwt_compass.h \
    qwt/qwt_compass_rose.h \
    qwt/qwt_compat.h \
    qwt/qwt_counter.h \
    qwt/qwt_curve_fitter.h \
    qwt/qwt_date.h \
    qwt/qwt_date_scale_draw.h \
    qwt/qwt_date_scale_engine.h \
    qwt/qwt_dial.h \
    qwt/qwt_dial_needle.h \
    qwt/qwt_dyngrid_layout.h \
    qwt/qwt_event_pattern.h \
    qwt/qwt_global.h \
    qwt/qwt_graphic.h \
    qwt/qwt_interval.h \
    qwt/qwt_interval_symbol.h \
    qwt/qwt_knob.h \
    qwt/qwt_legend.h \
    qwt/qwt_legend_data.h \
    qwt/qwt_legend_label.h \
    qwt/qwt_magnifier.h \
    qwt/qwt_math.h \
    qwt/qwt_matrix_raster_data.h \
    qwt/qwt_null_paintdevice.h \
    qwt/qwt_painter.h \
    qwt/qwt_painter_command.h \
    qwt/qwt_panner.h \
    qwt/qwt_picker.h \
    qwt/qwt_picker_machine.h \
    qwt/qwt_pixel_matrix.h \
    qwt/qwt_plot.h \
    qwt/qwt_plot_abstract_barchart.h \
    qwt/qwt_plot_barchart.h \
    qwt/qwt_plot_canvas.h \
    qwt/qwt_plot_curve.h \
    qwt/qwt_plot_dict.h \
    qwt/qwt_plot_directpainter.h \
    qwt/qwt_plot_grid.h \
    qwt/qwt_plot_histogram.h \
    qwt/qwt_plot_intervalcurve.h \
    qwt/qwt_plot_item.h \
    qwt/qwt_plot_layout.h \
    qwt/qwt_plot_legenditem.h \
    qwt/qwt_plot_magnifier.h \
    qwt/qwt_plot_marker.h \
    qwt/qwt_plot_multi_barchart.h \
    qwt/qwt_plot_panner.h \
    qwt/qwt_plot_picker.h \
    qwt/qwt_plot_rasteritem.h \
    qwt/qwt_plot_renderer.h \
    qwt/qwt_plot_rescaler.h \
    qwt/qwt_plot_scaleitem.h \
    qwt/qwt_plot_seriesitem.h \
    qwt/qwt_plot_shapeitem.h \
    qwt/qwt_plot_spectrocurve.h \
    qwt/qwt_plot_spectrogram.h \
    qwt/qwt_plot_svgitem.h \
    qwt/qwt_plot_textlabel.h \
    qwt/qwt_plot_tradingcurve.h \
    qwt/qwt_plot_zoneitem.h \
    qwt/qwt_plot_zoomer.h \
    qwt/qwt_point_3d.h \
    qwt/qwt_point_data.h \
    qwt/qwt_point_mapper.h \
    qwt/qwt_point_polar.h \
    qwt/qwt_raster_data.h \
    qwt/qwt_round_scale_draw.h \
    qwt/qwt_samples.h \
    qwt/qwt_sampling_thread.h \
    qwt/qwt_scale_div.h \
    qwt/qwt_scale_draw.h \
    qwt/qwt_scale_engine.h \
    qwt/qwt_scale_map.h \
    qwt/qwt_scale_widget.h \
    qwt/qwt_series_data.h \
    qwt/qwt_series_store.h \
    qwt/qwt_slider.h \
    qwt/qwt_spline.h \
    qwt/qwt_symbol.h \
    qwt/qwt_system_clock.h \
    qwt/qwt_text.h \
    qwt/qwt_text_engine.h \
    qwt/qwt_text_label.h \
    qwt/qwt_thermo.h \
    qwt/qwt_transform.h \
    qwt/qwt_wheel.h \
    qwt/qwt_widget_overlay.h \
    ui/bookmarks.h \
    ui/freqctrl.h \
    ui/gkdial.h \
    ui/gkpushbutton.h \
    ui/indicatorwidget.h \
    ui/ledindicator.h \
    ui/plotter.h \
    ui/qcustomplot.h \
    ui/spectrumplot.h \
    webinterface/webservice.h \
    mainwindow.h

RESOURCES += \
    resources/picsat.qrc

OTHER_FILES +=

DISTFILES += \
    bin/python/decodeZ2.py
