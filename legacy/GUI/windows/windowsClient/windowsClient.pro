#-------------------------------------------------
#
# Project created by QtCreator 2014-10-30T18:12:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = windowsClient
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    matchstatus.cpp

HEADERS  += mainwindow.h \
    structServClient.hpp \
    matchstatus.h

FORMS    += mainwindow.ui

win32: LIBS += -lXinput
win32:LIBS += -lWS2_32
