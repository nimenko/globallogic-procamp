TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += src/main.cpp \
    src/server.cpp

HEADERS += \
    include/server.h

LIBS += -L/usr/lib/ -lboost_system -lpthread
