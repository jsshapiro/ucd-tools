#-------------------------------------------------
#
# Project created by QtCreator 2016-10-22T13:26:10
#
#-------------------------------------------------

QT       -= core gui

TARGET = libucd
TEMPLATE = lib
CONFIG += staticlib

SOURCES += CodePointSet.cpp \
    utf8.cpp

HEADERS += CodePointSet.h \
    CodePoint.h \
    CodePointRange.h \
    utf8.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    README.md
