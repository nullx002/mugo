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
#ifndef PLAYGAME_H
#define PLAYGAME_H

#include <QObject>
#include "godata.h"

class BoardWidget;
//class PlayGamePrivate;

class PlayGame : public QObject{
Q_OBJECT

public:
    PlayGame(QObject *parent = 0);
    PlayGame(BoardWidget* board, QObject *parent = 0);
    PlayGame(BoardWidget* board, go::color color, bool newGame, QObject *parent = 0);
    virtual ~PlayGame();

    bool isNewGame() const{ return isNewGame_; }

    virtual bool isInitialized() const{ return true; }
    virtual void kill() = 0;
    virtual bool moving() const = 0;
    virtual bool undo() = 0;
    virtual bool move(int x, int y) = 0;
    virtual bool put(go::color c, int x, int y) = 0;
    virtual bool quit(bool resign) = 0;
    virtual bool abort() = 0;
    virtual bool wait() = 0;

    go::color color() const;
    bool isGameEnd() const;
    void setHandicap();
    bool isAbort() const{ return isAbort_; }
    void setAbort(bool abort){ isAbort_ = abort; }
    bool isResign() const{ return isResign_; }
    void setResign(bool resign){ isResign_ = resign; }
    void winner(go::color color){ winner_ = color; }
    go::color winner() const{ return winner_; }

signals:
    void gameEnded();

protected:
    BoardWidget* boardWidget_;
    go::color color_;
    bool isAbort_;
    bool isResign_;
    bool isNewGame_;
    go::color winner_;

//Q_DECLARE_PRIVATE(PlayGame)
};

#endif // PLAYGAME_H
