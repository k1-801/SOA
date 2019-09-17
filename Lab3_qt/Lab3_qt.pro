QT += xml httpserver
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

INCLUDEPATH += ../Lab1_qt/Lab/

SOURCES += \
        ../Lab1_qt/Lab/Input.cpp \
        ../Lab1_qt/Lab/Output.cpp \
        main.cpp

HEADERS += \
        ../Lab1_qt/Lab/Input.hpp \
        ../Lab1_qt/Lab/Output.hpp
