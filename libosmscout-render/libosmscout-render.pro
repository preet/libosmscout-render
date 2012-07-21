TEMPLATE = lib
TARGET = osmscoutrender
CONFIG += debug staticlib

#boost
USE_BOOST   {
   DEFINES += USE_BOOST
   INCLUDEPATH += /home/preet/Documents/boost-1.50
}

#libosmscout
INCLUDEPATH += /home/preet/Documents/libosmscout/include
LIBS += -L/home/preet/Documents/libosmscout/lib -losmscout

SOURCES += \
        RenderStyleConfigReader.cpp \
        MapRenderer.cpp
HEADERS += \
        RenderStyleConfigReader.h \
        RenderStyleConfig.hpp \
        Vec2.hpp \
        Vec3.hpp \
        SimpleLogger.hpp \
        MapRenderer.h

QMAKE_CXXFLAGS += -std=c++0x

#jansson
LIBS += -ljansson
