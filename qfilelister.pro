#-------------------------------------------------
#
# Project created by QtCreator 2015-09-11T23:13:28
#
#-------------------------------------------------

QT       += core gui widgets opengl

CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qfilelister
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mimetypes/mimetypes.cpp

HEADERS  += mainwindow.h \
    mimetypes/mimetypes.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    README.md
