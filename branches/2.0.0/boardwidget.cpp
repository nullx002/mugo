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
#include "boardwidget.h"
#include "sgfdocument.h"
#include "command.h"

class GraphicsLabelTextItem : public QGraphicsSimpleTextItem{
    public:
        GraphicsLabelTextItem(QGraphicsItem* parent = 0);
        GraphicsLabelTextItem(const QString& text, QGraphicsItem* parent = 0);

        void setBackgroundBrush(const QBrush& b){ backgroundBrush_ = b; update(); }
        QBrush backgroundBrush(QBrush& b) const{ return backgroundBrush_; }

    protected:
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

        QBrush backgroundBrush_;
};

GraphicsLabelTextItem::GraphicsLabelTextItem(QGraphicsItem* parent)
    : QGraphicsSimpleTextItem(parent)
{
}

GraphicsLabelTextItem::GraphicsLabelTextItem(const QString& text, QGraphicsItem* parent)
    : QGraphicsSimpleTextItem(text, parent)
{
}

void GraphicsLabelTextItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
    painter->fillRect(boundingRect(), backgroundBrush_);
    QGraphicsSimpleTextItem::paint(painter, option, widget);
}




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
    else if (boardBuffer[y][x].isStone())
        return;

    if (moveToChildItem(x, y) == false)
        createChildItem(x, y);
}

/**
    RightButton Down
*/
void BoardWidget::onRButtonDown(QMouseEvent* /*e*/){
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
    foreach(QGraphicsLineItem* line, hLines)
        scene->removeItem(line);
    hLines.clear();
    foreach(QGraphicsLineItem* line, vLines)
        scene->removeItem(line);
    vLines.clear();
    foreach(QGraphicsEllipseItem* star, stars)
        scene->removeItem(star);
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
    int width  = r.width() - 30;
    int height = r.height() - 30;
    int x = width  / gameInformation->xsize;
    int y = height / gameInformation->ysize;
    int size = qMin(x, y);
    int w = size * (gameInformation->xsize - 1);
    int h = size * (gameInformation->ysize - 1);

    x = (r.width() - w) / 2;
    y = (r.height() - h) / 2;
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

    // set position of stones and move number.
    Go::NodeList::iterator node = currentNodeList.begin();
    QList<QGraphicsItem*>::iterator stone = stones.begin();
    QList< QList<QGraphicsItem*> >::iterator bstones = blackStones.begin();
    QList< QList<QGraphicsItem*> >::iterator wstones = whiteStones.begin();
    QList< QList<QGraphicsItem*> >::iterator imarks = marks.begin();
    QList<QGraphicsSimpleTextItem*>::iterator number = numbers.begin();
    while(node != currentNodeList.end()){
        int x = (*node)->x();
        int y = (*node)->y();

        setStoneItemPosition(*stone, x, y);
        setTextItemPosition(*number, x, y);

        QList<QGraphicsItem*>::iterator bstone = bstones->begin();
        foreach(const Go::Stone& stone, (*node)->blackStones){
            int x = stone.position.x;
            int y = stone.position.y;
            setStoneItemPosition(*bstone, x, y);
            ++bstone;
        }

        QList<QGraphicsItem*>::iterator wstone = wstones->begin();
        foreach(const Go::Stone& stone, (*node)->whiteStones){
            int x = stone.position.x;
            int y = stone.position.y;
            setStoneItemPosition(*wstone, x, y);
            ++wstone;
        }

        QList<QGraphicsItem*>::iterator imark = imarks->begin();
        foreach(const Go::Mark& mark, (*node)->marks){
            setMarkItemPosition(*imark, mark);
            ++imark;
        }

        ++node;
        ++stone;
        ++number;
        ++bstones;
        ++wstones;
        ++imarks;
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
    if (pathItem){
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
        else if (mark.type == Go::Mark::dim)
            path = createDimPath(mark);
        else if (mark.type == Go::Mark::select)
            path = createSelectPath(mark);
        else
            return;
        pathItem->setPath(path);
    }
    else if (textItem){
        setTextItemPosition(textItem, mark.position.x, mark.position.y);
//        QRectF r = textItem->boundingRect();
//        text->setPos(vLines[x]->line().x1()-r.width()*0.5, hLines[y]->line().y1()-r.height()*0.5);
    }
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
    moveNumber = 1;
    capturedBlack = 0;
    capturedWhite = 0;
    Go::NodeList::iterator node = currentNodeList.begin();
    QList<QGraphicsItem*>::iterator iter_stone = stones.begin();
    QList< QList<QGraphicsItem*> >::iterator  iter_bstones = blackStones.begin();
    QList< QList<QGraphicsItem*> >::iterator  iter_wstones = whiteStones.begin();
    QList< QList<QGraphicsItem*> >::iterator  iter_marks   = marks.begin();
    QList<QGraphicsSimpleTextItem*>::iterator iter_number  = numbers.begin();
    while(node != currentNodeList.end()){
        if ( inBoard(*node) ){
            // hide move number before set move number
            if ((*node)->moveNumber > 0){
                moveNumber = (*node)->moveNumber;
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

        ++iter_stone;
        ++iter_number;
        ++iter_bstones;
        ++iter_wstones;
        ++iter_marks;
        if (*node == currentNode){
            ++node;
            break;
        }

        if ((*node)->isStone())
            ++moveNumber;
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

        QList<QGraphicsItem*>::iterator bstone = iter_bstones->begin();
        while (bstone != iter_bstones->end()){
            (*bstone)->hide();
            ++bstone;
        }

        QList<QGraphicsItem*>::iterator wstone = iter_wstones->begin();
        while (wstone != iter_wstones->end()){
            (*wstone)->hide();
            ++wstone;
        }

        QList<QGraphicsItem*>::iterator mark = iter_marks->begin();
        while (mark != iter_marks->end()){
            (*mark)->hide();
            ++mark;
        }

        ++iter_stone;
        ++iter_number;
        ++iter_bstones;
        ++iter_wstones;
        ++iter_marks;
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

    foreach(const Go::NodePtr& node, currentNodeList){
        blackStones.push_back(QList<QGraphicsItem*>());
        whiteStones.push_back(QList<QGraphicsItem*>());
        marks.push_back(QList<QGraphicsItem*>());

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

    if (number){
        number->show();
        number->setText( QString::number(moveNumber) );
    }

    return boardBuffer[y][x];
}

/**
  set graphics mark item to boardBuffer
*/
BoardWidget::TerritoryInfo& BoardWidget::addMarkToBuffer(const Go::Mark& mark, QGraphicsItem* item){
    TerritoryInfo& ti = boardBuffer[mark.position.y][mark.position.x];
    GraphicsLabelTextItem* label = dynamic_cast<GraphicsLabelTextItem*>(item);
    QAbstractGraphicsShapeItem* path = dynamic_cast<QAbstractGraphicsShapeItem*>(item);
    if (label){
        label->setBrush( QBrush(ti.color == Go::black ? Qt::white : Qt::black) );

        if (ti.color == Go::empty){
            label->setBackgroundBrush(board->brush());
        }
    }
    else if (path){
        path->setPen(QPen(ti.color == Go::black ? Qt::white : Qt::black, 2));

        if (mark.type == Go::Mark::select)
            path->setBrush(QBrush(ti.color == Go::black ? Qt::white : Qt::black));
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
    else if (mark.type == Go::Mark::dim)
        item = scene->addPath( createDimPath(mark) );
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
    int sceneX = vLines[mark.position.x]->line().x1() - xsize / 2;
    int sceneY = hLines[mark.position.y]->line().y1() - ysize / 2;

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
qDebug() << "createTerritoryPath";
    qreal size = vLines[1]->line().x1() - vLines[0]->line().x1();
    size *= 0.35;
    int sceneX = vLines[mark.position.x]->line().x1() - size / 2;
    int sceneY = hLines[mark.position.y]->line().y1() - size / 2;

    QPainterPath path;
    path.addRect(sceneX, sceneY, size, size);

    return path;
}

/**
  create dim path
*/
QPainterPath BoardWidget::createDimPath(const Go::Mark& mark){
    QPainterPath path;
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
  add item
*/
void BoardWidget::addItem(Go::NodePtr parent, Go::NodePtr node, int index){
    document()->getUndoStack()->push( new AddNodeCommand(document(), parent, node, index) );
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
