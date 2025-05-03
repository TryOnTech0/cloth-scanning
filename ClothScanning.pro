TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        ClothScanning.cpp \
        main.cpp

HEADERS += \
    ClothScanning.h
    INCLUDEPATH += /usr/include/opencv4
    LIBS += -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio

