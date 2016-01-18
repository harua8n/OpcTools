QT += core axcontainer
QT -= gui

TARGET = opcdadevicereadsyncstress
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    OPCDAAuto_i.c

HEADERS += \
    OPCDaAuto.h

