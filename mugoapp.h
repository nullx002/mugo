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
#ifndef MUGO_APPLICATION_H
#define MUGO_APPLICATION_H

#include "qtsingleapplication.h"
#include "appdef.h"

class MainWindow;

class Application : public QtSingleApplication{
Q_OBJECT
public:
    Application(int& argc, char** argv);
    virtual ~Application();

    // translation
    void    loadTranslation(QString locale);
    bool    installTranslator(QTranslator* translator, const QString& path, QString file);
    QString getTranslationPath();

    // window style
    void    setWindowStyle(const QString& style);

#if defined(Q_WS_MAC)
    bool event(QEvent*);
#endif

    void setMainWindow(MainWindow* win);

    const QString& defaultStyle() const{ return defaultStyle_; }

public slots:
    void received(const QString& msg);

private:
    MainWindow*  mainWindow;
    QTranslator* qtTranslator;
    QTranslator* myTranslator;
    QString defaultStyle_;
};


#endif
