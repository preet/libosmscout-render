TEMPLATE = lib
SUBDIRS = libosmscout-render
TARGET = osmscoutrender
CONFIG += debug
LIBS += -ljansson -losmscout
SOURCES += \
	MapRenderer.cpp \
	RendererDatabase.cpp \
        RendererStyleConfigReader.cpp
HEADERS += \
	MapRenderer.h \
	RendererDatabase.h \
        RendererStyleConfig.hpp \
        RendererStyleConfigReader.h \
