#ifndef PLAYGAME_H
#define PLAYGAME_H

#include <QObject>
#include "godata.h"

class BoardWidget;
//class PlayGamePrivate;

class PlayGame : public QObject{
Q_OBJECT

public:
    PlayGame(BoardWidget* board, go::color color, bool newGame, QObject *parent = 0);
    virtual ~PlayGame();

    bool isNewGame() const{ return isNewGame_; }

    virtual bool undo() = 0;

    virtual bool move(int x, int y) = 0;
    virtual bool put(go::color c, int x, int y) = 0;
    virtual bool wait() = 0;
    virtual bool moving() const = 0;

    go::color yourColor() const;
    bool isGameEnd() const;
    void setHandicap();
    bool isResign() const{ return isResign_; }
    void setResign(bool resign){ isResign_ = resign; }

signals:
    void gameEnded();

protected:
    BoardWidget* boardWidget_;
    go::color color_, yourColor_;
    bool isResign_;
    bool isNewGame_;

//Q_DECLARE_PRIVATE(PlayGame)
};

#endif // PLAYGAME_H
