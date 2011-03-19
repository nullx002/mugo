# -------------------------------------------------
# Project created by QtCreator 2010-06-04T18:33:47
# -------------------------------------------------
QT += network \
    phonon
TARGET = mugo2
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    boardwidget.cpp \
    document.cpp \
    sgfdocument.cpp \
    godata.cpp \
    sgf.cpp \
    command.cpp \
    gameinformationdialog.cpp \
    exportasciidialog.cpp \
    newdocumentdialog.cpp \
    libkombilo/search.cpp \
    libkombilo/abstractboard.cpp \
    libkombilo/sgfparser.cpp \
    setupdialog.cpp \
    countterritorydialog.cpp \
    saveimagedialog.cpp \
    playwithcomputerdialog.cpp \
    enginelistdialog.cpp \
    enginelist.cpp \
    gameinterface.cpp \
    gtp.cpp
HEADERS += mainwindow.h \
    boardwidget.h \
    document.h \
    sgfdocument.h \
    godata.h \
    mugoapp.h \
    sgf.h \
    command.h \
    gameinformationdialog.h \
    exportasciidialog.h \
    newdocumentdialog.h \
    libkombilo/search.h \
    libkombilo/abstractboard.h \
    libkombilo/sgfparser.h \
    setupdialog.h \
    countterritorydialog.h \
    saveimagedialog.h \
    playwithcomputerdialog.h \
    enginelistdialog.h \
    enginelist.h \
    gameinterface.h \
    gtp.h
FORMS += mainwindow.ui \
    gameinformationdialog.ui \
    exportasciidialog.ui \
    newdocumentdialog.ui \
    setupdialog.ui \
    countterritorydialog.ui \
    saveimagedialog.ui \
    playwithcomputerdialog.ui \
    enginelistdialog.ui
RESOURCES += mugo.qrc
win32:RC_FILE = mugo.rc
mac:RC_FILE = pics/mugo.icns
TRANSLATIONS += mugo.ja_JP.ts \
    mugo_mac.ja_JP.ts
INCLUDEPATH += .\libkombilo
win32:INCLUDEPATH += c:\libs \
    .\sqlite3
win32:HEADERS += sqlite3/sqlite3.h
win32:SOURCES += sqlite3/sqlite3.c
unix:LIBS += -lsqlite3
