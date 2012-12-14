# set these paths
LIBOSMSCOUT_PATH = /home/preet/Dev/env/sys/libosmscout
LIBOSMSCOUTRENDER_PATH = /home/preet/Dev/projects/libosmscout-render


QT       += core gui opengl
CONFIG   += debug link_pkgconfig
PKGCONFIG += openthreads openscenegraph
TARGET = mapviewer
TEMPLATE = app

INCLUDEPATH += $${LIBOSMSCOUTRENDER_PATH}
SOURCES += main.cpp\
           mapviewer.cpp \
           viewport.cpp

HEADERS += mapviewer.h \
           viewport.h

#boost // off by default!
#USE_BOOST   {
#   DEFINES += USE_BOOST
#   INCLUDEPATH += /home/preet/Dev/env/sys/boost-1.50
#}

QMAKE_CXXFLAGS += -std=c++11

#liblzma
HEADERS +=  \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/Alloc.h \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/LzFind.h \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/LzHash.h \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/LzmaEnc.h \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/LzmaLib.h \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/NameMangle.h \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/Types.h

SOURCES +=  \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/Alloc.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/LzFind.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/LzmaDec.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/LzmaEnc.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/liblzma/LzmaLib.c

# openctm
HEADERS += \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/openctmpp.h \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/openctm.h \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/internal.h

SOURCES += \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/stream.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/openctm.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/compressRAW.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/compressMG2.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/openctm/compressMG1.c

# jansson
HEADERS += \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/hashtable.h \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/jansson_config.h \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/jansson_private.h \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/jansson.h

SOURCES += \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/dump.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/error.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/hashtable.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/load.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/memory.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/pack_unpack.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/strbuffer.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/strconv.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/utf.c \
   $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/jansson/value.c

# clipper
HEADERS += $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/clipper/clipper.hpp
SOURCES += $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/clipper/clipper.cpp

#libosmscout
INCLUDEPATH += $${LIBOSMSCOUT_PATH}/include
LIBS += -L$${LIBOSMSCOUT_PATH}/lib -losmscout

#libosmscout-render
HEADERS += \
    $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/RenderStyleReader.h \
    $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/RenderStyleConfig.hpp \
    $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/SimpleLogger.hpp \
    $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/Vec2.hpp \
    $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/Vec3.hpp \
    $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/DataSet.hpp \
    $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/MapRenderer.h

SOURCES += \
    $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/RenderStyleReader.cpp \
    $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render/MapRenderer.cpp

#libosmscout-render-osg
HEADERS += $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render-osg/MapRendererOSG.h
SOURCES += $${LIBOSMSCOUTRENDER_PATH}/libosmscout-render-osg/MapRendererOSG.cpp

# install fonts
ifonts.path = $$OUT_PWD/fonts
ifonts.files += ../res/fonts/*

# install styles
istyles.path = $$OUT_PWD/styles
istyles.files += ../res/styles/*

# install shaders
ishaders.path = $$OUT_PWD/shaders
ishaders.files += ../libosmscout-render-osg/shaders/*

# install mesh
imesh.path = $$OUT_PWD/mesh
imesh.files += ../res/mesh/*

INSTALLS += ifonts istyles ishaders imesh
