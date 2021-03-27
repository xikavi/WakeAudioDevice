QT += gui widgets

TEMPLATE = app
CONFIG += c++17
RESOURCES = res.qrc
RC_ICONS = icon.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        audiodevice_win.cpp \
        audiodevicevolumecontrol_win.cpp \
        audiopeakmeter_win.cpp \
        audiorenderer_win.cpp \
        eventfilter_win.cpp \
        log.cpp \
        main.cpp \
        mainwindow.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    mainwindow.ui

HEADERS += \
    audiodevice.h \
    audiodevicevolumecontrol_win.h \
    audiopeakmeter_win.h \
    audiorenderer_win.h \
    def_win.h \
    eventfilter.h \
    log.h \
    mainwindow.h

INCLUDEPATH += include/

win32: LIBS += -L$$PWD/lib/x64 -lOle32 -lMfplat -lMfreadwrite

VERSION = 1.1.1.0
QMAKE_TARGET_COPYRIGHT = Xikavi
QMAKE_TARGET_COMPANY = Xikavi Soft
