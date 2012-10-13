QT       += core gui opengl
CONFIG   += debug
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
