#-------------------------------------------------
#
# Project created by QtCreator 2024-03-18
#
#-------------------------------------------------

QT += core gui
QT += printsupport
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = IVA23_v2.0
TEMPLATE = app

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

RC_ICONS = HTLOGO.ico   # 图标

#RC_FILE += heu.rc


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    appdata.cpp \
    datagenerator.cpp \
    dataplotter.cpp \
    fftprocess.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    udpprocess.cpp \
    uieventhandler.cpp

HEADERS += \
    appdata.h \
    datagenerator.h \
    dataplotter.h \
    fftprocess.h \
    fftw/fftw3.h \
    mainwindow.h \
    qcustomplot.h \
    udpprocess.h \
    uieventhandler.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH +=.\fftw

LIBS += -L$$PWD\fftw\ -llibfftw3-3
LIBS += -L$$PWD\fftw\ -llibfftw3f-3
LIBS += -L$$PWD\fftw\ -llibfftw3l-3

DISTFILES += \
    heu.rc

RESOURCES += \
    picture.qrc
