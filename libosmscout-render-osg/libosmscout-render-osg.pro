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

add_resources.path = $$OUT_PWD/res
add_resources.files += res/*.ttf
INSTALLS += add_resources

QMAKE_CXXFLAGS += -std=c++0x
