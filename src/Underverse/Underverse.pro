# -------------------------------------------------
# Project created by QtCreator 2010-03-29T13:28:53
# -------------------------------------------------

QT += core widgets gui webkitwidgets

TARGET = Underverse
TEMPLATE = app
RC_FILE	 = icon.rc

SOURCES += main.cpp \
    MainWindow.cpp \
    SettingsDialog.cpp
    
HEADERS += \ 
    MainWindow.h \
    SettingsDialog.h
    
FORMS += \ 
    MainWindow.ui \
    SettingsDialog.ui

#include cppCORE library
INCLUDEPATH += $$PWD/../../src/cppCORE
LIBS += -L$$PWD/../../bin -lcppCORE

#include cppGUI library
INCLUDEPATH += $$PWD/../../src/cppGUI
LIBS += -L$$PWD/../../bin -lcppGUI

#include sundown library
INCLUDEPATH += $$PWD/../../src/sundown/src/ $$PWD/../../src/sundown/html/
LIBS += -L$$PWD/../../bin -lsundown

#copy EXE to bin folder
DESTDIR = $$PWD/../../bin

RESOURCES += \
    resources.qrc

DISTFILES += \
    ToDos.txt
