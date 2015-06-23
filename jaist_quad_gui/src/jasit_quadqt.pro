#-------------------------------------------------
#
# Project created by QtCreator 2015-05-12T13:14:41
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = jasit_quadqt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ros_thread.cpp \
    ros_launch.cpp \
    qcustomplot.cpp \
    Dstar.cpp

HEADERS  += mainwindow.h \
    ros_thread.h \
    ros_launch.h \
    qcustomplot.h \
    Dstar.h \
    stack.h

FORMS    += mainwindow.ui
