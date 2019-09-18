QT -= gui
QT += xml

CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += \
        Input.cpp \
        Output.cpp \
        Serializer.cpp \
        main.cpp

HEADERS += \
        Input.hpp \
        Output.hpp \
        Serializer.hpp
