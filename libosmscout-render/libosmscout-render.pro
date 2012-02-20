TEMPLATE = lib
SUBDIRS = libosmscout-render
TARGET = osmscoutrender
CONFIG += debug
LIBS += -ljansson -losmscout
SOURCES += \
	MapRenderer.cpp \
        RenderStyleConfigReader.cpp
HEADERS += \
	MapRenderer.h \
        RenderStyleConfig.hpp \
        RenderStyleConfigReader.h \
