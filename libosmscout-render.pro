TEMPLATE =  subdirs

SUBDIRS +=  libosmscout-render \
            libosmscout-render-osg \
            mapviewer
			
CONFIG += ordered

add_resources.path = $$OUT_PWD
add_resources.files += res
INSTALLS += add_resources
