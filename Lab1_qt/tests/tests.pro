QT += testlib xml
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  \
    ../Lab/Input.cpp \
    ../Lab/Output.cpp \
    ../Lab/Serializer.cpp \
    tst_serializer_test.cpp

HEADERS += \
        ../Lab/Input.hpp \
        ../Lab/Output.hpp \
        ../Lab/Serializer.hpp
