QT += testlib xml
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_json_test.cpp \
    ../Lab/Input.cpp \
    ../Lab/Output.cpp
