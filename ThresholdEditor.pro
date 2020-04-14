QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    histogram/lab_tab.c \
    histogram/rgb2rgb_tab.c \
    histogram/yuv_tab.c \
    main.cpp \
    thresholdeditor.cpp

HEADERS += \
    thresholdeditor.h

FORMS += \
    thresholdeditor.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

IDE_DISPLAY_NAME = Threshold Editor
APP_VERSION = 1.0.0
APP_DISPLAY_VERSION = 1.0.0

PRODUCT_BUNDLE_ORGANIZATION = org.qt-project
PROJECT_USER_FILE_EXTENSION = .user

win32 {
    COPYRIGHT="大来智能科技有限公司"
    APPLICATION_NAME = "$${IDE_DISPLAY_NAME}"
    DEFINES += \
        RC_APPLICATION_NAME=\"$$replace(APPLICATION_NAME, " ", "\\x20")\" \
        RC_VERSION=$$replace(APP_VERSION, "\\.", ","),0 \
        RC_VERSION_STRING=\"$${APP_DISPLAY_VERSION}\" \
        RC_COPYRIGHT=\"$$replace(COPYRIGHT, " ", "\\x20")\"
    RC_FILE = app.rc
}

DISTFILES += app.rc

