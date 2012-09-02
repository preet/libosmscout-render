QT       += core gui opengl
CONFIG   += debug

#openscenegraph
#CONFIG += link_pkgconfig
#PKGCONFIG += openthreads openscenegraph

TARGET = mapviewer
TEMPLATE = app

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

#libosmscout-render-osg
INCLUDEPATH += ../libosmscout-render-osg
LIBS += -L../libosmscout-render-osg -losmscoutrenderosg

#libosmscout-render
INCLUDEPATH += ../libosmscout-render
LIBS += -L../libosmscout-render -losmscoutrender

#libosmscout
INCLUDEPATH += /home/preet/Dev/env/sys/libosmscout/include
LIBS += -L/home/preet/Dev/env/sys/libosmscout/lib -losmscout

#openscenegraph
gl_legacy  {
    # using fixed-function deprecated opengl
    DEFINES += GL_LEGACY
    OSGDIR = /home/preet/Dev/env/sys/osg-legacy
    OSGLIBDIR = /home/preet/Dev/env/sys/osg-legacy/lib64
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
}

gl_modern {
    # use modern shader based opengl and try
    # to maintain compatibility with OpenGL ES 2
    DEFINES += GL_MODERN
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
}

#jansson
LIBS += -ljansson

#QMAKE_CXXFLAGS += -std=c++0x
