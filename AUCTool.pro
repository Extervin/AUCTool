QT       += core gui sql concurrent network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
SOURCES += \
    confirmation.cpp \
    fileoperations.cpp \
    main.cpp \
    objects_table.cpp \
    serverinterface.cpp \
    switchbutton.cpp

HEADERS += \
    confirmation.h \
    fileoperations.h \
    objects_table.h \
    serverinterface.h \
    switchbutton.h

FORMS += \
    confirmation.ui \
    serverinterface.ui

TRANSLATIONS += \
    AUCTool_bg_BG.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc
