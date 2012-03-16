TEMPLATE = lib
TARGET = osmscoutrender
CONFIG += debug staticlib
LIBS += -ljansson -losmscout
SOURCES += \
        RenderStyleConfigReader.cpp \
        MapRenderer.cpp
HEADERS += \
        RenderStyleConfigReader.h \
        RenderStyleConfig.hpp \
        Vec3.hpp \
        SimpleLogger.hpp \
        MapRenderer.h
