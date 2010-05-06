#include <QDebug>
#include <QSettings>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QDir>
#include <QMessageBox>
#ifdef Q_WS_WIN
#  include "qtdotnetstyle.h"
#elif defined(Q_WS_MAC)
#   include <QFileOpenEvent>
#endif
#include "appdef.h"
#include "mugoapp.h"
#include "mainwindow.h"


QString getTranslationPath(){
    QString appPath = qApp->applicationDirPath();
    QStringList pathList;

#ifdef Q_WS_WIN
    pathList << appPath + "/translations";
#elif defined(Q_WS_X11)
    pathList << appPath + "/translations"
             << "/usr/share/" APPNAME "/translations"
             << "/usr/local/share/" APPNAME "/translations";
#elif defined(Q_WS_MAC)
    pathList << QFileInfo(appPath + "/../Resources/translations/").absolutePath()
             << "./translations";
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

bool installTranslator(QApplication& a, QTranslator& translator, const QString& path, QString file){
    if (translator.load(file, path) == false){
        file = file.toLower();
        if (translator.load(file, path) == false)
            return false;
    }
    a.installTranslator(&translator);
    return true;
}


Application::~Application(){
}

#if defined(Q_WS_MAC)
bool Application::event(QEvent* e){
    switch(e->type()){
        case QEvent::FileOpen:{
            if (mainWindow)
                mainWindow->fileOpen( static_cast<QFileOpenEvent*>(e)->file() );
            return true;
        }

        default:
            return QtSingleApplication::event(e);
    }
}
#endif

void Application::received(const QString& msg){
    QStringList files = msg.split("\n");
    foreach(const QString& f, files)
        if (f.isEmpty() == false)
            mainWindow->fileOpen(f);
}

void Application::setMainWindow(MainWindow* win){
    setActivationWindow(win);
    mainWindow = win;
}

int main(int argc, char *argv[])
{
    putenv( "UNICODEMAP_JP=cp932" );

// is QFileOpenEvent received on macx??
    Application a(argc, argv);

    QStringList args = a.arguments();
    QString param;
    for(int i=1; i<args.size(); ++i)
        param.append(args[i]).append("\n");
    if (a.sendMessage(param))
        return 0;

//    a.addLibraryPath( a.applicationDirPath() + "/plugins" );
    a.setOrganizationName(AUTHOR);
    a.setApplicationName(APPNAME);
    a.setApplicationVersion(VERSION);
#ifdef Q_WS_WIN
    a.setStyle( new QtDotNetStyle(QtDotNetStyle::Standard) );
#endif

    // Load translation
    QSettings settings;
    QString locale = settings.value("language").toString();
    if (locale.isEmpty())
        locale = QLocale::system().name();

    // application's translation path
    QTranslator appTranslator;
    QString translationPath = getTranslationPath();

    // qt translation
    QTranslator qtTranslator;
    if (installTranslator(a, qtTranslator, QLibraryInfo::location(QLibraryInfo::TranslationsPath), "qt_" + locale) == false)
        installTranslator(a, qtTranslator, translationPath, "qt_" + locale);

    // application translation
    QTranslator myTranslator;
    installTranslator(a, myTranslator, translationPath, "mugo." + locale);

//    QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
//    QTextCodec::setCodecForTr( QTextCodec::codecForLocale() );
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName("UTF-8") );
    QTextCodec::setCodecForTr( QTextCodec::codecForName("UTF-8") );

    MainWindow m;
    m.show();
    a.setMainWindow(&m);
    return a.exec();
}
