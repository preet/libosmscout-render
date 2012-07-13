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

QMAKE_CXXFLAGS += -std=c++0x


# okay for some reason -std=c++0x breaks
# GraphicsWindowEmbedded or at least it
# seems that way

# actually what would make sense is that
# you're trying to compile stuff with
# multiple definitions -- osg isn't compiled
# with -std=c++0x (i think?)

# this doesnt explain why it worked before
# but you might try discarding c++0x and
# using boost from the get-go

# dunno what to do about libosmscout though
# apparently we can try recompiling it without
# stdc++0x...
# http://libosmscout.git.sourceforge.net/git/gitweb.cgi?p=libosmscout/libosmscout;a=commit;h=d9b371fafe2355ea5367860b4e28e753c7c15870

# so try boost
