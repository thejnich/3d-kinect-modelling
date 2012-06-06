
QT += opengl
TEMPLATE = app
TARGET = 
DEPENDPATH += . headers source
INCLUDEPATH += . headers /usr/local/include/libfreenect
LIBS += `pkg-config opencv --libs` -lfreenect -lCGAL -lCGAL_CORE
CONFIG += silent
QMAKE_CXXFLAGS += -ggdb -frounding-math

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



