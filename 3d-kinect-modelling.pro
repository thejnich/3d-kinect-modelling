
QT += opengl
TEMPLATE = app
TARGET = 
DEPENDPATH += . headers source
INCLUDEPATH += . headers /usr/include/libfreenect
LIBS += `pkg-config opencv --libs` -lfreenect
CONFIG += silent
QMAKE_CXXFLAGS = -ggdb

OBJECTS_DIR = build
MOC_DIR = build

# Input
HEADERS += headers/Mutex.h \
           headers/MyFreenectDevice.h \
           headers/RenderView.h \
           headers/Window.h \
		   headers/ObjWriter.h \
    headers/ObjectDetector.h \
	 headers/Primitives.h
SOURCES += source/main.cpp \
           source/RenderView.cpp \
           source/Window.cpp \
		   source/ObjWriter.cpp \
    source/ObjectDetector.cpp



