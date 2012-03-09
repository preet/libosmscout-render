TEMPLATE = app
SUBDIRS = libosmscout-render
TARGET = osmscoutrender
CONFIG += debug
LIBS += -ljansson -losmscout
SOURCES += \
        RenderStyleConfigReader.cpp \
	MapRenderer.cpp \
        test.cpp
HEADERS += \
        RenderStyleConfigReader.h \
        RenderStyleConfig.hpp \
        Vec3.hpp \
        SimpleLogger.hpp \
        MapRenderer.h
