# -------------------------------------------------
# Project created by QtCreator 2009-07-20T15:57:40
# -------------------------------------------------
TARGET = ps3muxer
TEMPLATE = app
VPATH += ./ebml
INCLUDEPATH += ./ebml
SOURCES += main.cpp \
    mainwindow.cpp \
    ebml.cpp \
    execwindow.cpp
HEADERS += mainwindow.h \
    ebml.h \
    execwindow.h
FORMS += mainwindow.ui \
    execwindow.ui
TRANSLATIONS += ps3muxer_ru.ts
system(lupdate ps3muxer.pro)
system(lrelease ps3muxer.pro)
RESOURCES += ps3muxer.qrc

win32-msvc {
    RC_FILE = ps3muxer.rc
    DEFINES += _CRT_SECURE_NO_WARNINGS
    LIBS += Ws2_32.lib
}

win32-g++ {
    RC_FILE = ps3muxer.rc
    LIBS += libws2_32
}

linux-g++ {
}

macx-g++ {
}
