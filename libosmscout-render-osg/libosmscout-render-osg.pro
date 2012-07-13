TEMPLATE = lib
TARGET = osmscoutrenderosg
CONFIG += debug staticlib

#openscenegraph
#CONFIG += link_pkgconfig
#PKGCONFIG += openthreads openscenegraph

#libosmscout-render
INCLUDEPATH += ../libosmscout-render
LIBS += -L../libosmscout-render -losmscoutrender

#libosmscout
LIBS += -losmscout

#openscenegraph
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

#jansson
LIBS += -ljansson

SOURCES += \
        MapRendererOSG.cpp
HEADERS += \
        MapRendererOSG.h

add_resources.path = $$OUT_PWD/res
add_resources.files += res/*.ttf
INSTALLS += add_resources

QMAKE_CXXFLAGS += -std=c++0x
