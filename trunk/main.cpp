#include <QtGui/QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTextCodec>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    bool aa = myappTranslator.load("mugo." + QLocale::system().name());
    a.installTranslator(&myappTranslator);

    QTextCodec::setCodecForCStrings( QTextCodec::codecForName("UTF-8") );
    QTextCodec::setCodecForTr( QTextCodec::codecForName("UTF-8") );

    MainWindow w;
    w.show();
    return a.exec();
}
