#-------------------------------------------------
#
# Project created by QtCreator 2017-06-06T20:49:41
#
#-------------------------------------------------

QT       += core network serialport
QT       += gui

TARGET = TCP_Server_Console
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    server.cpp \
    SerialPort.cpp \
    Segmenter.cpp \
    VideoControl.cpp

HEADERS += \
    server.h \
    SerialPort.h \
    Segmenter.h \
    VideoControl.h \
    global_var.h

win32 {
#如果在window下运行，推荐将工程目录下的opencv-win-lib\bin和添加到系统环境变量中，
#不然需要把这个目录下的dll文件复制到可执行程序目录下(不推荐)
INCLUDEPATH+=$$PWD\opencv3.2.0-win-lib\include
LIBS += $$PWD\opencv3.2.0-win-lib\lib\libopencv_videoio320.dll.a\
        $$PWD\opencv3.2.0-win-lib\lib\libopencv_core320.dll.a\
        $$PWD\opencv3.2.0-win-lib\lib\libopencv_highgui320.dll.a\
        $$PWD\opencv3.2.0-win-lib\lib\libopencv_imgproc320.dll.a\
        $$PWD\opencv3.2.0-win-lib\lib\libopencv_imgcodecs320.dll.a\
        $$PWD\opencv3.2.0-win-lib\lib\libopencv_video320.dll.a\
        $$PWD\opencv3.2.0-win-lib\lib\libopencv_objdetect320.dll.a
}

linux {
INCLUDEPATH += /usr/include/
LIBS += -L/usr/lib/arm-linux-gnueabihf \
          /usr/lib/arm-linux-gnueabihf/libopencv_core.so \
          /usr/lib/arm-linux-gnueabihf/libopencv_video.so \
          /usr/lib/arm-linux-gnueabihf/libopencv_highgui.so \
          /usr/lib/arm-linux-gnueabihf/libopencv_imgproc.so \
          /usr/lib/arm-linux-gnueabihf/libopencv_imgcodecs.so \
          /usr/lib/arm-linux-gnueabihf/libopencv_videoio.so
}
