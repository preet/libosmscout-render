QT       += core gui opengl
CONFIG   += debug
TARGET = mapviewer
TEMPLATE = app

LIBOSMSCOUTRENDER_PATH = /home/preet/Dev/projects/libosmscout-render
INCLUDEPATH += $${LIBOSMSCOUTRENDER_PATH}

SOURCES += main.cpp\
           mapviewer.cpp \
           viewport.cpp

HEADERS += mapviewer.h \
           viewport.h

#boost
USE_BOOST   {
   DEFINES += USE_BOOST
   INCLUDEPATH += /home/preet/Dev/env/sys/boost-1.50
}

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

#openscenegraph
OSGDIR = /home/preet/Dev/env/sys/osg-modern
OSGLIBDIR = /home/preet/Dev/env/sys/osg-modern/lib64
INCLUDEPATH += $${OSGDIR}/include
LIBS += -L$${OSGLIBDIR}/osgdb_freetyperd.so
LIBS += -L$${OSGLIBDIR}/osgdb_jpegrd.so
LIBS += -L$${OSGLIBDIR}/osgdb_pngrd.so
LIBS += -L$${OSGLIBDIR} -losgViewerrd
LIBS += -L$${OSGLIBDIR} -losgTextrd
LIBS += -L$${OSGLIBDIR} -losgGArd
LIBS += -L$${OSGLIBDIR} -losgUtilrd
LIBS += -L$${OSGLIBDIR} -losgDBrd
LIBS += -L$${OSGLIBDIR} -losgrd
LIBS += -L$${OSGLIBDIR} -lOpenThreadsrd

#libosmscout
INCLUDEPATH += /home/preet/Dev/env/sys/libosmscout/include
LIBS += -L/home/preet/Dev/env/sys/libosmscout/lib -losmscout

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

# install coastlines0
icoastlines0.path = $$OUT_PWD/coastlines0
icoastlines0.files += ../res/coastlines0/*

INSTALLS += ifonts istyles ishaders icoastlines0
