TEMPLATE = app
TARGET = osmscoutrender-osg
CONFIG += debug link_pkgconfig
PKGCONFIG += openthreads openscenegraph
INCLUDEPATH += ../libosmscout-render
LIBS += -L../libosmscout-render -losmscoutrender
LIBS += -ljansson -losmscout
SOURCES += \
        MapRendererOSG.cpp \
        test.cpp
HEADERS += \
        MapRendererOSG.h
