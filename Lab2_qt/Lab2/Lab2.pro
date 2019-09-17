QT += xml
QT -= gui
CONFIG += console c++11
CONFIG -= app_bundle

#DEFINES += DEBUG

INCLUDEPATH += ../../Lab1_qt/Lab

SOURCES += \
        main.cpp \
        ../../Lab1_qt/Lab/Input.cpp \
        ../../Lab1_qt/Lab/Output.cpp

HEADERS += \
        ../../Lab1_qt/Lab/Input.hpp \
        ../../Lab1_qt/Lab/Output.hpp
