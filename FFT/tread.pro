QT += core gui
QT += printsupport


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    datagenerator.cpp \
    dataplotter.cpp \
    fftprocess.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp

HEADERS += \
    datagenerator.h \
    dataplotter.h \
    fftprocess.h \
    fftw/fftw3.h \
    mainwindow.h \
    qcustomplot.h

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
