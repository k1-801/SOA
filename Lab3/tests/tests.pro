QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_json_test.cpp \
    ../qt_server/Input.cpp \
    ../qt_server/Output.cpp \
    ../qt_server/Server.cpp \
    ../qt_server/Connection.cpp
