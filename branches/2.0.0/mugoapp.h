/*
    mugo, sgf editor.
    Copyright (C) 2009-2011 nsase.

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
#ifndef __mugoapp_h__
#define __mugoapp_h__


#include <QApplication>
#include <QList>


#define APP_NAME     "mugo"
#define APP_VERSION  "2.0.0"
#define SETTING_NAME "mugo2"
#define AUTHOR       "nsase"
#define COPYRIGHT    "Copyright 2009-2011 nsase."

// DEFAULT SETTINGS
#define BOARD_IMAGE ":/res/bg.png"
#define BOARD_COLOR QColor(255, 200, 100)
#define SHADOW_COLOR QColor(10, 10, 10, 130)
#define COORDINATE_COLOR Qt::black
#define BG_COLOR Qt::white
#define TUTOR_BG_COLOR QColor(0x20, 0xa0, 0xa0)
#define WHITE_STONE_IMAGE ":/res/white_128_ds.png"
#define BLACK_STONE_IMAGE ":/res/black_128_ds.png"
#define WHITE_STONE_COLOR Qt::white
#define BLACK_STONE_COLOR Qt::black
#define FOCUS_COLOR Qt::red
#define BRANCH_COLOR Qt::blue
#define MOVE_SOUND_FILE "sound/stone.wav"
#define SAVE_FILE_NAME "$(DT)_$(PW)_$(PB)"
#define FAST_MOVE_STEPS 10
#define AUTO_REPLAY_INTERVAL 1000


class QAction;
class QTextCodec;


class MugoApplication;
inline MugoApplication* mugoApp(){
    return (MugoApplication*)qApp;
}


/**
  Mugo Application
*/
class MugoApplication : public QApplication{
    Q_OBJECT
    public:
        MugoApplication(int& argc, char** argv);

        void setDefaultCodec(QTextCodec* codec){ defaultCodec_ = codec; }
        QTextCodec* defaultCodec() const{ return defaultCodec_; }

        void setEncodingActions( const QList<QAction*>& actions ){ encodingActions_ = actions; }
        const QList<QAction*>& encodingActions() const{ return encodingActions_; }

        void setCodecs( QList<QTextCodec*>& codecs ){ codecs_ = codecs; }
        const QList<QTextCodec*>& codecs() const{ return codecs_; }

    private:
        QTextCodec* defaultCodec_;
        QList<QAction*> encodingActions_;
        QList<QTextCodec*> codecs_;
};


/*
#include "godata.h"
int replaceSgfProperty(const Go::NodePtr& game, const QString& in, QString& out, QMap<QString, QString> addProps = QMap<QString, QString>());
*/



#endif
