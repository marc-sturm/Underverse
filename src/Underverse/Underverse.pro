#c++11 support
CONFIG += c++11

QT += core widgets gui network

TARGET = Underverse
TEMPLATE = app
RC_FILE	 = icon.rc

SOURCES += main.cpp \
    GitWorker.cpp \
    MainWindow.cpp \
    SettingsDialog.cpp \
    MarkDownHighlighter.cpp \
    Editor.cpp \
    NotesBrowser.cpp \
    SearchBox.cpp
    
HEADERS += \
    GitWorker.h \
    MainWindow.h \
    SettingsDialog.h \
    MarkDownHighlighter.h \
    Editor.h \
    NotesBrowser.h \
    SearchBox.h
    
FORMS += \ 
    MainWindow.ui \
    SettingsDialog.ui \
    SearchBox.ui

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
    ../../todo.txt \
    Resources/git_pull_highlight.png \
    Resources/git_push_highlight.png
