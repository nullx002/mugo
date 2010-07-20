# -------------------------------------------------
# Project created by QtCreator 2010-06-04T18:33:47
# -------------------------------------------------
QT += network
unix:QT += phonon
mac:QT += phonon
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
    sqlite3/sqlite3.c \
    libkombilo/abstractboard.cpp \
    libkombilo/sgfparser.cpp
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
    sqlite3/sqlite3.h \
    libkombilo/abstractboard.h \
    libkombilo/sgfparser.h
FORMS += mainwindow.ui \
    gameinformationdialog.ui \
    exportasciidialog.ui \
    newdocumentdialog.ui
RESOURCES += mugo.qrc
win32:RC_FILE = mugo.rc
mac:RC_FILE = pics/mugo.icns
TRANSLATIONS += mugo.ja_JP.ts \
    mugo_mac.ja_JP.ts
win32:INCLUDEPATH += c:\libs\
    .\libkombilo\
    .\sqlite3
