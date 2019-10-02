#-------------------------------------------------
#
# Project created by QtCreator 2015-09-23T11:57:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SudokuSolver
TEMPLATE = app


SOURCES += main.cpp\
        detectgrid.cpp \
        numberrecognition.cpp \
        mainwindow.cpp

HEADERS  += mainwindow.h \
    detectgrid.h \
    numberrecognition.h

FORMS    += mainwindow.ui

INCLUDEPATH += "d:/opencv-4.1.1/build/install/include"

LIBS += "d:/opencv-4.1.1/build/install/x64/mingw/bin/libopencv*.dll"
