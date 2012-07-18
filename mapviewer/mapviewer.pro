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

#libosmscout-render-osg
INCLUDEPATH += ../libosmscout-render-osg
LIBS += -L../libosmscout-render-osg -losmscoutrenderosg

#libosmscout-render
INCLUDEPATH += ../libosmscout-render
LIBS += -L../libosmscout-render -losmscoutrender

#libosmscout
INCLUDEPATH += /home/preet/Documents/libosmscout/include
LIBS += -L/home/preet/Documents/libosmscout/lib -losmscout

#openscenegraph
gl_legacy  {
    # using fixed-function deprecated opengl
    DEFINES += GL_LEGACY
    OSGDIR = /home/preet/Documents/osg-legacy-cpp11
    OSGLIBDIR = /home/preet/Documents/osg-legacy-cpp11/lib64
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
    OSGDIR = /home/preet/Documents/osg-modern-cpp11
    OSGLIBDIR = /home/preet/Documents/osg-modern-cpp11/lib64
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

QMAKE_CXXFLAGS += -std=c++0x
