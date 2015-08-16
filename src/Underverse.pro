TEMPLATE = subdirs
CONFIG += console

#Library targets and depdendencies
SUBDIRS = cppCORE\
        cppGUI\
        sundown\
        Underverse

cppGUI.depends = cppCORE
Underverse.depends = cppCORE cppGUI sundown
