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
