#include <QDebug>
#include "boardwidget.h"
#include "playgame.h"

//class PlayGamePrivate{
//};


PlayGame::PlayGame(BoardWidget* board, go::color color, QObject*parent) : QObject(parent), boardWidget_(board), yourColor_(color), isResign_(false)
{
}

PlayGame::~PlayGame(){
}

go::color PlayGame::yourColor() const{
    return yourColor_;
}

bool PlayGame::isGameEnd() const{
    const go::nodeList& nodeList = boardWidget_->getCurrentNodeList();
    if (nodeList.size() < 2)
        return false;

    go::nodeList::const_iterator iter = nodeList.end();
    const go::nodePtr& node1 = *--iter;
    const go::nodePtr& node2 = *--iter;
    return node1->isPass() && node2->isPass();
}

void PlayGame::setHandicap(){
    go::data& data = boardWidget_->getData();

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
    for (int i=0; i<qMin(4, data.root->handicap); ++i){
        xlist.push_back( x[i] );
        ylist.push_back( y[i] );
    }

    if (data.root->handicap > 5){
        xlist.push_back( x[5] );
        xlist.push_back( x[6] );
        ylist.push_back( y[5] );
        ylist.push_back( y[6] );
    }
    if (data.root->handicap > 7){
        xlist.push_back( x[7] );
        xlist.push_back( x[8] );
        ylist.push_back( y[7] );
        ylist.push_back( y[8] );
    }

    if (data.root->handicap > 4 && data.root->handicap % 2 != 0){
        xlist.push_back( x[4] );
        ylist.push_back( y[4] );
    }

    for (int i=0; i<data.root->handicap; ++i)
        put(go::black, xlist[i], ylist[i]);
}
