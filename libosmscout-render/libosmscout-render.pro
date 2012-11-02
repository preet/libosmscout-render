TEMPLATE = lib
TARGET = osmscoutrender
CONFIG += debug staticlib

#boost
USE_BOOST   {
   DEFINES += USE_BOOST
   INCLUDEPATH += /home/preet/Dev/env/sys/boost-1.50
}

#liblzma
HEADERS +=  openctm/liblzma/Alloc.h \
            openctm/liblzma/LzFind.h \
            openctm/liblzma/LzHash.h \
            openctm/liblzma/LzmaEnc.h \
            openctm/liblzma/LzmaLib.h \
            openctm/liblzma/NameMangle.h \
            openctm/liblzma/Types.h

SOURCES +=  openctm/liblzma/Alloc.c \
            openctm/liblzma/LzFind.c \
            openctm/liblzma/LzmaDec.c \
            openctm/liblzma/LzmaEnc.c \
            openctm/liblzma/LzmaLib.c

# openctm
HEADERS += openctm/openctmpp.h \
           openctm/openctm.h \
           openctm/internal.h

SOURCES += openctm/stream.c \
           openctm/openctm.c \
           openctm/compressRAW.c \
           openctm/compressMG2.c \
           openctm/compressMG1.c

# jansson
HEADERS += \
   jansson/hashtable.h \
   jansson/jansson_config.h \
   jansson/jansson_private.h \
   jansson/jansson.h

SOURCES += \
   jansson/dump.c \
   jansson/error.c \
   jansson/hashtable.c \
   jansson/load.c \
   jansson/memory.c \
   jansson/pack_unpack.c \
   jansson/strbuffer.c \
   jansson/strconv.c \
   jansson/utf.c \
   jansson/value.c

# clipper
HEADERS += clipper/clipper.hpp
SOURCES += clipper/clipper.cpp

#libosmscout
INCLUDEPATH += /home/preet/Dev/env/sys/libosmscout/include
LIBS += -L/home/preet/Dev/env/sys/libosmscout/lib -losmscout

SOURCES += \
        RenderStyleReader.cpp \
        MapRenderer.cpp
HEADERS += \
        RenderStyleReader.h \
        RenderStyleConfig.hpp \
        Vec2.hpp \
        Vec3.hpp \
        SimpleLogger.hpp \
        DataSet.hpp \
        MapRenderer.h
