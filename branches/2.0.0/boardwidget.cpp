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
#include <QGraphicsSimpleTextItem>
#include <QInputDialog>
#include "boardwidget.h"
#include "sgfdocument.h"
#include "command.h"

#define _USE_MATH_DEFINES
#include <math.h>

/**
  GraphicsLabelTextItem

  draw simple text and fill background.
*/
class GraphicsLabelTextItem : public QGraphicsSimpleTextItem{
    public:
        GraphicsLabelTextItem(QGraphicsItem* parent = 0);
        GraphicsLabelTextItem(const QString& text, QGraphicsItem* parent = 0);

        void setBackgroundBrush(const QBrush& b){ backgroundBrush_ = b; update(); }
        QBrush backgroundBrush() const{ return backgroundBrush_; }

    protected:
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

        QBrush backgroundBrush_;
};

/**
  Constructor
*/
GraphicsLabelTextItem::GraphicsLabelTextItem(QGraphicsItem* parent)
    : QGraphicsSimpleTextItem(parent)
{
}

/**
  Constructor
*/
GraphicsLabelTextItem::GraphicsLabelTextItem(const QString& text, QGraphicsItem* parent)
    : QGraphicsSimpleTextItem(text, parent)
{
}

/**
  Paint
*/
void GraphicsLabelTextItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
    painter->fillRect(boundingRect(), backgroundBrush_);
    QGraphicsSimpleTextItem::paint(painter, option, widget);
}


/**
  GraphicsArrowItem

  draw arrow line.
*/
class GraphicsArrowItem : public QGraphicsPathItem{
    public:
        enum Shape{ none, normal, vee };

        GraphicsArrowItem(QGraphicsItem* parent = 0);
        GraphicsArrowItem(qreal x1, qreal y1, qreal x2, qreal y2, Shape start = none, Shape end = none, QGraphicsItem* parent = 0);

        void setLine(qreal x1, qreal y1, qreal x2, qreal y2);

    protected:
        void createNormal(QPainterPath& path, qreal x1, qreal y1, qreal x2, qreal y2);

        Shape start;
        Shape end;
};

/**
  Constructor
*/
GraphicsArrowItem::GraphicsArrowItem(QGraphicsItem* parent)
    : QGraphicsPathItem(parent)
{
}

/**
  Constructor
*/
GraphicsArrowItem::GraphicsArrowItem(qreal x1, qreal y1, qreal x2, qreal y2, Shape start_, Shape end_, QGraphicsItem* parent)
    : QGraphicsPathItem(parent)
    , start(start_)
    , end(end_)
{
    setLine(x1, y1, x2, y2);
}

void GraphicsArrowItem::setLine(qreal x1, qreal y1, qreal x2, qreal y2){
    QPainterPath path;
    path.moveTo(x1, y1);
    path.lineTo(x2, y2);

    if (start == normal)
        createNormal(path, x2, y2, x1, y1);

    if (end == normal)
        createNormal(path, x1, y1, x2, y2);

    setPath(path);
}

void GraphicsArrowItem::createNormal(QPainterPath& path, qreal x1, qreal y1, qreal x2, qreal y2){
    QPainterPath p;
    p.moveTo(0, 0);
    p.lineTo(-10, -5);
    p.lineTo(-10, 5);
    p.lineTo(0, 0);

    qreal x = x2 - x1;
    qreal y = y2 - y1;
    qreal r = acos(x / sqrt(x*x + y*y));
    qreal deg = (r / M_PI) * 180.0;

    if (y < 0)
        deg = 360 - deg;

    QMatrix m;
    m.translate(x2, y2);
    m.rotate(deg);

    path.addPath(p * m);

}

/**
  Constructor
*/
BoardWidget::BoardWidget(SgfDocument* doc, QWidget *parent)
    : QGraphicsView(parent)
    , document_(doc)
    , scene( new QGraphicsScene(this) )
    , editMode(EditMode::alternateMove)
    , jumpToClicked(false)
    , showMoveNumber(true)
    , resetMoveNumberInBranch(false)
    , showMoveNumberCount(-1)
{
//    connect(document_, SIGNAL(nodeAdded(Go::NodePtr)), SLOT(on_sgfdocument_nodeAdded(Go::NodePtr)));
    connect(document_, SIGNAL(nodeDeleted(Go::NodePtr, bool)), SLOT(on_sgfdocument_nodeDeleted(Go::NodePtr, bool)));
    connect(document_, SIGNAL(nodeModified(Go::NodePtr, bool)), SLOT(on_sgfdocument_nodeModified(Go::NodePtr, bool)));
    connect(document_, SIGNAL(gameModified(Go::NodePtr)), SLOT(on_sgfdocument_gameModified(Go::NodePtr)));

    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    setScene(scene);

    // create board
    shadow = scene->addRect(0, 0, 1, 1, QPen(Qt::transparent), QBrush(QColor(10, 10, 10, 130)));
    board  = scene->addRect(0, 0, 1, 1, QPen(QColor(255, 200, 100)), QBrush(QColor(255, 200, 100)));
//    board  = scene->addRect(0, 0, 1, 1, QPen(QColor(255, 200, 100)), QBrush(QPixmap(":/res/bg.png")));
    board->setZValue(1);

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
    setItemsPosition();
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

    // if jump to clicked mode, change current node and return
    if (jumpToClicked){
        Go::NodePtr node = currentNode;
        while (node){
            if (node->isStone() && node->x() == x && node->y() == y)
                break;
            node = node->parent();
        }
        if (node){
            jumpToClicked = false;
            setCurrentNode(node, true);
        }
        return;
    }

    switch(editMode){
        case EditMode::alternateMove:
            alternateMove(x, y);
            break;

        case EditMode::addBlack:
            addStone(x, y, Go::black);
            break;

        case EditMode::addWhite:
            addStone(x, y, Go::white);
            break;

        case EditMode::addEmpty:
            addStone(x, y, Go::empty);
            break;

        case EditMode::addLabel:
            addLabel(x, y, true);
            break;

        case EditMode::addLabelManually:
            addLabel(x, y, false);
            break;

        case EditMode::addCircle:
            addMarker(x, y, Go::Mark::circle);
            break;

        case EditMode::addCross:
            addMarker(x, y, Go::Mark::cross);
            break;

        case EditMode::addTriangle:
            addMarker(x, y, Go::Mark::triangle);
            break;

        case EditMode::addSquare:
            addMarker(x, y, Go::Mark::square);
            break;

        case EditMode::removeMarker:
            removeMarker(x, y);
            break;
    }
}

/**
    RightButton Down
*/
void BoardWidget::onRButtonDown(QMouseEvent* /*e*/){
    document()->getUndoStack()->undo();
}

/**
  get next color
*/
Go::Color BoardWidget::getNextColor() const{
    Go::NodePtr node = currentNode;
    while(node){
        if (node->nextColor == Go::white)
            return Go::white;
        else if (node->nextColor == Go::black)
            return Go::black;
        else if (node->color == Go::white)
            return Go::black;
        else if (node->color == Go::black)
            return Go::white;

        node = node->parent();
    }
    return Go::empty;
}

/**
  Set Document
*/
void BoardWidget::setDocument(SgfDocument* doc){
    document_ = doc;
    setCurrentGame(doc->gameList[0], true);
};

/**
  Set Current Game
*/
void BoardWidget::setCurrentGame(Go::NodePtr game, bool forceChange){
    if (forceChange == false && game == currentGame)
        return;

    currentGame = game;
    gameInformation = currentGame->getInformation();
    currentNodeList.clear();

    // delete lines and stars
    qDeleteAll(hLines);
    hLines.clear();

    qDeleteAll(vLines);
    vLines.clear();

    qDeleteAll(stars);
    stars.clear();

    // create lines
    int xsize = gameInformation->xsize;
    int ysize = gameInformation->ysize;
    for (int y=0; y<ysize; ++y){
        QGraphicsLineItem* item = scene->addLine(0, 0, 0, 0, QPen(Qt::black));
        item->setZValue(2);
        hLines.push_back(item);
    }
    for (int x=0; x<xsize; ++x){
        QGraphicsLineItem* item = scene->addLine(0, 0, 0, 0, QPen(Qt::black));
        item->setZValue(2);
        vLines.push_back(item);
    }

    // create stars
    QList<int> xpos, ypos;
    getStarPosition(xpos, ypos);
    for (int y=0; y<ypos.size(); ++y)
        for (int x=0; x<xpos.size(); ++x){
            QGraphicsEllipseItem* item = scene->addEllipse(x, y, 2, 2, QPen(Qt::black), QBrush(Qt::black));
            item->setZValue(2);
            stars.push_back(item);
        }

    setItemsPosition();

    emit currentGameChanged(currentGame);

    setCurrentNode(game, forceChange);
}

/**
  Set Current Node
*/
void BoardWidget::setCurrentNode(Go::NodePtr node, bool forceChange){
    if (forceChange == false && node == currentNode)
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
  set scene items position
*/
void BoardWidget::setItemsPosition(){
    QRectF r = scene->sceneRect();

    // calculate board size
    int width  = r.width();
    int height = r.height();
    int gridW = width  / (gameInformation->xsize + 1);
    int gridH = height / (gameInformation->ysize + 1);
    int gridSize = qMin(gridW, gridH);
    int w = gridSize * (gameInformation->xsize - 1);
    int h = gridSize * (gameInformation->ysize - 1);

    int x = (r.width() - w) / 2;
    int y = (r.height() - h) / 2;
    int margin = gridSize * 0.8;
    QRect boardRect(QPoint(x-margin, y-margin), QPoint(x+w+margin, y+h+margin));

    // set position of shadow.
    shadow->setRect(boardRect.left()+4, boardRect.top()+4, boardRect.width(), boardRect.height());

    // set position of board.
    board->setRect(boardRect);

    // set position of vertical lines.
    int xx = x;
    foreach(QGraphicsLineItem* item, vLines){
        item->setLine(xx, y, xx, y+h);
        xx += gridSize;
    }

    // set position of horizontal lines.
    int yy = y;
    foreach(QGraphicsLineItem* item, hLines){
        item->setLine(x, yy, x+w, yy);
        yy += gridSize;
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

    // set position of stones and move number.
    Go::NodeList::iterator node = currentNodeList.begin();
    QList<QGraphicsItem*>::iterator stone = stones.begin();
    QList< QList<QGraphicsItem*> >::iterator iter_bstones = blackStones.begin();
    QList< QList<QGraphicsItem*> >::iterator iter_wstones = whiteStones.begin();
    QList< QList<QGraphicsItem*> >::iterator iter_marks = marks.begin();
    QList< QList<QGraphicsItem*> >::iterator iter_territories = territories.begin();
    QList< QList<QGraphicsRectItem*> >::iterator iter_dims = dims.begin();
    QList< QList<GraphicsArrowItem*> >::iterator iter_lines = lines.begin();
    QList<QGraphicsSimpleTextItem*>::iterator iter_number = numbers.begin();
    while(node != currentNodeList.end()){
        int x = (*node)->x();
        int y = (*node)->y();

        setStoneItemPosition(*stone, x, y);
        setTextItemPosition(*iter_number, x, y);

        QList<QGraphicsItem*>::iterator bstone = iter_bstones->begin();
        foreach(const Go::Stone& stone, (*node)->blackStones){
            int x = stone.position.x;
            int y = stone.position.y;
            setStoneItemPosition(*bstone, x, y);
            ++bstone;
        }

        QList<QGraphicsItem*>::iterator wstone = iter_wstones->begin();
        foreach(const Go::Stone& stone, (*node)->whiteStones){
            int x = stone.position.x;
            int y = stone.position.y;
            setStoneItemPosition(*wstone, x, y);
            ++wstone;
        }

        QList<QGraphicsItem*>::iterator mark = iter_marks->begin();
        foreach(const Go::Mark& m, (*node)->marks){
            setMarkItemPosition(*mark, m);
            ++mark;
        }

        QList<QGraphicsItem*>::iterator territory = iter_territories->begin();
        foreach(const Go::Mark& m, (*node)->whiteTerritories){
            setMarkItemPosition(*territory, m);
            ++territory;
        }
        foreach(const Go::Mark& m, (*node)->blackTerritories){
            setMarkItemPosition(*territory, m);
            ++territory;
        }

        QList<QGraphicsRectItem*>::iterator dim = iter_dims->begin();
        foreach(const Go::Mark& m, (*node)->dims){
            setMarkItemPosition(*dim, m);
            ++dim;
        }

        QList<GraphicsArrowItem*>::iterator line = iter_lines->begin();
        foreach(const Go::Line& l, (*node)->lines){
            setLineItemPosition(*line, l);
            ++line;
        }

        ++node;
        ++stone;
        ++iter_number;
        ++iter_bstones;
        ++iter_wstones;
        ++iter_marks;
        ++iter_territories;
        ++iter_dims;
        ++iter_lines;
    }
}

/**
  set grahics stone item position
*/
void BoardWidget::setStoneItemPosition(QGraphicsItem* item, int x, int y){
    qreal size = (vLines[1]->line().x1() - vLines[0]->line().x1()) * 0.95;
    QGraphicsEllipseItem* ellipse = dynamic_cast<QGraphicsEllipseItem*>(item);
    if (ellipse)
        ellipse->setRect(vLines[x]->line().x1()-size/2, hLines[y]->line().y1()-size/2, size, size);
}

/**
  set grahics mark item position
*/
void BoardWidget::setMarkItemPosition(QGraphicsItem* item, const Go::Mark& mark){
    QGraphicsPathItem* pathItem = dynamic_cast<QGraphicsPathItem*>(item);
    QGraphicsSimpleTextItem* textItem = dynamic_cast<QGraphicsSimpleTextItem*>(item);
    if (textItem){
        setTextItemPosition(textItem, mark.position.x, mark.position.y);
//        QRectF r = textItem->boundingRect();
//        text->setPos(vLines[x]->line().x1()-r.width()*0.5, hLines[y]->line().y1()-r.height()*0.5);
    }
    else if (mark.type == Go::Mark::dim){
        qreal size = hLines[1]->line().y1() - hLines[0]->line().y1();
        qreal x = vLines[mark.position.x]->line().x1();
        qreal y = hLines[mark.position.y]->line().y1();

        QGraphicsRectItem* rect = dynamic_cast<QGraphicsRectItem*>(item);
        if (rect == NULL)
            return;
        rect->setRect(x-size/2, y-size/2, size, size);
    }
    else if (pathItem){
        QPainterPath path;
        if (mark.type == Go::Mark::cross)
            path = createCrossPath(mark);
        else if (mark.type == Go::Mark::circle)
            path = createCirclePath(mark);
        else if (mark.type == Go::Mark::square)
            path = createSquarePath(mark);
        else if (mark.type == Go::Mark::triangle)
            path = createTrianglePath(mark);
        else if (mark.type == Go::Mark::blackTerritory)
            path = createTerritoryPath(mark);
        else if (mark.type == Go::Mark::whiteTerritory)
            path = createTerritoryPath(mark);
        else if (mark.type == Go::Mark::select)
            path = createSelectPath(mark);
        else
            return;
        pathItem->setPath(path);
    }
}

/**
  set grahics line item position
*/
void BoardWidget::setLineItemPosition(GraphicsArrowItem* item, const Go::Line& line){
    qreal x1 = vLines[line.position1.x]->line().x1();
    qreal x2 = vLines[line.position2.x]->line().x2();
    qreal y1 = hLines[line.position1.y]->line().y1();
    qreal y2 = hLines[line.position2.y]->line().y2();

    item->setLine(x1, y1, x2, y2);
}

/**
  set graphics text item position
*/
void BoardWidget::setTextItemPosition(QGraphicsSimpleTextItem* text, int x, int y){
    qreal size = (vLines[1]->line().x1() - vLines[0]->line().x1()) * 0.95;
    if (text){
        qreal number_size;
        if (text->text().size() > 2)
            number_size = size * 0.4;
        else if (text->text().size() > 1)
            number_size = size * 0.45;
        else
            number_size = size * 0.5;

        text->setFont( QFont("Helvetica", number_size) );
        QRectF r = text->boundingRect();
        text->setPos(vLines[x]->line().x1()-r.width()*0.5, hLines[y]->line().y1()-r.height()*0.5);
    }
}

/**
  create buffer
*/
void BoardWidget::createBuffer(bool erase){
    if (erase)
        eraseBuffer();

    // create boardBuffer
    boardBuffer.clear();
    boardBuffer.resize(gameInformation->ysize);
    for (int i=0; i<boardBuffer.size(); ++i)
        boardBuffer[i].resize(gameInformation->xsize);

    // show or hide stones
    moveNumber = 0;
    capturedBlack = 0;
    capturedWhite = 0;
    showNumbers.clear();
    Go::NodeList::iterator node = currentNodeList.begin();
    QList<QGraphicsItem*>::iterator iter_stone = stones.begin();
    QList< QList<QGraphicsItem*> >::iterator  iter_bstones = blackStones.begin();
    QList< QList<QGraphicsItem*> >::iterator  iter_wstones = whiteStones.begin();
    QList< QList<QGraphicsItem*> >::iterator  iter_marks = marks.begin();
    QList< QList<QGraphicsItem*> >::iterator  iter_territories = territories.begin();
    QList< QList<QGraphicsRectItem*> >::iterator iter_dims = dims.begin();
    QList< QList<GraphicsArrowItem*> >::iterator iter_lines = lines.begin();
    QList<QGraphicsSimpleTextItem*>::iterator iter_number = numbers.begin();
    while(node != currentNodeList.end()){
        if ((*node)->isStone())
            ++moveNumber;

        if ( inBoard(*node) ){
            // reset move number
            bool resetMoveNumber = (*node)->moveNumber > 0;
            if (resetMoveNumberInBranch){
                Go::NodePtr parent = (*node)->parent();
                if (parent && parent->childNodes.size() > 1)
                    resetMoveNumber = true;
            }
            if (resetMoveNumber){
                showNumbers.clear();
                moveNumber = (*node)->moveNumber ? (*node)->moveNumber : 1;
                QList<QGraphicsSimpleTextItem*>::iterator number = iter_number;
                while(true){
                    if (number == numbers.begin())
                        break;
                    if (*--number)
                        (*number)->hide();
                }
            }

            // add stone to buffer if stone is in board
            addStoneToBuffer((*node)->position.x, (*node)->position.y, (*node)->color, moveNumber, *iter_stone, *iter_number);
            killStones((*node)->position.x, (*node)->position.y);
        }

        // add black stones
        QList<QGraphicsItem*>::iterator bstone = iter_bstones->begin();
        foreach(const Go::Stone& stone, (*node)->blackStones){
            addStoneToBuffer(stone.position.x, stone.position.y, stone.color, 0, *bstone, NULL);
            ++bstone;
        }

        // add white stones
        QList<QGraphicsItem*>::iterator wstone = iter_wstones->begin();
        foreach(const Go::Stone& stone, (*node)->whiteStones){
            addStoneToBuffer(stone.position.x, stone.position.y, stone.color, 0, *wstone, NULL);
            ++wstone;
        }

        // add empty stones
        foreach(const Go::Stone& stone, (*node)->emptyStones){
            TerritoryInfo& ti = boardBuffer[stone.position.y][stone.position.x];
            ti.color = Go::empty;
            if (ti.stone){
                ti.stone->hide();
                ti.stone = NULL;
            }
            if (ti.number){
                ti.number->hide();
                ti.number = NULL;
            }
        }

        // marks
        QList<QGraphicsItem*>::iterator mark = iter_marks->begin();
        foreach(const Go::Mark& m, (*node)->marks){
            if (*node == currentNode)
                addMarkToBuffer(m, *mark);
            else
                removeMarkFromBuffer(m, *mark);
            ++mark;
        }

        // territories
        QList<QGraphicsItem*>::iterator territory = iter_territories->begin();
        foreach(const Go::Mark& m, (*node)->whiteTerritories){
            if (*node == currentNode)
                addMarkToBuffer(m, *territory);
            else
                removeMarkFromBuffer(m, *territory);
            ++territory;
        }
        foreach(const Go::Mark& m, (*node)->blackTerritories){
            if (*node == currentNode)
                addMarkToBuffer(m, *territory);
            else
                removeMarkFromBuffer(m, *territory);
            ++territory;
        }

        // dims
        QList<QGraphicsRectItem*>::iterator dim = iter_dims->begin();
        foreach(const Go::Mark& m, (*node)->dims){
            addMarkToBuffer(m, *dim);
            ++dim;
        }

        // lines
        QList<GraphicsArrowItem*>::iterator line = iter_lines->begin();
        foreach(const Go::Line& l, (*node)->lines){
            if (*node == currentNode)
                addLineToBuffer(l, *line);
            else
                removeLineFromBuffer(l, *line);
            ++line;
        }

        ++iter_stone;
        ++iter_number;
        ++iter_bstones;
        ++iter_wstones;
        ++iter_marks;
        ++iter_territories;
        ++iter_dims;
        ++iter_lines;
        if (*node == currentNode){
            ++node;
            break;
        }
        ++node;
    };

    int moveNumber2 = moveNumber + 1;
    while(node != currentNodeList.end()){
        if ((*node)->moveNumber > 0)
            moveNumber2 = (*node)->moveNumber;

        if (*iter_stone)
            (*iter_stone)->hide();

        if (*iter_number){
            (*iter_number)->hide();
            (*iter_number)->setText( QString::number(moveNumber2++) );
        }

        foreach(QGraphicsItem* item, *iter_wstones)
            item->hide();

        foreach(QGraphicsItem* item, *iter_bstones)
            item->hide();

        foreach(QGraphicsItem* item, *iter_marks)
            item->hide();

        foreach(QGraphicsItem* item, *iter_territories)
            item->hide();

        foreach(QGraphicsRectItem* item, *iter_dims)
            item->hide();

        foreach(GraphicsArrowItem* item, *iter_lines)
            item->hide();

        ++iter_stone;
        ++iter_number;
        ++iter_bstones;
        ++iter_wstones;
        ++iter_marks;
        ++iter_territories;
        ++iter_dims;
        ++iter_lines;
        ++node;
    }

    if (erase)
        setItemsPosition();
}

 /**
  erase buffer
*/
void BoardWidget::eraseBuffer(){
    // clear buffer
    currentNodeList.clear();

    // erase stone items
    qDeleteAll(stones);
    stones.clear();

    // erase text items
    qDeleteAll(numbers);
    numbers.clear();

    // black stones
    for (int i=0; i<blackStones.size(); ++i)
        qDeleteAll(blackStones[i]);
    blackStones.clear();

    // white stones
    for (int i=0; i<whiteStones.size(); ++i)
        qDeleteAll(whiteStones[i]);
    whiteStones.clear();

    // marks
    for (int i=0; i<marks.size(); ++i)
        qDeleteAll(marks[i]);
    marks.clear();

    // territories
    for (int i=0; i<territories.size(); ++i)
        qDeleteAll(territories[i]);
    territories.clear();

    // dims
    for (int i=0; i<dims.size(); ++i)
        qDeleteAll(dims[i]);
    dims.clear();

    // lines
    for (int i=0; i<lines.size(); ++i)
        qDeleteAll(lines[i]);
    lines.clear();

    // create currentNodeList
    Go::NodePtr n = currentNode;
    while(n){
        currentNodeList.push_front(n);
        n = n->parent();
    }

    n = currentNode;
    while(n->childNodes.empty() == false){
        n = n->childNodes.front();
        currentNodeList.push_back(n);
    }

    // create graphics items
    foreach(const Go::NodePtr& node, currentNodeList){
        blackStones.push_back(QList<QGraphicsItem*>());
        whiteStones.push_back(QList<QGraphicsItem*>());
        marks.push_back(QList<QGraphicsItem*>());
        territories.push_back(QList<QGraphicsItem*>());
        dims.push_back(QList<QGraphicsRectItem*>());
        lines.push_back(QList<GraphicsArrowItem*>());

        if ( inBoard(node) ){
            QGraphicsItem* stone = createStoneItem(node->x(), node->y(), node->color);
            stones.push_back(stone);

            QGraphicsSimpleTextItem* number = scene->addSimpleText("");
            number->setBrush( QBrush(node->color == Go::black ? Qt::white : Qt::black) );
            number->setZValue(4);
            numbers.push_back(number);
        }
        else{
            stones.push_back(NULL);
            numbers.push_back(NULL);
        }

        foreach(const Go::Stone& stone, node->blackStones){
            QGraphicsItem* item = createStoneItem(stone.position.x, stone.position.y, stone.color);
            blackStones.last().push_back(item);
        }

        foreach(const Go::Stone& stone, node->whiteStones){
            QGraphicsItem* item = createStoneItem(stone.position.x, stone.position.y, stone.color);
            whiteStones.last().push_back(item);
        }

        foreach(const Go::Mark& mark, node->marks){
            QGraphicsItem* item = createMarkItem(mark);
            if (item)
                marks.last().push_back(item);
        }

        foreach(const Go::Mark& mark, node->whiteTerritories){
            QGraphicsItem* item = createMarkItem(mark);
            if (item)
                territories.last().push_back(item);
        }

        foreach(const Go::Mark& mark, node->blackTerritories){
            QGraphicsItem* item = createMarkItem(mark);
            if (item)
                territories.last().push_back(item);
        }

        foreach(const Go::Mark& mark, node->dims){
            QGraphicsRectItem* item = dynamic_cast<QGraphicsRectItem*>(createMarkItem(mark));
            if (item)
                dims.last().push_back(item);
        }

        foreach(const Go::Line& line, node->lines){
            GraphicsArrowItem* item = createLineItem(line);
            if (item)
                lines.last().push_back(item);
        }
    }
}

/**
  create TerritoryInfo and set to boardBuffer
*/
BoardWidget::TerritoryInfo& BoardWidget::addStoneToBuffer(int x, int y, Go::Color color, int moveNumber, QGraphicsItem* stone, QGraphicsSimpleTextItem* number){
    TerritoryInfo ti;
    ti.color = color;
    ti.moveNumber = moveNumber;
    ti.stone = stone;
    ti.number = number;
    boardBuffer[y][x] = ti;

    stone->show();
    stone->setOpacity(1.0);

    if (number){
        if (showMoveNumber){
            number->show();
            showNumbers.push_back(number);
            if (showMoveNumberCount >= 0 && showNumbers.size() > showMoveNumberCount){
                showNumbers.front()->hide();
                showNumbers.pop_front();
            }
        }
        else
            number->hide();
        number->setText( QString::number(moveNumber) );
    }

    return boardBuffer[y][x];
}

/**
  set graphics mark item to boardBuffer
*/
BoardWidget::TerritoryInfo& BoardWidget::addMarkToBuffer(const Go::Mark& mark, QGraphicsItem* item){
    TerritoryInfo& ti = boardBuffer[mark.position.y][mark.position.x];

    if (mark.type == Go::Mark::dim){
        ti.dim = item;
        item->show();
        return ti;
    }

    GraphicsLabelTextItem* label = dynamic_cast<GraphicsLabelTextItem*>(item);
    QAbstractGraphicsShapeItem* path = dynamic_cast<QAbstractGraphicsShapeItem*>(item);
    if (label){
        label->setBrush( QBrush(ti.color == Go::black ? Qt::white : Qt::black) );

        if (ti.color == Go::empty){
            label->setBackgroundBrush(board->brush());
        }
    }
    else if (path){
        if (mark.type == Go::Mark::blackTerritory || mark.type == Go::Mark::whiteTerritory){
            QColor color = mark.type == Go::Mark::blackTerritory ? Qt::black : Qt::white;
            path->setPen( QPen(color));
            path->setBrush(QBrush(color));
            if (ti.stone)
                ti.stone->setOpacity(0.3);
        }
        else if (mark.type == Go::Mark::select){
            path->setPen(QPen(ti.color == Go::black ? Qt::white : Qt::black));
            path->setBrush(QBrush(ti.color == Go::black ? Qt::white : Qt::black));
        }
        else{
            path->setPen(QPen(ti.color == Go::black ? Qt::white : Qt::black, 2));
        }
    }

    ti.mark = item;
    item->show();
    return ti;
}

/**
  remove graphics mark item from boardBuffer
*/
BoardWidget::TerritoryInfo& BoardWidget::removeMarkFromBuffer(const Go::Mark& mark, QGraphicsItem* item){
    TerritoryInfo& ti = boardBuffer[mark.position.y][mark.position.x];

    ti.mark = NULL;
    item->hide();

    return ti;
}

/**
  set graphics line item to boardBuffer
*/
void BoardWidget::addLineToBuffer(const Go::Line& /*line*/, GraphicsArrowItem* item){
    item->setBrush( QBrush(item->pen().color()) );
    item->show();
}

/**
  remove graphics line item from boardBuffer
*/
void BoardWidget::removeLineFromBuffer(const Go::Line& /*line*/, GraphicsArrowItem* item){
    item->hide();
}

/**
  create graphic stone item
*/
QGraphicsItem* BoardWidget::createStoneItem(int x, int y, Go::Color color){
    int size = vLines[1]->line().x1() - vLines[0]->line().x1();
    int sceneX = vLines[x]->line().x1() - size / 2;
    int sceneY = hLines[y]->line().y1() - size / 2;

    QGraphicsItem* stone = scene->addEllipse( sceneX, sceneY, size, size, QPen(Qt::black), QBrush(color == Go::black ? Qt::black : Qt::white) );
    stone->setZValue(3);

    return stone;
}

/**
  create graphic stone item
*/
QGraphicsItem* BoardWidget::createMarkItem(const Go::Mark& mark){
    QGraphicsItem* item = NULL;
    if (mark.type == Go::Mark::cross)
        item = scene->addPath( createCrossPath(mark) );
    else if (mark.type == Go::Mark::circle)
        item = scene->addPath( createCirclePath(mark) );
    else if (mark.type == Go::Mark::square)
        item = scene->addPath( createSquarePath(mark) );
    else if (mark.type == Go::Mark::triangle)
        item = scene->addPath( createTrianglePath(mark) );
    else if (mark.type == Go::Mark::character){
//        item = scene->addSimpleText(mark.text);
        item = new GraphicsLabelTextItem(mark.text);
        scene->addItem(item);
    }
    else if (mark.type == Go::Mark::blackTerritory)
        item = scene->addPath( createTerritoryPath(mark) );
    else if (mark.type == Go::Mark::whiteTerritory)
        item = scene->addPath( createTerritoryPath(mark) );
    else if (mark.type == Go::Mark::dim){
        item = scene->addRect(0, 0, 0, 0, QPen(Qt::transparent), QBrush(Qt::black));
        item->setOpacity(0.65);
    }
    else if (mark.type == Go::Mark::select)
        item = scene->addPath( createSelectPath(mark) );
    else
        return NULL;

    item->setZValue(4);
    return item;
}

/**
  create cross path
*/
QPainterPath BoardWidget::createCrossPath(const Go::Mark& mark){
    qreal size = vLines[1]->line().x1() - vLines[0]->line().x1();
    size *= 0.4;
    int sceneX = vLines[mark.position.x]->line().x1() - size / 2;
    int sceneY = hLines[mark.position.y]->line().y1() - size / 2;

    QPainterPath path;
    path.moveTo(sceneX, sceneY);
    path.lineTo(sceneX+size, sceneY+size);
    path.moveTo(sceneX+size, sceneY);
    path.lineTo(sceneX, sceneY+size);

    return path;
}

/**
  create circle path
*/
QPainterPath BoardWidget::createCirclePath(const Go::Mark& mark){
    qreal size = vLines[1]->line().x1() - vLines[0]->line().x1();
    size *= 0.5;
    int sceneX = vLines[mark.position.x]->line().x1() - size / 2;
    int sceneY = hLines[mark.position.y]->line().y1() - size / 2;

    QPainterPath path;
    path.addEllipse(sceneX, sceneY, size, size);

    return path;
}

/**
  create square path
*/
QPainterPath BoardWidget::createSquarePath(const Go::Mark& mark){
    qreal size = vLines[1]->line().x1() - vLines[0]->line().x1();
    size *= 0.45;
    int sceneX = vLines[mark.position.x]->line().x1() - size / 2;
    int sceneY = hLines[mark.position.y]->line().y1() - size / 2;

    QPainterPath path;
    path.addRect(sceneX, sceneY, size, size);

    return path;
}

/**
  create triangle path
*/
QPainterPath BoardWidget::createTrianglePath(const Go::Mark& mark){
    qreal size = vLines[1]->line().x1() - vLines[0]->line().x1();
    qreal xsize = size * 0.5;
    qreal ysize = size * 0.4;
    qreal sceneX = vLines[mark.position.x]->line().x1() - xsize / 2;
    qreal sceneY = hLines[mark.position.y]->line().y1() - ysize / 2;

    QPainterPath path;
    path.moveTo(sceneX+xsize/2, sceneY);
    path.lineTo(sceneX, sceneY+ysize);
    path.lineTo(sceneX+xsize, sceneY+ysize);
    path.lineTo(sceneX+xsize/2, sceneY);

    return path;
}

/**
  create black territory path
*/
QPainterPath BoardWidget::createTerritoryPath(const Go::Mark& mark){
    qreal size = vLines[1]->line().x1() - vLines[0]->line().x1();
    size *= 0.35;
    int sceneX = vLines[mark.position.x]->line().x1() - size / 2;
    int sceneY = hLines[mark.position.y]->line().y1() - size / 2;

    QPainterPath path;
    path.addRect(sceneX, sceneY, size, size);

    return path;
}

/**
  create select path
*/
QPainterPath BoardWidget::createSelectPath(const Go::Mark& mark){
    qreal size = vLines[1]->line().x1() - vLines[0]->line().x1();
    size *= 0.45;
    int sceneX = vLines[mark.position.x]->line().x1() - size / 2;
    int sceneY = hLines[mark.position.y]->line().y1() - size / 2;

    QPainterPath path;
    path.addRect(sceneX, sceneY, size, size);

    return path;
}

/**
  create line or arrow item
*/
GraphicsArrowItem* BoardWidget::createLineItem(const Go::Line& line){
    qreal x1 = vLines[line.position1.x]->line().x1();
    qreal x2 = vLines[line.position2.x]->line().x2();
    qreal y1 = hLines[line.position1.y]->line().y1();
    qreal y2 = hLines[line.position2.y]->line().y2();

    GraphicsArrowItem* item;
    GraphicsArrowItem::Shape shape = line.type == Go::Line::arrow ? GraphicsArrowItem::normal : GraphicsArrowItem::none;
    item = new GraphicsArrowItem(x1, y1, x2, y2, GraphicsArrowItem::none, shape);

    item->setPen( QPen(Qt::blue, 3) );
    item->setZValue(4);
    scene->addItem(item);

    return item;
}

/**
  alternate move
*/
void BoardWidget::alternateMove(int x, int y){
    if (boardBuffer[y][x].isStone())
        return;

    if (moveToChildItem(x, y) == false){
        Go::Color nextColor = getNextColor();
        boardBuffer[y][x].color = nextColor;
        if (canKillStones(x, y) == true || isDeadStones(x, y) == false)
            createChildItem(x, y);
        else
            boardBuffer[y][x].color = Go::empty;
    }
}

/**
  add stone
*/
void BoardWidget::addStone(int x, int y, Go::Color color){
    TerritoryInfo& ti = boardBuffer[y][x];

    Go::Point p(x, y);

    if (ti.stone && ti.number == NULL){
        bool removed = removeStone(currentNode->blackStones, p) ||
                       removeStone(currentNode->whiteStones, p) ||
                       removeStone(currentNode->emptyStones, p);
        if (removed)
            return;
    }

    if ((color == Go::empty && ti.stone) || (color != Go::empty && ti.stone == NULL)){
        // if currentNode is stone, create child node and add stone to new node.
        if (currentNode->isStone()){
            Go::NodePtr node(new Go::Node(currentNode));
            addItem(currentNode, node, -1);
            setCurrentNode(node);
        }

        AddStoneCommand* command = new AddStoneCommand(document(), currentNode, Go::Stone(p, color));
        document()->getUndoStack()->push(command);
    }
}

/**
  remove stone
*/
bool BoardWidget::removeStone(const Go::StoneList& stoneList, const Go::Point& p){
    foreach(const Go::Stone& s, stoneList){
        if (s.position == p){
            RemoveStoneCommand* command = new RemoveStoneCommand(document(), currentNode, p);
            document()->getUndoStack()->push(command);
            return true;
        }
    }
    return false;
}

/**
  add label
*/
void BoardWidget::addLabel(int x, int y, bool autoLabel){
    TerritoryInfo& ti = boardBuffer[y][x];
    if(ti.mark){
        RemoveMarkCommand* command = new RemoveMarkCommand(document(), currentNode, Go::Point(x, y));
        document()->getUndoStack()->push(command);
        return;
    }

    QString str;
    if (autoLabel == false){
        str = QInputDialog::getText(this, QString(), tr("Input label"));
        if (str.isEmpty())
            return;
    }
    else{
        char c = 'A';

        QStringList strList;
        foreach(const Go::Mark& mark, currentNode->marks){
            if (mark.type == Go::Mark::character)
                strList.push_back(mark.text);
        }

        str.sprintf("%c", c);
        while (strList.indexOf(str) >= 0)
            str.sprintf("%c", ++c);
    }
    AddMarkCommand* command = new AddMarkCommand(document(), currentNode, Go::Mark(x, y, str));
    document()->getUndoStack()->push(command);
}

/**
  add mark(circle, cross, square, triangle)
*/
void BoardWidget::addMarker(int x, int y, Go::Mark::Type mark){
    TerritoryInfo& ti = boardBuffer[y][x];
    if(ti.mark){
        RemoveMarkCommand* command = new RemoveMarkCommand(document(), currentNode, Go::Point(x, y));
        document()->getUndoStack()->push(command);
    }
    else{
        AddMarkCommand* command = new AddMarkCommand(document(), currentNode, Go::Mark(x, y, mark));
        document()->getUndoStack()->push(command);
    }
}

/**
  remove marker
*/
void BoardWidget::removeMarker(int x, int y){
    Go::Point p(x, y);
    TerritoryInfo& ti = boardBuffer[y][x];
    if(ti.mark){
        RemoveMarkCommand* command = new RemoveMarkCommand(document(), currentNode, p);
        document()->getUndoStack()->push(command);
    }
    else{
        removeStone(currentNode->blackStones, p) ||
            removeStone(currentNode->whiteStones, p) ||
            removeStone(currentNode->emptyStones, p);
    }
}

/**
  Move next node
*/
void BoardWidget::forward(int step){
    Go::NodeList::const_iterator iter = qFind(currentNodeList, currentNode);
    if (iter == currentNodeList.end())
        return;

    Go::NodeList::const_iterator pos = iter;
    for (int i=0; i<step; ++i){
        if (++iter == currentNodeList.end())
            break;
        pos = iter;
    }
    setCurrentNode(*pos);
}

/**
  Move previous node
*/
void BoardWidget::back(int step){
    Go::NodeList::const_iterator iter = qFind(currentNodeList, currentNode);
    if (iter == currentNodeList.end())
        return;

    for (int i=0; i<step; ++i){
        if (iter == currentNodeList.begin())
            break;
        --iter;
    }
    setCurrentNode(*iter);
}

/**
  set edit mode
*/
void BoardWidget::setEditMode(EditMode::Mode editMode_){
    editMode = editMode_;
}

/**
  set show move number
*/
void BoardWidget::setShowMoveNumber(bool show){
    showMoveNumber = show;
    createBuffer(false);
}

/**
  set reset move number in branch
*/
void BoardWidget::setResetMoveNumberInBranch(bool reset){
    resetMoveNumberInBranch = reset;
    createBuffer(false);
}

/**
  set show move number count
*/
void BoardWidget::setShowMoveNumberCount(int cnt){
    showMoveNumberCount = cnt;
    createBuffer(false);
}

/**
  add item
*/
void BoardWidget::addItem(Go::NodePtr parent, Go::NodePtr node, int index){
    document()->getUndoStack()->push( new AddNodeCommand(document(), parent, node, index) );
}

/**
  Get Star Position
*/
void BoardWidget::getStarPosition(QList<int>& xpos, QList<int>& ypos){
    int xsize = gameInformation->xsize;
    int ysize = gameInformation->ysize;

    if (xsize > 6){
        xpos.push_back(xsize > 9 ? 3 : 2);
        xpos.push_back(xsize > 9 ? xsize-4 : xsize-3);
        if (xsize % 2 != 0 && xsize >= 9)
            xpos.push_back(xsize / 2);
    }

    if (ysize > 6){
        ypos.push_back(ysize > 9 ? 3 : 2);
        ypos.push_back(ysize > 9 ? ysize-4 : ysize-3);
        if (ysize % 2 != 0 && ysize >= 9)
            ypos.push_back(ysize / 2);
    }
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
    if (isDeadStones(x-1, y, color == Go::black ? Go::white : Go::black, buf) == true)
        killStones(buf);

    memset(buf, 0, ysize*xsize);
    if (isDeadStones(x+1, y, color == Go::black ? Go::white : Go::black, buf) == true)
        killStones(buf);

    memset(buf, 0, ysize*xsize);
    if (isDeadStones(x, y-1, color == Go::black ? Go::white : Go::black, buf) == true)
        killStones(buf);

    memset(buf, 0, ysize*xsize);
    if (isDeadStones(x, y+1, color == Go::black ? Go::white : Go::black, buf) == true)
        killStones(buf);

    // if suicide move, kill myself
    memset(buf, 0, ysize*xsize);
    if (isDeadStones(x, y, color, buf) == true)
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
                if (boardBuffer[y][x].color == Go::black)
                    ++capturedBlack;
                else if (boardBuffer[y][x].color == Go::white)
                    ++capturedWhite;

                boardBuffer[y][x].color = Go::empty;
                boardBuffer[y][x].stone->hide();
                boardBuffer[y][x].stone = NULL;
                boardBuffer[y][x].number->hide();
                boardBuffer[y][x].number = NULL;
            }
        }
    }
}

/**
*/
bool BoardWidget::canKillStones(int x, int y){
    Go::Color color = boardBuffer[y][x].color;
    if (color == Go::empty)
        return false;
    color = color == Go::black ? Go::white : Go::black;

    int xsize = gameInformation->xsize;
    int ysize = gameInformation->ysize;
    QVector<char> buf(ysize * xsize);

    if (inBoard(x-1, y) && isDeadStones(x-1, y, color, buf.data()) == true)
        return true;

    buf.fill(0);
    if (inBoard(x+1, y) && isDeadStones(x+1, y, color, buf.data()) == true)
        return true;

    buf.fill(0);
    if (inBoard(x, y-1) && isDeadStones(x, y-1, color, buf.data()) == true)
        return true;

    buf.fill(0);
    if (inBoard(x, y+1) && isDeadStones(x, y+1, color, buf.data()) == true)
        return true;

    return false;
}

/**
*/
bool BoardWidget::isDeadStones(int x, int y){
    Go::Color color = boardBuffer[y][x].color;
    if (color == Go::empty)
        return false;

    int xsize = gameInformation->xsize;
    int ysize = gameInformation->ysize;
    QVector<char> buf(ysize * xsize);

    if (isDeadStones(x, y, color, buf.data()) == false)
        return false;

    return true;
}

/**
*/
bool BoardWidget::isDeadStones(int x, int y, Go::Color color, char* buf){
    if (x < 0 || x >= gameInformation->xsize || y < 0 || y >= gameInformation->ysize)
        return true;
    else if (boardBuffer[y][x].color == Go::empty)
        return false;
    else if (boardBuffer[y][x].color != color)
        return true;

    if (buf[y * gameInformation->xsize + x])
        return true;

    buf[y * gameInformation->xsize + x] = 1;

    if (isDeadStones(x-1, y, color, buf) == false)
        return false;

    if (isDeadStones(x+1, y, color, buf) == false)
        return false;

    if (isDeadStones(x, y-1, color, buf) == false)
        return false;

    if (isDeadStones(x, y+1, color, buf) == false)
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
*/
bool BoardWidget::inBoard(int x, int y){
    if (x < 0 || y < 0 || x >= boardBuffer[0].size() || y >= boardBuffer.size())
        return false;

    return true;
}

/**
*/
bool BoardWidget::moveToChildItem(int x, int y){
    foreach (const Go::NodePtr& node, currentNode->childNodes){
        if (node->x() == x && node->y() == y){
            setCurrentNode(node);
            return true;
        }
    }

    return false;
}

/**
*/
bool BoardWidget::createChildItem(int x, int y){
    // create node
    Go::Color nextColor = getNextColor();
    Go::NodePtr node;
    if (nextColor == Go::black)
        node = Go::createBlackNode(currentNode, x, y);
    else if (nextColor == Go::white)
        node = Go::createWhiteNode(currentNode, x, y);
    else
        return false;

    addItem(currentNode, node, -1);
    setCurrentNode(node);

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

/**
  Slot
  node modified
*/
void BoardWidget::on_sgfdocument_nodeModified(Go::NodePtr /*node*/, bool needRecreateBoard){
    if (needRecreateBoard == false)
        return;
    createBuffer(true);
}

/**
  Slot
  node modified
*/
void BoardWidget::on_sgfdocument_gameModified(Go::NodePtr game){
    if (game != currentGame)
        return;

    Go::NodePtr node = currentNode;
    setCurrentGame(game, true);
    setCurrentNode(node);
}