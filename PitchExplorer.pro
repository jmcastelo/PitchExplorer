QT += core gui multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

# Uncomment to compile for Windows 64bit platform
#CONFIG += win64

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TARGET = PitchExplorer

INCLUDEPATH += extra/

SOURCES += \
    src/fourier.cpp \
    src/hurst.cpp \
    src/kmeans.cpp \
    src/main.cpp \
    src/mainWindow.cpp \
    src/pca.cpp \
    extra/fftw3.h \
    extra/flowLayout.cpp \
    extra/qcustomplot.cpp

HEADERS += \
    src/fourier.h \
    src/hurst.h \
    src/kmeans.h \
    src/mainWindow.h \
    src/pca.h \
    extra/dr_flac.h \
    extra/dr_mp3.h \
    extra/dr_wav.h \
    extra/fftw3.h \
    extra/flowLayout.h \
    extra/miniaudio.h \
    extra/qcustomplot.h

win64 {
    LIBS += "$$PWD/extra/libfftw3-3.dll" -lm
} else {
    LIBS += -lfftw3 -lm -ldl -lpthread
}

QMAKE_CXXFLAGS_RELEASE += -O3

RESOURCES = PitchExplorer.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
