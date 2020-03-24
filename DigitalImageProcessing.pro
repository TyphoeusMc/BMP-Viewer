#-------------------------------------------------
#
# Project created by QtCreator 2018-09-27T13:00:43
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = DigitalImageProcessing
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    bmpinfo.cpp \
    colorpalette.cpp \
    inputnewname.cpp \
    newcolor.cpp \
    newcolor8bit.cpp \
    interpolation.cpp \
    gaussiansmooth.cpp \
    about.cpp \
    addimpulsivenoise.cpp \
    histogram.cpp \
    qcustomplot.cpp \
    bilateralfilter.cpp

HEADERS += \
        mainwindow.h \
    bmpinfo.h \
    colorpalette.h \
    inputnewname.h \
    newcolor.h \
    newcolor8bit.h \
    interpolation.h \
    gaussiansmooth.h \
    bmpimagestructure.h \
    about.h \
    addimpulsivenoise.h \
    histogram.h \
    qcustomplot.h \
    bilateralfilter.h

FORMS += \
        mainwindow.ui \
    bmpinfo.ui \
    colorpalette.ui \
    inputnewname.ui \
    newcolor.ui \
    newcolor8bit.ui \
    interpolation.ui \
    gaussiansmooth.ui \
    about.ui \
    addimpulsivenoise.ui \
    histogram.ui \
    bilateralfilter.ui
