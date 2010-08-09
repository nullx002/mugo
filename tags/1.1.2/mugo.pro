# -------------------------------------------------
# Project created by QtCreator 2009-05-16T20:10:53
# -------------------------------------------------
TARGET = mugo
mac:CONFIG += x86 \
    ppc
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
    printoptiondialog.cpp \
    gib.cpp \
    ngf.cpp \
    boardsizedialog.cpp \
    saveimagedialog.cpp \
    qtsingleapplication.cpp \
    qtlockedfile_win.cpp \
    qtlockedfile_unix.cpp \
    qtlockedfile.cpp \
    qtlocalpeer.cpp \
    enginelistdialog.cpp \
    enginelist.cpp
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
    printoptiondialog.h \
    gib.h \
    ngf.h \
    boardsizedialog.h \
    saveimagedialog.h \
    qtsingleapplication.h \
    qtlockedfile.h \
    qtlocalpeer.h \
    mugoapp.h \
    enginelistdialog.h \
    enginelist.h
win32:SOURCES += qtdotnetstyle.cpp
win32:HEADERS += qtdotnetstyle.h
FORMS += mainwindow.ui \
    boardwidget.ui \
    gameinformationdialog.ui \
    countterritorydialog.ui \
    setupdialog.ui \
    playwithcomputerdialog.ui \
    exportasciidialog.ui \
    printoptiondialog.ui \
    boardsizedialog.ui \
    saveimagedialog.ui \
    enginelistdialog.ui
RESOURCES += resources.qrc
win32:RC_FILE = mugo.rc
mac:RC_FILE = pics/mugo.icns
win32:LIBS += libwinmm
TRANSLATIONS += mugo.ja_JP.ts \
    mugo_mac.ja_JP.ts
