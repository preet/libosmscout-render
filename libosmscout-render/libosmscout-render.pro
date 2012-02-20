TEMPLATE = lib
SUBDIRS = libosmscout-render
TARGET = osmscoutrender
CONFIG += debug
LIBS += -ljansson -losmscout
SOURCES += \
	MapRenderer.cpp \
	RendererDatabase.cpp \
	RendererStyleConfig.cpp \
        RendererStyleConfigReader.cpp
HEADERS += \
	MapRenderer.h \
	RendererDatabase.h \
	RendererStyleConfig.h \
        RendererStyleConfigReader.h
