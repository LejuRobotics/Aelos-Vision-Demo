1#-------------------------------------------------
#
# Project created by QtCreator 2017-05-26T18:44:40
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ColorSegmentation
TEMPLATE = app

INCLUDEPATH += $$PWD/Public

SOURCES += main.cpp\
    VideoArea.cpp \
    Public/painterlabel.cpp\
    Public/ScanIpDiaog.cpp \
    Public/ScanIpThread.cpp \
    Public/PortSetupDialog.cpp \
    Public/ServerWifiSettings.cpp \
    Public/ConnectionBox.cpp

HEADERS += VideoArea.h\
    Public/painterlabel.h\
    precompiled.h \
    Public/ScanIpDiaog.h \
    Public/ScanIpThread.h \
    Public/PortSetupDialog.h \
    Public/ServerWifiSettings.h \
    Public/ConnectionBox.h

FORMS    +=  VideoArea.ui\
    Public/ScanIpDiaog.ui \
    Public/PortSetupDialog.ui \
    Public/serverwifisettings.ui \
    Public/ConnectionBox.ui

PRECOMPILED_HEADER = $$PWD/precompiled.h

MOC_DIR += ./moc
OBJECTS_DIR += ./moc
RCC_DIR += ./moc
