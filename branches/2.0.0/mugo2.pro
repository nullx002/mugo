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
    gameinformationdialog.cpp
HEADERS += mainwindow.h \
    boardwidget.h \
    document.h \
    sgfdocument.h \
    godata.h \
    mugoapp.h \
    sgf.h \
    command.h \
    gameinformationdialog.h
FORMS += mainwindow.ui \
    gameinformationdialog.ui
RESOURCES += mugo.qrc
