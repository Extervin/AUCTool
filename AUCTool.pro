QT       += core gui widgets concurrent\

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    lib/serveraucoperations.cpp \
    lib/serverupdate.cpp \
    lib\auctooloperations.cpp \
    lib\settingsdialog.cpp \
    lib\main.cpp \
    lib\mainwindow.cpp

HEADERS += \
    lib/serveraucoperations.h \
    lib/serverupdate.h \
    lib\auctooloperations.h \
    lib\settingsdialog.h \
    lib\mainwindow.h

FORMS += \
    lib/serverupdate.ui \
    lib\settingsdialog.ui \
    lib\mainwindow.ui

TRANSLATIONS += \
    AUCTool_bg_BG.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += "C:\0-project\project-3\zlib\include"
LIBS += -L"C:\0-project\project-3\zlib\lib"
LIBS += -lz

INCLUDEPATH += "C:\0-project\project-3\quazip\include"
LIBS += -L"C:\0-project\project-3\quazip\lib" -lquazip
