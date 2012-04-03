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
        MapRendererOSG.h \

# poly2tri
HEADERS +=  poly2tri/poly2tri.h \
            poly2tri/common/shapes.h \
            poly2tri/common/utils.h \
            poly2tri/sweep/advancing_front.h \
            poly2tri/sweep/cdt.h \
            poly2tri/sweep/sweep.h \
            poly2tri/sweep/sweep_context.h

SOURCES +=  poly2tri/common/shapes.cc \
            poly2tri/sweep/advancing_front.cc \
            poly2tri/sweep/cdt.cc \
            poly2tri/sweep/sweep.cc \
            poly2tri/sweep/sweep_context.cc

add_resources.path = $$OUT_PWD/res
add_resources.files += res/*.ttf
INSTALLS += add_resources

QMAKE_CXXFLAGS += -std=c++0x
