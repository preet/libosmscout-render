TARGET = qglviewport
CONFIG += qt debug
LIBS += -ljansson -losmscout
LIBS += -L/home/preet/Dev/_libosmscout-render-build/libosmscout-render -losmscoutrender
INCLUDEPATH += /home/preet/Dev/libosmscout-render/libosmscout-render
SOURCES = MapRendererQt5.cpp qglosmviewport.cpp main.cpp
HEADERS = MapRendererQt5.h qglosmviewport.h
QT += 3d
