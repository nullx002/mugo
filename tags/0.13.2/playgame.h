#ifndef PLAYGAME_H
#define PLAYGAME_H

#include <QObject>
#include "godata.h"

class BoardWidget;
//class PlayGamePrivate;

class PlayGame : public QObject{
Q_OBJECT

public:
    PlayGame(BoardWidget* board, go::color color, QObject *parent = 0);
    virtual ~PlayGame();

    virtual void move(int x, int y) = 0;
    virtual bool moving() const = 0;
    virtual void put(go::color c, int x, int y) = 0;
    virtual void wait() = 0;

    go::color yourColor() const;
    bool isGameEnd() const;
    void setHandicap();
    bool isResign() const{ return isResign_; }

signals:
    void gameEnded();

protected:
    BoardWidget* boardWidget_;
    go::color yourColor_;
    bool isResign_;

//Q_DECLARE_PRIVATE(PlayGame)
};

#endif // PLAYGAME_H
