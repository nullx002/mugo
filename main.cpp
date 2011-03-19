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
#include <QSettings>
#include <QDateTime>
#include "mugoapp.h"
#include "mainwindow.h"
#include "sgf.h"


/**
  Constructor
*/
MugoApplication::MugoApplication(int& argc, char** argv) : QApplication(argc, argv){
    setApplicationName(SETTING_NAME);
    setApplicationVersion(APP_VERSION);
    setOrganizationDomain(AUTHOR);

    // Settings
    QSettings settings;

    // default codec
    defaultCodec_ = QTextCodec::codecForName( settings.value("defaultCodec", "UTF-8").toByteArray() );
}


int replaceSgfProperty(const Go::NodePtr& game, const QString& in, QString& out, QMap<QString, QString> addProps){
    QDateTime dt = QDateTime::currentDateTime();
    addProps.insert("DATETIME", dt.toString(Qt::DefaultLocaleShortDate));
    addProps.insert("DATE", dt.date().toString(Qt::DefaultLocaleShortDate));
    addProps.insert("TIME", dt.time().toString(Qt::DefaultLocaleShortDate));

    Go::Sgf::Node node;
    node.set(game);

    out = in;
    int count = 0;
    int pos = 0;

    QRegExp rx("\\$\\(.+\\)");
    rx.setMinimal(true);
    while ((pos = rx.indexIn(out, pos)) != -1){
        QString cap = rx.cap(0);
        QString prop = cap.mid(2, cap.size() - 3);
        QString value;

        QMap<QString, QString>::iterator iter1 = addProps.find(prop);
        if (iter1 != addProps.end())
            value = *iter1;
        else{
            Go::Sgf::PropertyType::const_iterator iter2 = node.properties.find(prop);
            if (iter2 != node.properties.end())
                foreach(const QString& v, *iter2)
                    value.append(v);
        }

        out.replace(pos, cap.size(), value);
        pos += value.size();
        if (value.isEmpty() == false)
            ++count;
    }

    out.remove('/');

    return count;
}



int main(int argc, char *argv[])
{
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName("UTF-8") );
    QTextCodec::setCodecForTr( QTextCodec::codecForName("UTF-8") );

    MugoApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
