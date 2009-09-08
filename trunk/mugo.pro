# -------------------------------------------------
# Project created by QtCreator 2009-05-16T20:10:53
# -------------------------------------------------
TARGET = mugo
TEMPLATE = app
unix:QT += phonon
mac:QT += phonon
QT += network
SOURCES += main.cpp \
    mainwindow.cpp \
    boardwidget.cpp \
    godata.cpp \
    gameinformationdialog.cpp \
    sgf.cpp \
    ugf.cpp \
    countterritorydialog.cpp \
    setupdialog.cpp \
    command.cpp \
    playwithcomputerdialog.cpp \
    exportasciidialog.cpp \
    playgame.cpp \
    gtp.cpp \
    printoptiondialog.cpp
HEADERS += mainwindow.h \
    boardwidget.h \
    godata.h \
    gameinformationdialog.h \
    appdef.h \
    sgf.h \
    ugf.h \
    countterritorydialog.h \
    setupdialog.h \
    command.h \
    playwithcomputerdialog.h \
    exportasciidialog.h \
    playgame.h \
    gtp.h \
    printoptiondialog.h
FORMS += mainwindow.ui \
    boardwidget.ui \
    gameinformationdialog.ui \
    countterritorydialog.ui \
    setupdialog.ui \
    playwithcomputerdialog.ui \
    exportasciidialog.ui \
    printoptiondialog.ui
RESOURCES += resources.qrc
win32:RC_FILE = mugo.rc
mac:RC_FILE = pics/mugo.icns
win32:LIBS += libwinmm
TRANSLATIONS += mugo.ja_JP.ts \
    mugo_mac.ja_JP.ts
