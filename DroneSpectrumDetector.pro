QT += core gui widgets

CONFIG += c++11
TEMPLATE = app
TARGET = DroneSpectrumDetector

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    spectrumsimulator.cpp \
    spectrumanalyzer.cpp \
    spectrumplotwidget.cpp \
    waterfallwidget.cpp

HEADERS += \
    mainwindow.h \
    spectrumsimulator.h \
    spectrumanalyzer.h \
    spectrumplotwidget.h \
    waterfallwidget.h

FORMS += \
    mainwindow.ui
