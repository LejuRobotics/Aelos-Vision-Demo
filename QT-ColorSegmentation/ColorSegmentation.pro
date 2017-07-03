#-------------------------------------------------
#
# Project created by QtCreator 2017-05-26T18:44:40
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ColorSegmentation
TEMPLATE = app


SOURCES += main.cpp\
    VideoArea.cpp \
    painterlabel.cpp\
    portdialog.cpp


HEADERS += VideoArea.h\
    painterlabel.h\
    portdialog.h

FORMS    +=  VideoArea.ui\
    portdialog.ui
