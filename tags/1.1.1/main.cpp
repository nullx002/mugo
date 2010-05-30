/*
    mugo, sgf editor.
    Copyright (C) 2009-2010 nsase.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QDebug>
#include <QSettings>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QDir>
#include <QStyle>
#include <QMessageBox>
#ifdef Q_WS_WIN
#  include "qtdotnetstyle.h"
#elif defined(Q_WS_MAC)
#   include <QFileOpenEvent>
#endif
#include "appdef.h"
#include "mugoapp.h"
#include "mainwindow.h"


/**
* Constructor
*/
Application::Application(int argc, char** argv)
    : QtSingleApplication(argc, argv)
    , mainWindow(NULL)
    , qtTranslator(NULL)
    , myTranslator(NULL){
    if (style())
        defaultStyle_ = style()->objectName();
//    a.addLibraryPath( a.applicationDirPath() + "/plugins" );

    setOrganizationName(AUTHOR);
    setApplicationName(APPNAME);
    setApplicationVersion(VERSION);

    connect(this, SIGNAL(messageReceived(const QString&)), SLOT(received(const QString&)));
}

/**
* Destructor
*/
Application::~Application(){
}

/**
* find translation file and install.
*/
void Application::loadTranslation(QString locale){
    // if translator is installed, remove first.
    if (qtTranslator){
        removeTranslator(qtTranslator);
        delete qtTranslator;
    }
    if (myTranslator){
        removeTranslator(myTranslator);
        delete myTranslator;
    }

    // Load translation
    if (locale.isEmpty())
        locale = QLocale::system().name();

    // application's translation path
    QString translationPath = getTranslationPath();

    // qt translation
    qtTranslator = new QTranslator;
    if (installTranslator(qtTranslator, QLibraryInfo::location(QLibraryInfo::TranslationsPath), "qt_" + locale) == false)
        if (installTranslator(qtTranslator, translationPath, "qt_" + locale) == false){
            delete qtTranslator;
            qtTranslator = NULL;
        }

    // application translation
    myTranslator = new QTranslator;
    if (installTranslator(myTranslator, translationPath, "mugo." + locale) == false){
        delete myTranslator;
        myTranslator = NULL;
    }
}

/**
* install translator
*/
bool Application::installTranslator(QTranslator* translator, const QString& path, QString file){
    if (translator->load(file, path) == false){
        file = file.toLower();
        if (translator->load(file, path) == false)
            return false;
    }
    QtSingleApplication::installTranslator(translator);
    return true;
}

/**
* get translation path
*/
QString Application::getTranslationPath(){
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

    foreach( const QString& path, pathList){
        QDir dir(path);
        if ( QDir(path).exists() )
            return path;
    }

    return "./";
}

/**
* set window style
*/
void Application::setWindowStyle(const QString& style){
    if (style.isEmpty() == false)
        setStyle(style);
    else
#if defined(Q_WS_WIN)
        setStyle( new QtDotNetStyle(QtDotNetStyle::Standard) );
#elif defined(Q_WS_MAC)
        setStyle( "macintosh" );
#else
        setStyle( defaultStyle_ );
#endif
}

/**
* event
* in mac, filename is not set to argc/argv
*/
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

/**
* receive message from other instance.
*/
void Application::received(const QString& msg){
    if (mainWindow == NULL)
        return;

    QStringList files = msg.split("\n");
    foreach(const QString& f, files)
        if (f.isEmpty() == false)
            mainWindow->fileOpen(f);

#ifdef Q_WS_WIN
    // Is not highlighted on taskbar automatically?
    ::SetActiveWindow( mainWindow->winId() );
#endif
}

/**
* set main window
*/
void Application::setMainWindow(MainWindow* win){
    setActivationWindow(win);
    mainWindow = win;
}

/**
* entory point
*/
int main(int argc, char *argv[])
{
    putenv( "UNICODEMAP_JP=cp932" );

    // application
    Application a(argc, argv);

    // if 2nd instance, file open in 1st instance.
    QStringList args = a.arguments();
    QString param;
    for(int i=1; i<args.size(); ++i)
        param.append(args[i]).append("\n");
    if (a.sendMessage(param))
        return 0;

    // settings
    QSettings settings;

    // window style
    a.setWindowStyle( settings.value("style").toString() );

    // Load translation
    a.loadTranslation( settings.value("language").toString() );

    // set codecs
//    QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
//    QTextCodec::setCodecForTr( QTextCodec::codecForLocale() );
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName("UTF-8") );
    QTextCodec::setCodecForTr( QTextCodec::codecForName("UTF-8") );

    // create main window
    MainWindow m;
    m.show();
    a.setMainWindow(&m);
    return a.exec();
}
