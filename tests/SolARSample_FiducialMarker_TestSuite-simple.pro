QT += testlib         #Include QtTest to use SignalSpy, QTest::mouseClick, etc

TARGET = Tests
TEMPLATE = app

CONFIG += c++17
CONFIG += testcase    #Creates 'check' target in Makefile.
CONFIG -= debug_and_release
CONFIG += cmdline

INCLUDEPATH += C:\Users\jmhenaff\.conan\data\gtest\1.10.0\_\_\package\0156ebe61e4171e9ea7ab9a72d018e2e1f6f6627\include

LIBS += -LC:\Users\jmhenaff\.conan\data\gtest\1.10.0\_\_\package\0156ebe61e4171e9ea7ab9a72d018e2e1f6f6627\lib -lgtestd

HEADERS +=

SOURCES += main.cpp \
           test.cpp
