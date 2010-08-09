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
#ifndef APPDEF_H
#define APPDEF_H


#define APPNAME "mugo"
#define VERSION "1.1.3"
#define AUTHOR   "nsase"

#define SGF_LINEWIDTH 60

// default value
#define WIN_W 800
#define WIN_H 600
#define BOARD_COLOR QColor(255, 200, 100)
#define WHITE_COLOR QColor(255, 255, 255)
#define BLACK_COLOR QColor(0, 0, 0)
#define COORDINATE_COLOR QColor(0, 0, 0)
#define BG_COLOR QColor(255, 255, 255)
#define BG_TUTOR_COLOR QColor(85, 175, 127)
#define FOCUS_WHITE_COLOR QColor(255, 0, 0)
#define FOCUS_BLACK_COLOR QColor(255, 0, 0)
#define BRANCH_COLOR QColor(00, 0, 255)
#define FAST_MOVE_STEPS 10
#define AUTO_REPLAY_INTERVAL 1300
#define SAVE_NAME "%DT%_%PB%_%PW%"


#include <QFileDialog>
#include <QMap>
#include "godata.h"
QString getOpenFileName(QWidget* parent = 0, const QString& caption = QString(), const QString& dir = QString(), const QString& filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);
QString getSaveFileName(QWidget* parent = 0, const QString& caption = QString(), const QString& dir = QString(), const QString& filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);
int replaceSgfProperty(const go::data* data, const QString& in, QString& out, QMap<QString, QString> addProps = QMap<QString, QString>());

extern QList<QAction*> codecActions;
extern QList<const char*> codecNames;


#if defined(Q_WS_WIN)
#   define strcasecmp _stricmp
#endif


#endif // APPDEF_H
