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
#include <QDebug>
#include <QResizeEvent>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include "boardwidget.h"
#include "sgfdocument.h"
#include "command.h"

/**
  Constructor
*/
BoardWidget::BoardWidget(SgfDocument* doc, QWidget *parent) :
    QGraphicsView(parent),
    document_(doc),
    scene( new QGraphicsScene(this) )
{
//    connect(document_, SIGNAL(nodeAdded(Go::NodePtr)), SLOT(on_sgfdocument_nodeAdded(Go::NodePtr)));
    connect(document_, SIGNAL(nodeDeleted(Go::NodePtr, bool)), SLOT(on_sgfdocument_nodeDeleted(Go::NodePtr, bool)));

    setScene(scene);

    // create board
    shadow = scene->addRect(0, 0, 1, 1, QPen(Qt::transparent), QBrush(QColor(10, 10, 10, 130)));
    board  = scene->addRect(0, 0, 1, 1, QPen(QColor(255, 200, 100)), QBrush(QColor(255, 200, 100)));

    // set current game
    setCurrentGame(document()->gameList.front());
}

/**
  Destructor
*/
BoardWidget::~BoardWidget()
{
}

/**
  resize
*/
void BoardWidget::resizeEvent(QResizeEvent* e){
    scene->setSceneRect(0, 0, e->size().width(), e->size().height());

    // calculate board size
    int width  = e->size().width() - 30;
    int height = e->size().height() - 30;
    int x = width  / gameInformation->xsize;
    int y = height / gameInformation->ysize;
    int size = qMin(x, y);
    int w = size * (gameInformation->xsize - 1);
    int h = size * (gameInformation->ysize - 1);

    x = (e->size().width() - w) / 2;
    y = (e->size().height() - h) / 2;
    QRect boardRect(QPoint(x-size*0.8, y-size*0.8), QPoint(x+w+size*0.8, y+w+size*0.8));

    // set position of shadow.
    shadow->setRect(boardRect.left()+4, boardRect.top()+4, boardRect.width(), boardRect.height());

    // set position of board.
    board->setRect(boardRect);

    // set position of vertical lines.
    int xx = x;
    foreach(QGraphicsLineItem* item, vLines){
        item->setLine(xx, y, xx, y+h);
        xx += size;
    }

    // set position of horizontal lines.
    int yy = y;
    foreach(QGraphicsLineItem* item, hLines){
        item->setLine(x, yy, x+w, yy);
        yy += size;
    }

    // set position of stars.
    QList<int> xpos, ypos;
    getStarPosition(xpos, ypos);
    for (int y=0, n=0; y<ypos.size(); ++y){
        for (int x=0; x<xpos.size(); ++x, ++n){
            int xx = vLines[xpos[x]]->line().x1() - 2;
            int yy = hLines[ypos[y]]->line().y1() - 2;
            stars[n]->setRect(xx, yy, 4, 4);
        }
    }

    // set position of stones.
    Go::NodeList::iterator iter = currentNodeList.begin();
    foreach (QGraphicsItem* item, stones){
        if (item){
            int x = (*iter)->position.x;
            int y = (*iter)->position.y;
            QGraphicsEllipseItem* ellipse = dynamic_cast<QGraphicsEllipseItem*>(item);
            if (ellipse)
                ellipse->setRect(vLines[x]->line().x1()-size/2, hLines[y]->line().y1()-size/2, size, size);
        }
        ++iter;
    }
}

/**
  Mouse Wheel
*/
void BoardWidget::wheelEvent(QWheelEvent* e){
    if (e->delta() > 0)
        back();
    else if (e->delta() < 0)
        forward();
}

/**
  Mouse Release
*/
void BoardWidget::mouseReleaseEvent(QMouseEvent* e){
    if (e->button() == Qt::LeftButton)
        onLButtonDown(e);
    else if (e->button() == Qt::RightButton)
        onRButtonDown(e);
}

/**
    LeftButton Down
*/
void BoardWidget::onLButtonDown(QMouseEvent* e){
    // get mouse position
    int size = vLines[1]->line().x1() - vLines[0]->line().x1();
    int x = (e->x() - vLines[0]->line().x1() + size / 2) / size;
    int y = (e->y() - hLines[0]->line().y1() + size / 2) / size;

    if (x < 0 || y < 0 || x >= gameInformation->xsize || y >= gameInformation->ysize)
        return;
    else if (boardBuffer[y][x].isStone())
        return;

    // create node
    Go::NodePtr node;
    if (currentNode->nextColor == Go::black)
        node = Go::createBlackNode(currentNode, x, y);
    else if (currentNode->nextColor == Go::white)
        node = Go::createWhiteNode(currentNode, x, y);
    else if (currentNode->isBlack())
        node = Go::createWhiteNode(currentNode, x, y);
    else if (currentNode->isWhite())
        node = Go::createBlackNode(currentNode, x, y);
    else
        return;

    document()->getUndoStack()->push( new AddNodeCommand(document(), currentNode, node, -1) );
    setCurrentNode(node);
}

/**
    RightButton Down
*/
void BoardWidget::onRButtonDown(QMouseEvent* /*e*/){
}

/**
  Set Document
*/
void BoardWidget::setDocument(SgfDocument* doc){
    document_ = doc;
    setCurrentGame(doc->gameList[0]);
};

/**
  Set Current Game
*/
void BoardWidget::setCurrentGame(Go::NodePtr game){
    if (game == currentGame)
        return;

    currentGame = game;
    gameInformation = currentGame->getInformation();

    // create line
    int xsize = gameInformation->xsize;
    int ysize = gameInformation->ysize;
    for (int y=0; y<ysize; ++y)
        hLines.push_back( scene->addLine(0, 0, 0, 0, QPen(Qt::black)) );
    for (int x=0; x<xsize; ++x)
        vLines.push_back( scene->addLine(0, 0, 0, 0, QPen(Qt::black)) );

    // create star
    QList<int> xpos, ypos;
    getStarPosition(xpos, ypos);
    for (int y=0; y<ypos.size(); ++y)
        for (int x=0; x<xpos.size(); ++x)
            stars.push_back( scene->addEllipse(x, y, 2, 2, QPen(Qt::black), QBrush(Qt::black)) );

    emit currentGameChanged(currentGame);
    setCurrentNode(game);
}

/**
  Set Current Node
*/
void BoardWidget::setCurrentNode(Go::NodePtr node){
    if (node == currentNode)
        return;

    currentNode = node;
    gameInformation = currentNode->getInformation();

    Go::NodeList::iterator iter = qFind(currentNodeList.begin(), currentNodeList.end(), currentNode);
    if (iter == currentNodeList.end())
        createBuffer(true);
    else
        createBuffer(false);

    emit currentNodeChanged(currentNode);
}

/**
  create buffer
*/
void BoardWidget::createBuffer(bool erase){
    if (erase){
        int size = vLines[1]->line().x1() - vLines[0]->line().x1();

        // clear buffer
        currentNodeList.clear();
        foreach(QGraphicsItem* item, stones)
            if (item)
                scene->removeItem(item);
        stones.clear();

        // create currentNodeList
        Go::NodePtr n = currentNode;
        while(n){
            currentNodeList.push_front(n);

            if ( inBoard(n) ){
                int sceneX = vLines[n->position.x]->line().x1() - size / 2;
                int sceneY = hLines[n->position.y]->line().y1() - size / 2;
                QGraphicsItem* item = scene->addEllipse( sceneX, sceneY, size, size, QPen(Qt::black), QBrush(n->color == Go::black ? Qt::black : Qt::white) );
                stones.push_front(item);
            }
            else
                stones.push_front(NULL);

            n = n->parent();
        }

        n = currentNode;
        while(n->childNodes.empty() == false){
            n = n->childNodes.front();
            currentNodeList.push_back(n);

            if ( inBoard(n) ){
                int sceneX = vLines[n->position.x]->line().x1() - size / 2;
                int sceneY = hLines[n->position.y]->line().y1() - size / 2;
                QGraphicsItem* item = scene->addEllipse( sceneX, sceneY, size, size, QPen(Qt::black), QBrush(n->color == Go::black ? Qt::black : Qt::white) );
                stones.push_back(item);
            }
            else
                stones.push_back(NULL);
        }
    }

    // create boardBuffer
    boardBuffer.clear();
    boardBuffer.resize(gameInformation->ysize);
    for (int i=0; i<boardBuffer.size(); ++i)
        boardBuffer[i].resize(gameInformation->xsize);

    Go::NodeList::iterator node = currentNodeList.begin();
    QList<QGraphicsItem*>::iterator stone = stones.begin();
    while(node != currentNodeList.end()){
        if ( inBoard(*node) ){
            int x = (*node)->position.x;
            int y = (*node)->position.y;

            TerritoryInfo ti;
            ti.color = (*node)->color;
            ti.stone = *stone;
            if (*stone)
                (*stone)->show();

            boardBuffer[y][x] = ti;

            killStones(x, y);
        }

        ++stone;
        if (*node == currentNode)
            break;
        ++node;
    };

    while (stone != stones.end()){
        if (*stone)
            (*stone)->hide();
        ++stone;
    }
}

/**
  Move next node
*/
void BoardWidget::forward(){
    Go::NodeList::const_iterator iter = qFind(currentNodeList, currentNode);
    if (iter == currentNodeList.end() || iter + 1 == currentNodeList.end())
        return;
    setCurrentNode(*++iter);
}

/**
  Move previous node
*/
void BoardWidget::back(){
    Go::NodeList::const_iterator iter = qFind(currentNodeList, currentNode);
    if (iter == currentNodeList.begin() || iter == currentNodeList.end())
        return;
    setCurrentNode(*--iter);
}

QString BoardWidget::getCoordinateString(Go::NodePtr node, bool showI) const{
    QString str;
    Go::GameInformationPtr info = node->getInformation();
    int x = node->position.x % 26;
    if (showI == false && x > 7)
        ++x;
    int y = info->ysize - node->position.y;
    str.sprintf("%c%d", 'A'+x, y);
    return str;
}

/**
  Get Star Position
*/
void BoardWidget::getStarPosition(QList<int>& xpos, QList<int>& ypos){
    int xsize = gameInformation->xsize;
    int ysize = gameInformation->ysize;

    xpos.push_back(xsize > 9 ? 3 : 2);
    xpos.push_back(xsize > 9 ? xsize-4 : xsize-3);
    ypos.push_back(ysize > 9 ? 3 : 2);
    ypos.push_back(ysize > 9 ? ysize-4 : ysize-3);

    if (xsize % 2 != 0 && xsize >= 9)
        xpos.push_back(xsize / 2);

    if (ysize % 2 != 0 && ysize >= 9)
        ypos.push_back(ysize / 2);
}

/**
  kill stone
*/
void BoardWidget::killStones(int x, int y){
    Go::Color color = boardBuffer[y][x].color;
    if (color == Go::empty)
        return;

    int xsize = gameInformation->xsize;
    int ysize = gameInformation->ysize;

    char* buf = new char[ysize * xsize];

    memset(buf, 0, ysize*xsize);
    if (canKillStones(x-1, y, color == Go::black ? Go::white : Go::black, buf) == true)
        killStones(buf);

    memset(buf, 0, ysize*xsize);
    if (canKillStones(x+1, y, color == Go::black ? Go::white : Go::black, buf) == true)
        killStones(buf);

    memset(buf, 0, ysize*xsize);
    if (canKillStones(x, y-1, color == Go::black ? Go::white : Go::black, buf) == true)
        killStones(buf);

    memset(buf, 0, ysize*xsize);
    if (canKillStones(x, y+1, color == Go::black ? Go::white : Go::black, buf) == true)
        killStones(buf);

    delete[] buf;
}

/**
  kill stone
*/
void BoardWidget::killStones(char* buf){
    for (int y=0; y<boardBuffer.size(); ++y){
        for (int x=0; x<boardBuffer[y].size(); ++x){
            if (buf[y*gameInformation->xsize+x]){
                boardBuffer[y][x].color = Go::empty;
                boardBuffer[y][x].stone->hide();
                boardBuffer[y][x].stone = NULL;
            }
        }
    }
}

/**
*/
bool BoardWidget::canKillStones(int x, int y, Go::Color color, char* buf){
    if (x < 0 || x >= gameInformation->xsize || y < 0 || y >= gameInformation->ysize)
        return true;
    else if (boardBuffer[y][x].color == Go::empty)
        return false;
    else if (boardBuffer[y][x].color != color)
        return true;

    if (buf[y * gameInformation->xsize + x])
        return true;

    buf[y * gameInformation->xsize + x] = 1;

    if (canKillStones(x-1, y, color, buf) == false)
        return false;

    if (canKillStones(x+1, y, color, buf) == false)
        return false;

    if (canKillStones(x, y-1, color, buf) == false)
        return false;

    if (canKillStones(x, y+1, color, buf) == false)
        return false;

    return true;
}

/**
*/
bool BoardWidget::inBoard(Go::NodePtr node){
    if (node->isStone() == false || node->isPass() == true)
        return false;

    if (node->position.x >= gameInformation->xsize || node->position.y >= gameInformation->ysize)
        return false;

    return true;
}

/**
  Slot
  node added
*/
/*
void BoardWidget::on_sgfdocument_nodeAdded(Go::NodePtr node){
    setCurrentNode(node);
}
*/

/**
  Slot
  node deleted
*/
void BoardWidget::on_sgfdocument_nodeDeleted(Go::NodePtr node, bool /*removeChild*/){
    setCurrentNode(node->parent());
    Go::NodeList::iterator iter = qFind(currentNodeList.begin(), currentNodeList.end(), node);
    if (iter != currentNodeList.end()){
        iter = currentNodeList.erase(iter);
        createBuffer(true);
    }
}
