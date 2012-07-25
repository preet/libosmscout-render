TEMPLATE = lib
TARGET = osmscoutrenderosg
CONFIG += debug staticlib

HEADERS += MapRendererOSG.h

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

    SOURCES += MapRendererOSGLegacy.cpp
    HEADERS += MapRendererOSGLegacy.h
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

#    SOURCES += MapRendererOSGModern.cpp
#    HEADERS += MapRendererOSGModern.h

    SOURCES += MapRendererOSGExperimental.cpp
    HEADERS += MapRendererOSGExperimental.h
}

#jansson
LIBS += -ljansson

osgshaders.path = $$OUT_PWD/../res
osgshaders.files += shaders
INSTALLS += osgshaders

QMAKE_CXXFLAGS += -std=c++0x
