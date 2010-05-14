#include <QDebug>
#include "boardwidget.h"
#include "playgame.h"

//class PlayGamePrivate{
//};


PlayGame::PlayGame(QObject *parent)
    : QObject(parent), boardWidget_(NULL), color_(go::empty), isAbort_(true), isResign_(false), isNewGame_(true)
{
}

PlayGame::PlayGame(BoardWidget* board, QObject *parent)
    : QObject(parent), boardWidget_(board), color_(go::empty), isAbort_(true), isResign_(false), isNewGame_(false)
{
}

PlayGame::PlayGame(BoardWidget* board, go::color color, bool newGame, QObject *parent)
    : QObject(parent), boardWidget_(board), color_(color), isAbort_(true), isResign_(false), isNewGame_(newGame)
{
}

PlayGame::~PlayGame(){
}

go::color PlayGame::color() const{
    return color_;
}

bool PlayGame::isGameEnd() const{
    const go::nodePtr& node1 = boardWidget_->getCurrentNode();
    const go::nodePtr& node2 = node1->parent();

    if ( node2 == NULL || node2->color == go::empty || node1->color == go::empty)
        return false;

    return node1->isPass() && node2->isPass();
}

void PlayGame::setHandicap(){
    go::informationPtr& root = boardWidget_->getData().root;

    if ( root->handicap == 0)
        return;

    int xpos = root->xsize > 9 ? 4 : 3;
    int ypos = root->ysize > 9 ? 4 : 3;

    int x[9] = {
        root->xsize - xpos,
        xpos - 1,
        root->xsize - xpos,
        xpos - 1,
        root->xsize / 2,
        root->xsize - xpos,
        xpos - 1,
        root->xsize / 2,
        root->xsize / 2,
    };
    int y[9] = {
        ypos - 1,
        root->ysize - ypos,
        root->ysize - ypos,
        ypos - 1,
        root->ysize / 2,
        root->ysize / 2,
        root->ysize / 2,
        root->ysize - ypos,
        ypos - 1,
    };

    QList<int> xlist, ylist;
    for (int i=0; i<qMin(4, root->handicap); ++i){
        xlist.push_back( x[i] );
        ylist.push_back( y[i] );
    }

    if (root->handicap > 5){
        xlist.push_back( x[5] );
        xlist.push_back( x[6] );
        ylist.push_back( y[5] );
        ylist.push_back( y[6] );
    }
    if (root->handicap > 7){
        xlist.push_back( x[7] );
        xlist.push_back( x[8] );
        ylist.push_back( y[7] );
        ylist.push_back( y[8] );
    }

    if (root->handicap > 4 && root->handicap % 2 != 0){
        xlist.push_back( x[4] );
        ylist.push_back( y[4] );
    }

    for (int i=0; i<root->handicap; ++i)
        put(go::black, xlist[i], ylist[i]);
}
