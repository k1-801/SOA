TEMPLATE = subdirs
CONFIG += console c++17 testlib
CONFIG -= app_bundle
CONFIG -= qt
SUBDIRS += tests \
    qt_server

SOURCES += \
        main.cpp


HEADERS += \
        Input.hpp \
        Output.hpp \
        Connection.hpp \
        Server.hpp

DEFINES += DEBUG
