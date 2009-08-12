#include <QtGui/QApplication>
#include <QDebug>
#include <QSettings>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QDir>
#include "appdef.h"
#include "mainwindow.h"

void setDefaultSettings(){
#define setvalue(K, V) if (!settings.contains(K)) settings.setValue(K, V);

    QSettings settings(AUTHOR, APPNAME);

    setvalue("boardType", 0);
    setvalue("boardColor", QColor(255, 200, 100));
    setvalue("whiteType", 0);
    setvalue("whiteColor", QColor(255, 255, 255));
    setvalue("blackType", 0);
    setvalue("blackColor", QColor(0, 0, 0));

#undef setvalue
}

QString getTranslationPath(){
    QString appPath = qApp->applicationDirPath();
    QStringList pathList;

#ifdef Q_WS_WIN
    pathList << appPath + "/translations";
#elif defined(Q_WS_X11)
    pathList << appPath + "/translations"
             << "/usr/share/" APPNAME "/translations"
             << "/usr/local/share/" APPNAME "/translations";
#endif

    QStringList::iterator iter = pathList.begin();
    while (iter != pathList.end()){
        QDir dir(*iter);
        if (dir.exists())
            return *iter;
        ++iter;
    }

    return "./";
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(APPNAME);

    // set default settings
    setDefaultSettings();

    // Load translation
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QSettings settings(AUTHOR, APPNAME);
    QString locale = settings.value("language").toString();
    if (locale.isEmpty())
        locale = QLocale::system().name();
    QTranslator myappTranslator;
    myappTranslator.load("mugo." + locale, getTranslationPath());
    a.installTranslator(&myappTranslator);

    QTextCodec::setCodecForCStrings( QTextCodec::codecForName("UTF-8") );
    QTextCodec::setCodecForTr( QTextCodec::codecForName("UTF-8") );

    MainWindow w;
    w.show();
    return a.exec();
}
