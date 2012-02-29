CONFIG      += qt debug link_pkgconfig
PKGCONFIG   += openthreads openscenegraph
QT          += core gui opengl declarative script
TEMPLATE    = app
TARGET      = osgqdec

HEADERS += \
    qosgdeclarativeviewport.h \
    qosgdeclarativeviewport.h
			
SOURCES += \
    main.cpp \
    qosgdeclarativeviewport.cpp
			
OTHER_FILES += \
    main.qml
    
moreFiles.path = $$OUT_PWD
moreFiles.files += *.osg
moreFiles.files += *.qml
INSTALLS += moreFiles
