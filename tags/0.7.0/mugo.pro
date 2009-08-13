# -------------------------------------------------
# Project created by QtCreator 2009-05-16T20:10:53
# -------------------------------------------------
TARGET = mugo
TEMPLATE = app
QT += phonon
SOURCES += main.cpp \
    mainwindow.cpp \
    boardwidget.cpp \
    godata.cpp \
    gameinformationdialog.cpp \
    sgf.cpp \
    ugf.cpp \
    countterritorydialog.cpp \
    setupdialog.cpp
HEADERS += mainwindow.h \
    boardwidget.h \
    godata.h \
    gameinformationdialog.h \
    appdef.h \
    sgf.h \
    ugf.h \
    countterritorydialog.h \
    setupdialog.h
FORMS += mainwindow.ui \
    boardwidget.ui \
    gameinformationdialog.ui \
    countterritorydialog.ui \
    setupdialog.ui
RESOURCES += resources.qrc
RC_FILE = mugo.rc
TRANSLATIONS = mugo.ja_JP.ts
