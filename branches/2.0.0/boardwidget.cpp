/*
    mugo, sgf editor.
    Copyright (C) 2009-2011 nsase.

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
#include <QMouseEvent>
#include <QGraphicsItemGroup>
#include "mugoapp.h"
#include "boardwidget.h"
#include "sgfcommand.h"


/**
  paint
*/
void GraphicsPathItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
    QRect sourceRect(0, 0, boundingRect().width(), boundingRect().height());
    painter->drawPixmap(boundingRect(), backgroundImage_, sourceRect);
    QGraphicsPathItem::paint(painter, option, widget);
}


/**
  Constructor
*/
BoardWidget::BoardWidget(QWidget* parent)
    : QGraphicsView(parent)
    , document_(NULL)
    , blackStone_(NULL)
    , whiteStone_(NULL)
    , nextColor_(Go::eBlack)
    , editMode_(eAlternateMove)
    , moveNumber_(0)
    , showMoveNumber_(true)
    , showCoordinate_(true)
    , showCoordinateI_(false)
    , showVariation_(true)
{
    initialize();
}

/**
  Constructor
*/
BoardWidget::BoardWidget(GoDocument* doc, QWidget* parent)
    : QGraphicsView(parent)
    , document_(doc)
    , blackStone_(NULL)
    , whiteStone_(NULL)
    , nextColor_(Go::eBlack)
    , editMode_(eAlternateMove)
    , moveNumber_(0)
    , showMoveNumber_(true)
    , showCoordinate_(true)
    , showCoordinateI_(false)
    , showVariation_(true)
{
    initialize();

    // set current game
    setGame(document_->gameList.front());
}

/**
  initialize
*/
void BoardWidget::initialize(){
    setScene( new QGraphicsScene(this) );
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    // create board
    shadow = scene()->addRect(0, 0, 1, 1, QPen(Qt::NoPen), QBrush(SHADOW_COLOR));
    board  = scene()->addRect(0, 0, 1, 1, QPen(Qt::NoPen), QBrush(BOARD_COLOR));
    shadow->setZValue(0);
    board->setZValue(1);

    setMouseTracking(true);
}

/**
  resize event
*/
void BoardWidget::resizeEvent(QResizeEvent* e){
    // set items position
    setItemsPosition(e->size());

    // create black and white stones
    // these point to next stone.
    delete blackStone_;
    blackStone_ = createStoneItem(Go::eBlack, 0, 0);
    blackStone_->hide();
    blackStone_->setOpacity(0.5);
    delete whiteStone_;
    whiteStone_ = createStoneItem(Go::eWhite, 0, 0);
    whiteStone_->hide();
    whiteStone_->setOpacity(0.5);
}

/**
  mouse move event
*/
void BoardWidget::mouseMoveEvent(QMouseEvent* e){
    blackStone_->hide();
    whiteStone_->hide();

    // get sgf position from mouse event
    int sgfX, sgfY;
    if (viewToSgfCoordinate(e->x(), e->y(), sgfX, sgfY) == false)
        return;

    // if event position has stone, transparent stone isn't shown.
    if (data[sgfY][sgfX].color != Go::eDame)
        return;

    // get stone position from sgf coordinate
    qreal x, y;
    if (sgfToViewCoordinate(sgfX, sgfY, x, y) == false)
        return;

    // stone size
    qreal size = getGridSize() * 0.95;

    // transparent stone is shown at mouse position.
    Go::Color color = nextColor_;
    if (editMode_ == eAddBlack)
        color = Go::eBlack;
    else if (editMode_ == eAddWhite)
        color = Go::eWhite;
    else if (editMode_ != eAlternateMove)
        color = Go::eDame;
    QGraphicsEllipseItem* blackEllipse = dynamic_cast<QGraphicsEllipseItem*>(blackStone_);
    QGraphicsEllipseItem* whiteEllipse = dynamic_cast<QGraphicsEllipseItem*>(whiteStone_);
    blackEllipse->setRect(x-size/2.0, y-size/2.0, size, size);
    whiteEllipse->setRect(x-size/2.0, y-size/2.0, size, size);
    blackStone_->setVisible(color == Go::eBlack);
    whiteStone_->setVisible(color == Go::eWhite);
}

/**
  mouse button up event
*/
void BoardWidget::mouseReleaseEvent(QMouseEvent* e){
    if (e->button() == Qt::LeftButton)
        onLButtonUp(e);
    else if (e->button() == Qt::RightButton)
        onRButtonUp(e);
}

/**
  mouse wheel event
*/
void BoardWidget::wheelEvent(QWheelEvent* e){
    if (e->delta() > 0)
        back();
    else if (e->delta() < 0)
        forward();
}

/**
  mouse left button up event
*/
void BoardWidget::onLButtonUp(QMouseEvent* e){
    int sgfX, sgfY;
    if (viewToSgfCoordinate(e->x(), e->y(), sgfX, sgfY) == false)
        return;

    switch(editMode_){
        case eAlternateMove:
            alternateMove(sgfX, sgfY);
            break;
        case eAddBlack:
            addStone(sgfX, sgfY, Go::eBlack);
            break;
        case eAddWhite:
            addStone(sgfX, sgfY, Go::eWhite);
            break;
        case eAddEmpty:
            addStone(sgfX, sgfY, Go::eDame);
            break;
        case eAddLabel:
            break;
        case eAddLabelManually:
            break;
        case eAddCircle:
            break;
        case eAddSquare:
            break;
        case eAddTriangle:
            break;
        case eAddCross:
            break;
        case eRemoveMarker:
            break;
    }
}

/**
  mouse right button up event
*/
void BoardWidget::onRButtonUp(QMouseEvent* e){
}

/**
  set current document
*/
bool BoardWidget::setDocument(GoDocument* doc){
    document_ = doc;
    connect(document_, SIGNAL(nodeAdded(const Go::NodePtr&, const Go::NodePtr&)), SLOT(on_document_nodeAdded(const Go::NodePtr&, const Go::NodePtr&)));
    connect(document_, SIGNAL(nodeDeleted(const Go::NodePtr&, const Go::NodePtr&, bool)), SLOT(on_document_nodeDeleted(const Go::NodePtr&, const Go::NodePtr&, bool)));
    connect(document_, SIGNAL(nodeModified(const Go::NodePtr&, const Go::NodePtr&)), SLOT(on_document_nodeModified(const Go::NodePtr&, const Go::NodePtr&)));
    if (setGame(document_->gameList.front()) == false)
        return false;

    return true;
}

/**
  set current game

  @todo current stones, branch and stars must be deleted before change game
*/
bool BoardWidget::setGame(const Go::NodePtr& game){
    if (currentGame_ == game)
        return false;

    // if game is not found in game list, return false
    Go::NodeList::const_iterator iter = qFind(document_->gameList.begin(), document_->gameList.end(), game);
    if (iter == document_->gameList.end())
        return false;

    // change current game
    currentGame_ = game;
    currentNode_.clear();

    // create vertical lines
    qDeleteAll(vLines);
    vLines.clear();
    for(int i=0; i<xsize(); ++i){
        QGraphicsLineItem* line = scene()->addLine(0, 0, 0, 0, QPen(Qt::black));
        line->setZValue(2);
        vLines.push_back(line);
    }

    // create horizontal lines
    qDeleteAll(hLines);
    hLines.clear();
    for(int i=0; i<ysize(); ++i){
        QGraphicsLineItem* line = scene()->addLine(0, 0, 0, 0, QPen(Qt::black));
        line->setZValue(2);
        hLines.push_back(line);
    }

    // create star
    QList<int> xstarpos, ystarpos;
    getStarPositions(xstarpos, ystarpos);
    for (int i=0; i<xstarpos.size()*ystarpos.size(); ++i){
        QGraphicsEllipseItem* star = scene()->addEllipse(0, 0, 5, 5, QPen(Qt::black), QBrush(Qt::black));
        star->setZValue(2);
        stars.push_back(star);
    }

    // create buffer
    data.resize(ysize());
    buffer.resize(ysize());
    for (int i=0; i<ysize(); ++i){
        data[i].resize(xsize());
        buffer[i].resize(xsize());
    }

    // set items position
    setItemsPosition(geometry().size());

    // emit gameChanged
    emit gameChanged(currentGame_);

    // change current node
    setNode(currentGame_);

    return true;
}

/**
  set game information
*/
bool BoardWidget::setInformation(const Go::InformationPtr& information){
    if (currentInformation_ == information)
        return false;

    // change game information, and emit currentGameInformationChanged
    currentInformation_ = information;
    emit informationChanged(currentInformation_);
    return true;
}

/**
  set current node
*/
bool BoardWidget::setNode(const Go::NodePtr& node, bool forceChange){
    if (forceChange == false && currentNode_ == node)
        return false;

    // if node isn't in the node list, create node list
    Go::NodeList::const_iterator iter = qFind(currentNodeList_.begin(), currentNodeList_.end(), node);
    if (forceChange == false && iter == currentNodeList_.end())
        createNodeList(node);

    // change game information
    if (node->information())
        setInformation(node->information());

    // change current node, and emit currentNodeChanged
    currentNode_ = node;

    // create buffer
    createBoardBuffer();

    // emit nodeChanged
    emit nodeChanged(currentNode_);

    return true;
}

/**
  set scene items position in rect area
*/
void BoardWidget::setItemsPosition(const QSize& size){
    if (document_ == NULL)
        return;

    // set scene rect
    scene()->setSceneRect(QRect(0, 0, size.width(), size.height()));

    // calculate board size
    int width  = size.width();
    int height = size.height();

    int gridW = width  / (xsize() + 1);
    int gridH = height / (ysize() + 1);
    int gridSize = qMin(gridW, gridH);
    int w = gridSize * (xsize() - 1);
    int h = gridSize * (ysize() - 1);

    int x = (width - w) / 2;
    int y = (height - h) / 2;
    int margin = int(gridSize * 0.6);
    QRect boardRect(QPoint(x-margin, y-margin), QPoint(x+w+margin, y+h+margin));

    // set position of shadow.
    shadow->setRect(boardRect.left()+3, boardRect.top()+3, boardRect.width(), boardRect.height());

    // set position of board.
    board->setRect(boardRect);

    // set scene rect
    QRectF r = boardRect;
    r.setRight(shadow->rect().right());
    r.setBottom(shadow->rect().bottom());

    // move items position to current board size
    setVLinesPosition(x, y, gridSize);
    setHLinesPosition(x, y, gridSize);
    setStarsPosition();

    // move all datas position to current board size
    setDataPosition();
}

/**
  set vertical lines position
*/
void BoardWidget::setVLinesPosition(int x, int y, int gridSize){
    int len = gridSize * (ysize() - 1);
    for (int i=0; i<vLines.size(); ++i){
        vLines[i]->setLine(x, y, x, y+len);
        x += gridSize;
    }
}

/**
  set horizontal lines position
*/
void BoardWidget::setHLinesPosition(int x, int y, int gridSize){
    int len = gridSize * (xsize() - 1);
    for (int i=0; i<hLines.size(); ++i){
        hLines[i]->setLine(x, y, x+len, y);
        y += gridSize;
    }
}

/**
  set star position
*/
void BoardWidget::setStarsPosition(){
    QList<int> xpos, ypos;
    getStarPositions(xpos, ypos);
    for (int y=0, i=0; y<ypos.size(); ++y){
        qreal yy = hLines[ypos[y]]->line().y1();
        for (int x=0; x<xpos.size(); ++x, ++i){
            qreal xx = vLines[xpos[x]]->line().x1();
            qreal r = stars[i]->rect().width() / 2;
            stars[i]->setPos(xx-r, yy-r);
        }
    }

}

/**
  set datas position
*/
void BoardWidget::setDataPosition(){
    for (int y=0; y<data.size(); ++y){
        for (int x=0; x<data[y].size(); ++x){
            Data& d = data[y][x];

            if (d.stoneItem)
                d.stoneItem = GraphicsItemPtr( createStoneItem(d.color, x, y) );

            if (d.branchItem)
                d.branchItem.clear();

            if (d.numberItem)
                createMoveNumber(x, y, d.number, d.focus);
        }
    }

    // create graphics items of branch marker
    createBranchMarkers();

    // create graphics items of marker
    createMarkers();
}

/**
  create buffer
*/
void BoardWidget::createBoardBuffer(){
    capturedWhite_ = 0;
    capturedBlack_ = 0;

    for (int i=0; i<buffer.size(); ++i)
        qFill(buffer[i], Go::eDame);

    moveNumber_ = 1;
    Go::NodeList::iterator iter = currentNodeList_.begin();
    while (iter != currentNodeList_.end()){
        Go::NodePtr node(*iter);

        // change move number if move number is designated.
        if (node->moveNumber() > 0)
            moveNumber_ = node->moveNumber();

        // put stone if node is stone and node isn't pass.
        if (node->isStone() && !node->isPass() && node->x() < xsize() && node->y() < ysize()){
            buffer[node->y()][node->x()] = node->color();
            killStone(node->x(), node->y());

            // move number
            data[node->y()][node->x()].number = moveNumber_++;
        }

        // add empty stones
        foreach (const QPoint& p, node->emptyStones())
            buffer[p.y()][p.x()] = Go::eDame;

        // add black stones
        foreach (const QPoint& p, node->blackStones())
            buffer[p.y()][p.x()] = Go::eBlack;

        // add white stones
        foreach (const QPoint& p, node->whiteStones())
            buffer[p.y()][p.x()] = Go::eWhite;

        // next color
        nextColor_ = node->nextColor();

        if (node == currentNode_)
            break;

        ++iter;
    }
    --moveNumber_;

    // create graphics items
    for(int y=0; y<ysize(); ++y){
        for(int x=0; x<xsize(); ++x){
            // delete branch marker
            data[y][x].branchItem.clear();

            // delete marker
            data[y][x].markerItem.clear();

            // clear move number
            if (buffer[y][x] == Go::eDame)
                data[y][x].number = -1;

            // create move number
            bool focus = x == currentNode_->x() && y == currentNode_->y();
            if (showMoveNumber_ && data[y][x].number > 0 && (data[y][x].numberItem == false || data[y][x].focus != focus))
                createMoveNumber(x, y, data[y][x].number, focus);
            else if (data[y][x].number <= 0)
                data[y][x].numberItem.clear();

            // if this position already has stone, continue
            if (buffer[y][x] == data[y][x].color)
                continue;

            // delete current stone
            if (data[y][x].stoneItem)
                data[y][x].stoneItem.clear();
            data[y][x].color = buffer[y][x];

            if (buffer[y][x] != Go::eDame){
                // create new stone item
                data[y][x].stoneItem = GraphicsItemPtr(createStoneItem(buffer[y][x], x, y));
            }
        }
    }

    // create graphics items of branch marker
    createBranchMarkers();

    // create braphics items of marker
    createMarkers();
}

/**
  kill around stone
*/
void BoardWidget::killStone(int x, int y){
    Go::Color color = buffer[y][x] == Go::eBlack ? Go::eWhite : Go::eBlack;
    killStone(x-1, y, color);
    killStone(x+1, y, color);
    killStone(x, y-1, color);
    killStone(x, y+1, color);
}

/**
  kill stone
*/
void BoardWidget::killStone(int x, int y, Go::Color color){
    if (x < 0 || x >= xsize() || y < 0 || y >= ysize())
        return;

    QVector< QVector<bool> > checked(ysize());
    for (int i=0; i<ysize(); ++i)
        checked[i].resize(xsize());

    if (isDeadStone(x, y, color, checked) == false)
        return;

    for (int y=0; y<checked.size(); ++y){
        for (int x=0; x<checked[y].size(); ++x){
            if (checked[y][x] == true && buffer[y][x] == color){
                buffer[y][x] = Go::eDame;
                if (color == Go::eWhite)
                    ++capturedWhite_;
                else if (color == Go::eBlack)
                    ++capturedBlack_;
            }
        }
    }
}

/**
  @retval true  this group is dead
  @retval false this group is alive
*/
bool BoardWidget::isDeadStone(int x, int y){
    if (buffer[y][x] == Go::eDame)
        return false;
    else if (isDeadStone(x, y, buffer[y][x]) == true)
        return true;

    return false;
}

/**
  @retval true  this group is dead
  @retval false this group is alive
*/
bool BoardWidget::isDeadStone(int x, int y, Go::Color color){
    if (x < 0 || x >= xsize() || y < 0 || y >= ysize())
        return false;

    QVector< QVector<bool> > checked(ysize());
    for (int i=0; i<ysize(); ++i)
        checked[i].resize(xsize());

    return isDeadStone(x, y, color, checked);
}

/**
  @retval true  this group is dead
  @retval false this group is alive
*/
bool BoardWidget::isDeadStone(int x, int y, Go::Color color, QVector< QVector<bool> >& checked){
    if (x < 0 || x >= xsize() || y < 0 || y >= ysize())
        return true;
    else if (checked[y][x] == true)
        return true;

    checked[y][x] = true;

    if (buffer[y][x] == Go::eDame)
        return false;
    else if (buffer[y][x] != color)
        return true;

    if (isDeadStone(x, y-1, color, checked) == false)
        return false;
    if (isDeadStone(x, y+1, color, checked) == false)
        return false;
    if (isDeadStone(x-1, y, color, checked) == false)
        return false;
    if (isDeadStone(x+1, y, color, checked) == false)
        return false;

    return true;
}

/**
  @retval true  (x,y) position's stone can kill arround group
  @retval false (x,y) position's stone can't kill arround group
*/
bool BoardWidget::isKillStone(int x, int y){
    Go::Color color;
    if (buffer[y][x] == Go::eBlack)
        color = Go::eWhite;
    else if (buffer[y][x] == Go::eWhite)
        color = Go::eBlack;
    else
        return false;

    if (isDeadStone(x-1, y, color) == true)
        return true;
    if (isDeadStone(x+1, y, color) == true)
        return true;
    if (isDeadStone(x, y-1, color) == true)
        return true;
    if (isDeadStone(x, y+1, color) == true)
        return true;

    return false;
}

/**
  create node list
*/
void BoardWidget::createNodeList(Go::NodePtr node){
    currentNodeList_.clear();

    Go::NodePtr n = node;
    while(n){
        currentNodeList_.push_front(n);
        n = n->parent();
    }

    n = node;
    while(n->children().empty() == false){
        n = n->child(0);
        currentNodeList_.push_back(n);
    }
}

/**
  create stone item
*/
QGraphicsItem* BoardWidget::createStoneItem(Go::Color color, int sgfX, int sgfY){
    // get stone position from sgf coordinate
    qreal x, y;
    if (sgfToViewCoordinate(sgfX, sgfY, x, y) == false)
        return NULL;

    // stone size
    qreal size = getGridSize() * 0.95;

    // create stone item
    QGraphicsEllipseItem* stone = new QGraphicsEllipseItem(x-size/2, y-size/2, size, size);
    stone->setPen(QPen(Qt::black));
    stone->setBrush(QBrush(color == Go::eBlack ? Qt::black : Qt::white));

    stone->setZValue(3);
    scene()->addItem(stone);

    return stone;
}

/**
  create branch markers
*/
void BoardWidget::createBranchMarkers(){
    if (!currentNode_ || showVariation_ == false)
        return;

    if (currentGame_->information()->variationStyle() == 0)
        createChildBranchMarkers();
    else if (currentGame_->information()->variationStyle() == 1)
        createSiblingsranchMarkers();
}

/**
  show variations of successor node (children)
*/
void BoardWidget::createChildBranchMarkers(){
    char c = 'A';
    foreach(const Go::NodePtr& node, currentNode_->children()){
        if (!node->isStone() || node->isPass())
            continue;

        // create text item
        QString str(c);
        TextItemPtr branch( scene()->addSimpleText(str) );
        branch->setZValue(4);
        data[node->y()][node->x()].branchItem = branch;

        // set item position
        qreal x, y;
        if (sgfToViewCoordinate(node->x(), node->y(), x, y) == false)
            return;
        branch->setPos(x-branch->boundingRect().size().width()/2.0, y-branch->boundingRect().size().height()/2.0);

        ++c;
    }
}

/**
  show variations of current node (siblings)
*/
void BoardWidget::createSiblingsranchMarkers(){
    // if current is root node, can't show variations
    Go::NodePtr parent = currentNode_->parent();
    if (!parent)
        return;

    char c = 'A';
    foreach(const Go::NodePtr& node, parent->children()){
        if (!node->isStone() || node->isPass())
            continue;

        // current node is not shown
        if (node == currentNode_){
            ++c;
            continue;
        }

        // create text item
        QString str(c);
        TextItemPtr branch( scene()->addSimpleText(str) );
        branch->setZValue(4);
        data[node->y()][node->x()].branchItem = branch;

        // set item position
        qreal x, y;
        if (sgfToViewCoordinate(node->x(), node->y(), x, y) == false)
            return;
        branch->setPos(x-branch->boundingRect().size().width()/2.0, y-branch->boundingRect().size().height()/2.0);

        ++c;
    }
}

/**
    create graphics item of move number
*/
void BoardWidget::createMoveNumber(int sgfX, int sgfY, int number, bool active){
    // item position
    qreal x, y;
    if (sgfToViewCoordinate(sgfX, sgfY, x, y) == false)
        return;

    // create text item
    TextItemPtr text( scene()->addSimpleText(QString::number(number)) );
    text->setZValue(4);
    data[sgfY][sgfX].numberItem = text;
    text->setPos(x-text->boundingRect().size().width()/2.0, y-text->boundingRect().size().height()/2.0);

    // font color
    QColor color;
    if (active){
        color = Qt::red;
        data[sgfY][sgfX].focus = true;
    }
    else if (buffer[sgfY][sgfX] == Go::eBlack){
        color = Qt::white;
        data[sgfY][sgfX].focus = false;
    }
    else if (buffer[sgfY][sgfX] == Go::eWhite){
        color = Qt::black;
        data[sgfY][sgfX].focus = false;
    }
    text->setBrush( QBrush(color) );
//    text->setPen( QPen(Qt::red) );
}

/**
  create graphics item of markers
*/
void BoardWidget::createMarkers(){
    if (!currentNode_)
        return;

    foreach(const Go::Mark& m, currentNode_->marks())
        createMark(m);
}

/**
  create graphics item of markers
*/
void BoardWidget::createMark(const Go::Mark& m){
    QColor color = Qt::red;
    QPen pen(color, 2.0);

    qreal x, y;
    if (sgfToViewCoordinate(m.x(), m.y(), x, y) == false)
        return;

    qreal w = getGridSize() * 0.5;

    // make path of mark
    QPainterPath path;
    if (m.type() == Go::Mark::eCircle){
        path.addEllipse(x - w / 2.0, y - w / 2.0, w, w);
    }
    else if (m.type() == Go::Mark::eSquare){
        w *= 0.8;
        path.addRect(x - w / 2.0, y - w / 2.0, w, w);
    }
    else if (m.type() == Go::Mark::eTriangle){
        path.moveTo(0, 0);
        path.lineTo(w / -2.0, w * 0.8);
        path.lineTo(w / 2.0, w * 0.8);
        path.lineTo(0, 0);
        path.translate(x, y - w * 0.5);
    }
    else if (m.type() == Go::Mark::eCross){
        w *= 0.8;
        path.moveTo(0, 0);
        path.lineTo(w, w);
        path.moveTo(w, 0);
        path.lineTo(0, w);
        path.translate(x - w/2.0, y - w/2.0);
    }

    // draw background if mark isn't on the stone
    QGraphicsPathItem* item;
    if (data[m.y()][m.x()].color == Go::eDame){
        GraphicsPathItem* item2 = new GraphicsPathItem(path);
        item2->setBackgroundImage( createBackgroundImage(x, y) );
        item = item2;
    }
    else
        item = new QGraphicsPathItem(path);

    item->setPen(pen);
    item->setZValue(4);

    scene()->addItem(item);
    data[m.y()][m.x()].markerItem = GraphicsItemPtr(item);

}

/**
  create background of marker
*/
QPixmap BoardWidget::createBackgroundImage(int x, int y){
    qreal w = getGridSize();
    QPixmap pixmap(w, w);
    QPainter painter(&pixmap);
    painter.translate( (x - w / 2.0) * -1, (y - w / 2.0) * -1);
    painter.fillRect(x - w / 2.0, y - w  /2.0, w, w, board->brush());

    return pixmap;
}

/**
  get star positions
*/
void BoardWidget::getStarPositions(QList<int>& xstarpos, QList<int>& ystarpos) const{
    if (xsize() > 6 && xsize() < 10){
        xstarpos.push_back(2);
        xstarpos.push_back(xsize()-3);
    }
    if (xsize() > 10){
        xstarpos.push_back(3);
        xstarpos.push_back(xsize()-4);

        if (xsize() > 6 && xsize() % 2 == 1)
            xstarpos.push_back(xsize() / 2);
    }

    if (ysize() > 6 && ysize() < 10){
        ystarpos.push_back(2);
        ystarpos.push_back(ysize()-3);
    }
    if (ysize() > 10){
        ystarpos.push_back(3);
        ystarpos.push_back(ysize()-4);

        if (ysize() > 6 && ysize() % 2 == 1)
            ystarpos.push_back(ysize() / 2);
    }
}

/**
  view coordinate to sgf coordinate
*/
bool BoardWidget::viewToSgfCoordinate(qreal viewX, qreal viewY, int& sgfX, int& sgfY) const{
    qreal size = getGridSize();

//    if ((rotate % 2) == 0){
        sgfX = floor( (viewX - vLines[0]->line().x1() + size / 2.0) / size );
        sgfY = floor( (viewY - hLines[0]->line().y1() + size / 2.0) / size );
//    }
//    else{
//        sgfX = (fabs(viewY - vLines[0]->line().y1()) + size / 2.0) / size;
//        sgfY = (fabs(viewX - hLines[0]->line().x1()) + size / 2.0) / size;
//    }

    return sgfX >= 0 && sgfX < xsize() && sgfY >= 0 && sgfY < ysize();
}

/**
  sgf coordinate to view coordinate
*/
bool BoardWidget::sgfToViewCoordinate(int sgfX, int sgfY, qreal& viewX, qreal& viewY) const{
//    if ((rotate % 2) == 0){
        viewX = vLines[sgfX]->line().x1();
        viewY = hLines[sgfY]->line().y1();
//    }
//    else{
//        sgfX = (fabs(viewY - vLines[0]->line().y1()) + size / 2.0) / size;
//        sgfY = (fabs(viewX - hLines[0]->line().x1()) + size / 2.0) / size;
//    }

    return true;
}

/**
  move stone
  create and put stone
*/
bool BoardWidget::alternateMove(int sgfX, int sgfY){
    // if stone already exist, can't put new stone.
    if (data[sgfY][sgfX].color != Go::eDame)
        return false;

    // activate stone if next move already exists.
    foreach(const Go::NodePtr& child, currentNode_->children())
        if (child->x() == sgfX && child->y() == sgfY){
            setNode(child);
            return false;
        }

    // can not suicide move
    buffer[sgfY][sgfX] = currentNode_->nextColor();
    if (isDeadStone(sgfX, sgfY) && isKillStone(sgfX, sgfY) == false){
        buffer[sgfY][sgfX] = Go::eDame;
        return false;
    }

    // create new node, and add to document
    Go::NodePtr node( Go::createStoneNode(currentNode_->nextColor(), sgfX, sgfY) );
    document()->undoStack()->push( new AddNodeCommand(document(), currentGame_, currentNode_, node, -1) );

    return true;
}

/**
  add stone
  create and put stone.
  this stone don't kill enemy stones.
*/
bool BoardWidget::addStone(int sgfX, int sgfY, Go::Color color){
    // remove stone if stone already added
    bool removed = false;
    removed = removeStone(sgfX, sgfY, currentNode_->blackStones());
    removed |= removeStone(sgfX, sgfY, currentNode_->whiteStones());
    removed |= removeStone(sgfX, sgfY, currentNode_->emptyStones());
    if (removed)
        return true;

    // if stone already exist, can't put new stone.
    if (color != Go::eDame && data[sgfY][sgfX].color != Go::eDame)
        return false;
    else if (color == Go::eDame && data[sgfY][sgfX].color == Go::eDame)
        return false;

    // create new node, and add to document
    document()->undoStack()->push( new AddStoneCommand(document(), currentGame_, currentNode_, sgfX, sgfY, color) );

    return true;
}

/**
  remove stone
*/
bool BoardWidget::removeStone(int sgfX, int sgfY, QList<QPoint>& stones){
    foreach(const QPoint& p, stones){
        if (p.x() == sgfX && p.y() == sgfY){
            document()->undoStack()->push( new RemoveStoneCommand(document(), currentGame_, currentNode_, stones, p) );
            return true;
        }
    }

    return false;
}

/**
  move next node
*/
void BoardWidget::forward(int step){
    Go::NodeList::const_iterator iter = qFind(currentNodeList_.begin(), currentNodeList_.end(), currentNode_);
    if (iter == currentNodeList_.end())
        return;

    Go::NodeList::const_iterator pos = iter;
    for (int i=0; i<step; ++i){
        if (++iter == currentNodeList_.end())
            break;
        pos = iter;
    }
    setNode(*pos);
}

/**
  move previousn node
*/
void BoardWidget::back(int step){
    Go::NodeList::const_iterator iter = qFind(currentNodeList_.begin(), currentNodeList_.end(), currentNode_);
    if (iter == currentNodeList_.end())
        return;

    for (int i=0; i<step; ++i){
        if (iter == currentNodeList_.begin())
            break;
        --iter;
    }
    setNode(*iter);
}

/**
  node added

  @param[in] node
*/
void BoardWidget::on_document_nodeAdded(const Go::NodePtr& game, const Go::NodePtr& node){
    if (currentGame_ != game)
        return;

    setNode(node);
}

/**
  node deleted

  @param[in] node
*/
void BoardWidget::on_document_nodeDeleted(const Go::NodePtr& game, const Go::NodePtr& node, bool /*removeChildren*/){
    if (currentGame_ != game)
        return;

/*
    // return if deleted node isn't in the current node list
    Go::NodeList::const_iterator iter = qFind(currentNodeList_.begin(), currentNodeList_.end(), node);
    if (iter == currentNodeList_.end())
        return;
*/

    // re-create node list
    createNodeList(node->parent());
    setNode(node->parent(), true);
}

/**
  node modified
*/
void BoardWidget::on_document_nodeModified(const Go::NodePtr& /*game*/, const Go::NodePtr& node){
    if (node != currentNode_)
        return;

    // create buffer
    createBoardBuffer();
}
