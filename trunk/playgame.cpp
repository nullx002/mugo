#include <QDebug>
#include "boardwidget.h"
#include "playgame.h"

//class PlayGamePrivate{
//};


PlayGame::PlayGame(QObject *parent)
    : QObject(parent), boardWidget_(NULL), color_(go::empty), boardSize_(9), komi_(6.5), handicap_(0), isAbort_(true), isResign_(false), isNewGame_(true)
{
}

PlayGame::PlayGame(BoardWidget* board, go::color color, int boardSize, const qreal& komi, int handicap, bool newGame, QObject *parent)
    : QObject(parent), boardWidget_(board), color_(color), boardSize_(boardSize), komi_(komi), handicap_(handicap), isAbort_(true), isResign_(false), isNewGame_(newGame)
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
    go::data& data = boardWidget_->getData();

    if (handicap_ == 0)
        return;

    int xpos = data.root->xsize > 9 ? 4 : 3;
    int ypos = data.root->ysize > 9 ? 4 : 3;

    int x[9] = {
        data.root->xsize - xpos,
        xpos - 1,
        data.root->xsize - xpos,
        xpos - 1,
        data.root->xsize / 2,
        data.root->xsize - xpos,
        xpos - 1,
        data.root->xsize / 2,
        data.root->xsize / 2,
    };
    int y[9] = {
        ypos - 1,
        data.root->ysize - ypos,
        data.root->ysize - ypos,
        ypos - 1,
        data.root->ysize / 2,
        data.root->ysize / 2,
        data.root->ysize / 2,
        data.root->ysize - ypos,
        ypos - 1,
    };

    QList<int> xlist, ylist;
    for (int i=0; i<qMin(4, handicap_); ++i){
        xlist.push_back( x[i] );
        ylist.push_back( y[i] );
    }

    if (handicap_ > 5){
        xlist.push_back( x[5] );
        xlist.push_back( x[6] );
        ylist.push_back( y[5] );
        ylist.push_back( y[6] );
    }
    if (handicap_ > 7){
        xlist.push_back( x[7] );
        xlist.push_back( x[8] );
        ylist.push_back( y[7] );
        ylist.push_back( y[8] );
    }

    if (handicap_ > 4 && handicap_ % 2 != 0){
        xlist.push_back( x[4] );
        ylist.push_back( y[4] );
    }

    for (int i=0; i<handicap_; ++i)
        put(go::black, xlist[i], ylist[i]);
}
