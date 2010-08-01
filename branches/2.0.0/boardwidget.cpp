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
#include <QTimer>
#include <phonon>
#include "mugoapp.h"
#include "boardwidget.h"
#include "sgfdocument.h"
#include "command.h"

#define _USE_MATH_DEFINES
#include <math.h>

/**
  Sound
*/
class Sound{
public:
    Sound();
    ~Sound();

    void setFilePath(const QString& path);
    const QString& getFilePath() const{ return filePath; }

    void play();

private:
    Phonon::MediaObject* media;
    QString filePath;
};

/**
  Constructor
*/
Sound::Sound(){
    media = Phonon::createPlayer(Phonon::NotificationCategory);
}

/**
  Destructor
*/
Sound::~Sound(){
    delete media;
}

/**
  set file path
*/
void Sound::setFilePath(const QString& path){
    media->setCurrentSource(filePath = path);
}

/**
  play sound
*/
void Sound::play(){
    if (media->currentTime() == media->totalTime()){
        media->stop();
        media->seek(0);
    }

    if (media->currentTime() == 0)
        media->play();
}


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
    painter->translate(-pos().x(), -pos().y());
    painter->fillRect(pos().x(), pos().y(), boundingRect().width(), boundingRect().height(), backgroundBrush_);
    painter->translate(pos().x(), pos().y());
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
    , tutorMode(TutorMode::noTutor)
    , scoreMode(ScoreMode::noScore)
    , jumpToClicked(false)
    , showMoveNumber(true)
    , resetMoveNumberMode(ResetMoveNumber::noReset)
    , showMoveNumberCount(-1)
    , showCoordinate(true)
    , showMarker(true)
    , rotate(0)
    , flipHorizontally(false)
    , flipVertically(false)
    , replayTimer(NULL)
    , replayInterval(AUTO_REPLAY_INTERVAL)
    , moveEnemy(false)

    // Preferences
    , boardColor(BOARD_COLOR)
    , backgroundColor(BG_COLOR)
    , tutorBackgroundColor(TUTOR_BG_COLOR)
    , coordinateColor(COORDINATE_COLOR)
    , coordinateFont("Sans")
    , whiteStoneType(Preference::internal)
    , whiteStoneColor(WHITE_STONE_COLOR)
    , blackStoneType(Preference::internal)
    , blackStoneColor(BLACK_STONE_COLOR)
    , branchColor(BRANCH_COLOR)
    , focusColor(FOCUS_COLOR)
    , focusType(0)
    , labelType(Preference::large)
    , labelFont("Sans")
    , playSound(true)
{
    moveSound = new Sound;

//    connect(document_, SIGNAL(nodeAdded(Go::NodePtr)), SLOT(on_sgfdocument_nodeAdded(Go::NodePtr)));
    connect(document_, SIGNAL(nodeDeleted(Go::NodePtr, bool)), SLOT(on_sgfdocument_nodeDeleted(Go::NodePtr, bool)));
    connect(document_, SIGNAL(nodeModified(Go::NodePtr, bool)), SLOT(on_sgfdocument_nodeModified(Go::NodePtr, bool)));
    connect(document_, SIGNAL(gameModified(Go::NodePtr)), SLOT(on_sgfdocument_gameModified(Go::NodePtr)));

    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    setScene(scene);

    // create board
    shadow = scene->addRect(0, 0, 1, 1, QPen(Qt::NoPen), QBrush(SHADOW_COLOR));
    board  = scene->addRect(0, 0, 1, 1, QPen(Qt::NoPen), QBrush(QPixmap(BOARD_IMAGE)));
    board->setZValue(1);

    // set current game
    setCurrentGame(document()->gameList.front());
}

/**
  Destructor
*/
BoardWidget::~BoardWidget()
{
    delete moveSound;
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
    if (tutorMode != TutorMode::noTutor)
        return;
    else if (scoreMode != ScoreMode::noScore)
        return;

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
    int x, y;
    boardToSgfCoordinate(e->x(), e->y(), x, y);
    if (x < 0 || y < 0 || x >= gameInformation->xsize || y >= gameInformation->ysize)
        return;

    if (scoreMode == ScoreMode::final){
        if (boardBuffer[y][x].color != Go::empty)
            setDeadStones(x, y);
        return;
    }
    else if (tutorMode == TutorMode::tutorBothSides){
        moveToChildItem(x, y);
        return;
    }
    else if (tutorMode == TutorMode::tutorOneSide){
        if (moveEnemy || moveToChildItem(x, y) == false)
            return;

        Go::NodeList::iterator iter = qFind(currentNodeList.begin(), currentNodeList.end(), currentNode);
        if (iter == currentNodeList.end())
            return;
        else if (++iter == currentNodeList.end())
            return;

        moveEnemy = true;
        QTimer* timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), SLOT(on_tutorOneSide_timeout()));
        timer->start(680);

        return;
    }
    else if (jumpToClicked){
        // if jump to clicked mode, change current node and return
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
  get line position
*/
void BoardWidget::getGridLinePosition(int x, int y, int gridSize, int x1, int y1, int x2, int y2, QLineF& line) const{
    if (flipHorizontally){
        x1 = vLines.size() - x1 - 1;
        x2 = vLines.size() - x2 - 1;
    }

    if (flipVertically){
        y1 = hLines.size() - y1 - 1;
        y2 = hLines.size() - y2 - 1;
    }

    qreal xx1, yy1, xx2, yy2;
    if (rotate == 0){
        xx1 = x + x1 * gridSize;
        xx2 = x + x2 * gridSize;
        yy1 = y + y1 * gridSize;
        yy2 = y + y2 * gridSize;
    }
    else if (rotate == 1){
        xx1 = x + (hLines.size() - y1 - 1) * gridSize;
        xx2 = x + (hLines.size() - y2 - 1) * gridSize;
        yy1 = y + x1 * gridSize;
        yy2 = y + x2 * gridSize;
    }
    else if (rotate == 2){
        xx1 = x + (vLines.size() - x1 - 1) * gridSize;
        xx2 = x + (vLines.size() - x2 - 1) * gridSize;
        yy1 = y + (hLines.size() - y1 - 1) * gridSize;
        yy2 = y + (hLines.size() - y2 - 1) * gridSize;
    }
    else if (rotate == 3){
        xx1 = x + y1 * gridSize;
        xx2 = x + y2 * gridSize;
        yy1 = y + (vLines.size() - x1 - 1) * gridSize;
        yy2 = y + (vLines.size() - x2 - 1) * gridSize;
    }

    line.setLine(xx1, yy1, xx2, yy2);
}

/**
  get grid size
*/
qreal BoardWidget::getGridSize() const{
    if ((rotate % 2) == 0)
        return fabs(vLines[1]->line().x1() - vLines[0]->line().x1());
    else
        return fabs(vLines[1]->line().y1() - vLines[0]->line().y1());
}

/**
  grid coordinate to view coordinate
*/
void BoardWidget::sgfToBoardCoordinate(int sgfX, int sgfY, qreal& boardX, qreal& boardY) const{
    if ((rotate % 2) == 0){
        boardX = vLines[sgfX]->line().x1();
        boardY = hLines[sgfY]->line().y1();
    }
    else{
        boardX = hLines[sgfY]->line().x1();
        boardY = vLines[sgfX]->line().y1();
    }
}

/**
  view coordinate to sgf coordinate
*/
void BoardWidget::boardToSgfCoordinate(qreal boardX, qreal boardY, int& sgfX, int& sgfY) const{
    qreal size = getGridSize();

    if ((rotate % 2) == 0){
        sgfX = (fabs(boardX - vLines[0]->line().x1()) + size / 2.0) / size;
        sgfY = (fabs(boardY - hLines[0]->line().y1()) + size / 2.0) / size;
    }
    else{
        sgfX = (fabs(boardY - vLines[0]->line().y1()) + size / 2.0) / size;
        sgfY = (fabs(boardX - hLines[0]->line().x1()) + size / 2.0) / size;
    }
}

void BoardWidget::setVCoordinatePosition(int pos, const QRectF& rect, QGraphicsSimpleTextItem* left, QGraphicsSimpleTextItem* right){
    if (rotate == 0){
        qreal x1 = board->rect().left();
        qreal x2 = board->rect().right();
        qreal y1 = hLines[pos]->line().y1();
        qreal y2 = y1;
        left->setPos(x1 - (rect.width() - left->boundingRect().width()) / 2.0 - left->boundingRect().width() - 4
                   , y1 - left->boundingRect().height() / 2);
        right->setPos(x2 + (rect.width() - right->boundingRect().width()) / 2.0 + 4
                    , y2 - right->boundingRect().height() / 2);
    }
    else if (rotate == 1){
        qreal x1 = hLines[pos]->line().x1();
        qreal x2 = x1;
        qreal y1 = board->rect().top();
        qreal y2 = board->rect().bottom();
        left->setPos(x1 - left->boundingRect().width() / 2
                   , y1 - left->boundingRect().height() - 4);
        right->setPos(x2 - right->boundingRect().width() / 2
                    , y2 + 4);
    }
    else if (rotate == 2){
        qreal x1 = board->rect().right();
        qreal x2 = board->rect().left();
        qreal y1 = hLines[pos]->line().y1();
        qreal y2 = y1;
        left->setPos(x1 + (rect.width() - left->boundingRect().width()) / 2.0 + 4
                   , y1 - left->boundingRect().height() / 2);
        right->setPos(x2 - (rect.width() - right->boundingRect().width()) / 2.0 - right->boundingRect().width() - 4
                    , y2 - right->boundingRect().height() / 2);
    }
    else if (rotate == 3){
        qreal x1 = hLines[pos]->line().x1();
        qreal x2 = x1;
        qreal y1 = board->rect().bottom();
        qreal y2 = board->rect().top();
        left->setPos(x1 - left->boundingRect().width() / 2
                   , y1 + 4);
        right->setPos(x2 - right->boundingRect().width() / 2
                    , y2 - right->boundingRect().height() - 4);
    }
}

void BoardWidget::setHCoordinatePosition(int pos, const QRectF& rect, QGraphicsSimpleTextItem* top, QGraphicsSimpleTextItem* bottom){
    if (rotate == 0){
        qreal x1 = vLines[pos]->line().x1();
        qreal x2 = x1;
        qreal y1 = board->rect().top();
        qreal y2 = board->rect().bottom();
        top->setPos(x1 - top->boundingRect().width() / 2
                  , y1 - top->boundingRect().height() - 4);
        bottom->setPos(x2 - bottom->boundingRect().width() / 2
                     , y2 + 4);
    }
    else if (rotate == 1){
        qreal x1 = board->rect().right();
        qreal x2 = board->rect().left();
        qreal y1 = vLines[pos]->line().y1();
        qreal y2 = y1;
        top->setPos(x1 + (rect.width() - top->boundingRect().width()) / 2 + 4
                  , y1 - top->boundingRect().height() / 2);
        bottom->setPos(x2 - (rect.width() - bottom->boundingRect().width()) / 2 - bottom->boundingRect().width() - 4
                     , y2 - bottom->boundingRect().height() / 2);
    }
    else if (rotate == 2){
        qreal x1 = vLines[pos]->line().x1();
        qreal x2 = x1;
        qreal y1 = board->rect().bottom();
        qreal y2 = board->rect().top();
        top->setPos(x1 - top->boundingRect().width() / 2
                  , y1 + 4);
        bottom->setPos(x2 - bottom->boundingRect().width() / 2
                     , y2 - bottom->boundingRect().height() - 4);
    }
    else if (rotate == 3){
        qreal x1 = board->rect().left();
        qreal x2 = board->rect().right();
        qreal y1 = vLines[pos]->line().y1();
        qreal y2 = y1;
        top->setPos(x1 - (rect.width() - top->boundingRect().width()) / 2 - top->boundingRect().width() - 4
                  , y1 - top->boundingRect().height() / 2);
        bottom->setPos(x2 + (rect.width() - bottom->boundingRect().width()) / 2 + 4
                     , y2 - bottom->boundingRect().height() / 2);
    }
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

    // create board items(board, shadow, lines, coordinate)
    createBoard();

    emit currentGameChanged(currentGame);

    setCurrentNode(game, forceChange);
}

/**
  Set Current Node
*/
void BoardWidget::setCurrentNode(Go::NodePtr node, bool forceChange){
    if (forceChange == false && node == currentNode)
        return;

    // get game information in new current node
    currentNode = node;
    gameInformation = currentNode->getInformation();

    // create buffer
    Go::NodeList::iterator iter = qFind(currentNodeList.begin(), currentNodeList.end(), currentNode);
    if (iter == currentNodeList.end())
        createBuffer(true);
    else
        createBuffer(false);

    // emit
    emit currentNodeChanged(currentNode);

    // play sound
    if (playSound && inBoard(currentNode))
        moveSound->play();
}

/**
  create board
*/
void BoardWidget::createBoard(){
    // delete lines and stars
    qDeleteAll(hLines);
    qDeleteAll(vLines);
    hLines.clear();
    vLines.clear();

    qDeleteAll(stars);
    stars.clear();

    qDeleteAll(coordinateLeft);
    qDeleteAll(coordinateRight);
    qDeleteAll(coordinateTop);
    qDeleteAll(coordinateBottom);
    coordinateLeft.clear();
    coordinateRight.clear();
    coordinateTop.clear();
    coordinateBottom.clear();

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

    // create coordinate
    for (int i=gameInformation->ysize; i>0; --i){
        QGraphicsSimpleTextItem* left  = scene->addSimpleText(QString::number(i), QFont(coordinateFont, 10));
        QGraphicsSimpleTextItem* right = scene->addSimpleText(QString::number(i), QFont(coordinateFont, 10));
        left->setZValue(2);
        right->setZValue(2);
        left->setBrush( QBrush(coordinateColor) );
        right->setBrush( QBrush(coordinateColor) );
        coordinateLeft.push_back(left);
        coordinateRight.push_back(right);
    }
    for (int i=0; i<gameInformation->xsize; ++i){
        int n = i % (document()->showCoordinateWithI ? 26 : 25);
        if (document()->showCoordinateWithI == false && n > 7)
            ++n;
        QGraphicsSimpleTextItem* top    = scene->addSimpleText(QString().sprintf("%c", 'A' + n), QFont(coordinateFont, 10));
        QGraphicsSimpleTextItem* bottom = scene->addSimpleText(QString().sprintf("%c", 'A' + n), QFont(coordinateFont, 10));
        top->setZValue(2);
        bottom->setZValue(2);
        top->setBrush( QBrush(coordinateColor) );
        bottom->setBrush( QBrush(coordinateColor) );
        coordinateTop.push_back(top);
        coordinateBottom.push_back(bottom);
    }

    setItemsPosition();
}

/**
  set scene items position
*/
void BoardWidget::setItemsPosition(){
    QRectF r = scene->sceneRect();

    // get coordinate item size
    QRectF coordinateRect;
    foreach(QGraphicsSimpleTextItem* item, coordinateLeft){
        coordinateRect.setWidth(qMax(coordinateRect.width(), item->boundingRect().width()));
        coordinateRect.setHeight(qMax(coordinateRect.height(), item->boundingRect().height()));
    }
    foreach(QGraphicsSimpleTextItem* item, coordinateTop){
        coordinateRect.setWidth(qMax(coordinateRect.width(), item->boundingRect().width()));
        coordinateRect.setHeight(qMax(coordinateRect.height(), item->boundingRect().height()));
    }

    // calculate board size
    int width  = r.width();
    int height = r.height();
    if (showCoordinate){
        width  -= coordinateRect.width()  * 2 + 10;
        height -= coordinateRect.height() * 2 + 10;
    }

    int xsize = gameInformation->xsize;
    int ysize = gameInformation->ysize;
    if (rotate % 2)
        qSwap(xsize, ysize);

    int gridW = width  / (xsize + 1);
    int gridH = height / (ysize + 1);
    int gridSize = qMin(gridW, gridH);
    int w = gridSize * (xsize - 1);
    int h = gridSize * (ysize - 1);

    int x = (r.width() - w) / 2;
    int y = (r.height() - h) / 2;
    int margin = gridSize * 0.6;
    QRect boardRect(QPoint(x-margin, y-margin), QPoint(x+w+margin, y+h+margin));

    // set position of shadow.
    shadow->setRect(boardRect.left()+3, boardRect.top()+3, boardRect.width(), boardRect.height());

    // set position of board.
    board->setRect(boardRect);

    // set position of vertical lines.
    int i = 0;
    foreach(QGraphicsLineItem* item, vLines){
        QLineF line;
        getGridLinePosition(x, y, gridSize, i, 0, i, hLines.size()-1, line);
        item->setLine(line);
        ++i;
    }

    // set position of horizontal lines.
    i = 0;
    foreach(QGraphicsLineItem* item, hLines){
        QLineF line;
        getGridLinePosition(x, y, gridSize, 0, i, vLines.size()-1, i, line);
        item->setLine(line);
        ++i;
    }

    // set position of stars.
    QList<int> xpos, ypos;
    getStarPosition(xpos, ypos);
    for (int y=0, n=0; y<ypos.size(); ++y){
        for (int x=0; x<xpos.size(); ++x, ++n){
            qreal xx, yy;
            sgfToBoardCoordinate(xpos[x], ypos[y], xx, yy);
            stars[n]->setRect(xx-2, yy-2, 4, 4);
        }
    }

    // set positions of coordinate
    for (int i=0; i<gameInformation->ysize; ++i){
        setVCoordinatePosition(i, coordinateRect, coordinateLeft[i], coordinateRight[i]);
        coordinateLeft[i]->setVisible(showCoordinate);
        coordinateRight[i]->setVisible(showCoordinate);
    }
    for (int i=0; i<gameInformation->xsize; ++i){
        setHCoordinatePosition(i, coordinateRect, coordinateTop[i], coordinateBottom[i]);
        coordinateTop[i]->setVisible(showCoordinate);
        coordinateBottom[i]->setVisible(showCoordinate);
    }

    // set position of stones and move number.
    createStonePixmap();
    createBoardItemList();
}

/**
  set graphics text item position
*/
void BoardWidget::setTextItemPosition(QGraphicsSimpleTextItem* text, int x, int y){
    qreal size = getGridSize() * 0.95;
    if (text){
        qreal number_size;
        if (text->text().size() > 2)
            number_size = size * 0.4;
        else if (text->text().size() > 1)
            number_size = size * 0.45;
        else
            number_size = size * 0.5;

        QFont f = text->font();
        f.setPointSizeF(number_size);
        text->setFont(f);
        QRectF r = text->boundingRect();

        qreal xx, yy;
        sgfToBoardCoordinate(x, y, xx, yy);
        text->setPos(xx-r.width()*0.5, yy-r.height()*0.5);
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

    number = 0;
    moveNumber = 0;
    capturedBlack = 0;
    capturedWhite = 0;
    Go::NodeList::iterator node = currentNodeList.begin();
    do{
        if ((*node)->isStone())
            ++moveNumber;

        if ( inBoard(*node) ){
            TerritoryInfo& ti = boardBuffer[(*node)->y()][(*node)->x()];

            // reset move number
            Go::NodePtr parent = (*node)->parent();
            bool isBranch = parent && parent->childNodes.size() > 1;
            bool firstChild = parent && parent->childNodes.indexOf(*node) == 0;
            bool resetMoveNumber = (*node)->moveNumber > 0;
            if ((isBranch && resetMoveNumberMode == ResetMoveNumber::allBranch) || (!firstChild && resetMoveNumberMode != ResetMoveNumber::noReset))
                resetMoveNumber = true;

            if (resetMoveNumber){
                number = 0;
                moveNumber = (*node)->moveNumber ? (*node)->moveNumber : 1;
                for (int y=0; y<boardBuffer.size(); ++y){
                    for (int x=0; x<boardBuffer[y].size(); ++x){
                        if (ti.number)
                            ti.number = 0;
                    }
                }
            }

            // add stone to buffer
            ti.color = (*node)->color;
            ti.moveNumber = moveNumber;
            ti.number = ++number;
            killStones((*node)->x(), (*node)->y());
        }

        // add stones
        foreach(const Go::Stone& stone, (*node)->whiteStones)
            boardBuffer[stone.position.y][stone.position.x].color = stone.color;
        foreach(const Go::Stone& stone, (*node)->blackStones)
            boardBuffer[stone.position.y][stone.position.x].color = stone.color;
        foreach(const Go::Stone& stone, (*node)->emptyStones)
            boardBuffer[stone.position.y][stone.position.x].color = stone.color;

        // add dims
        foreach(const Go::Mark& dim, (*node)->dims)
            boardBuffer[dim.position.y][dim.position.x].dim = &dim;
    } while(*node++ != currentNode);

    // add mark
    foreach(const Go::Mark& mark, currentNode->marks)
        boardBuffer[mark.position.y][mark.position.x].mark = &mark;
    foreach(const Go::Line& line, currentNode->lines)
        boardBuffer[line.position1.y][line.position1.x].lineList.push_back(line);
    foreach(const Go::Mark& mark, currentNode->whiteTerritories)
        boardBuffer[mark.position.y][mark.position.x].territory = Go::white;
    foreach(const Go::Mark& mark, currentNode->blackTerritories)
        boardBuffer[mark.position.y][mark.position.x].territory = Go::black;

    createBoardItemList();
}

/**
  create mark items
*/
void BoardWidget::createBoardItemList(){
    if (currentNode == NULL)
        return;

    createStoneItemList();
    createNumberItemList();
    createMarkItemList();
}

/**
  create stone items
*/
void BoardWidget::createStoneItemList(){
    qDeleteAll(stones);
    stones.clear();

    for(int y=0; y<boardBuffer.size(); ++y){
        for(int x=0; x<boardBuffer.size(); ++x){
            TerritoryInfo& ti = boardBuffer[y][x];

            if (ti.color != Go::empty){
                QGraphicsItem* item = createStoneItem(x, y, ti.color);
                stones.push_back(item);
                ti.stoneItem = item;
            }
        }
    }
}

/**
  create number item list
*/
void BoardWidget::createNumberItemList(){
    qDeleteAll(numbers);
    numbers.clear();

    for(int y=0; y<boardBuffer.size(); ++y){
        for(int x=0; x<boardBuffer.size(); ++x){
            TerritoryInfo& ti = boardBuffer[y][x];

            if (showMoveNumber && ti.number > 0 && ti.color != Go::empty){
                if (showMoveNumberCount == -1 || number - ti.number < showMoveNumberCount){
                    QGraphicsSimpleTextItem* item = createMoveNumberItem(x, y, ti.moveNumber);
                    if (ti.number == number){
                        QFont f = item->font();
                        f.setWeight(QFont::Bold);
                        item->setFont(f);
                        item->setBrush(QBrush(focusColor));
                    }
                    numbers.push_back(item);
                }
                else if (showMoveNumberCount == 0 && ti.number == number)
                    createFocusItem(x, y);
            }
        }
    }
}

/**
  create mark items
*/
void BoardWidget::createMarkItemList(){
    qDeleteAll(marks);
    marks.clear();

    for(int y=0; y<boardBuffer.size(); ++y){
        for(int x=0; x<boardBuffer.size(); ++x){
            TerritoryInfo& ti = boardBuffer[y][x];

            if (ti.territory != Go::empty){
                QGraphicsItem* item = createTerritoryItem(x, y, ti.territory);
                marks.push_back(item);
                if (ti.stoneItem)
                    ti.stoneItem->setOpacity(0.3);
            }
            else if (ti.stoneItem)
                ti.stoneItem->setOpacity(1.0);


            if (showMarker){
                if (ti.mark && showMarker){
                    QGraphicsItem* item = createMarkItem(*ti.mark);
                    marks.push_back(item);
                }

                foreach(const Go::Line& line, ti.lineList){
                    QGraphicsItem* item = createLineItem(line.position1.x, line.position1.y, line.position2.x, line.position2.y, line.type);
                    marks.push_back(item);
                }

                if (ti.dim){
                    QGraphicsItem* item = createMarkItem(*ti.dim);
                    marks.push_back(item);
                }
            }
        }
    }
    createVariationItemList(currentNode);
}

 /**
  erase buffer
*/
void BoardWidget::eraseBuffer(){
    // clear buffer
    currentNodeList.clear();

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
}

/**
  create territories
*/
void BoardWidget::createTerritories(){
    for (int y=0; y<boardBuffer.size(); ++y)
        for (int x=0; x<boardBuffer[y].size(); ++x)
            setTerritories(x, y);
    createMarkItemList();

    int total = boardBuffer.size() * boardBuffer[0].size();
    int alive_b = 0, alive_w = 0, dead_b = 0, dead_w = 0, blackTerritory = 0, whiteTerritory = 0;

    for (int y=0; y<boardBuffer.size(); ++y){
        for (int x=0; x<boardBuffer[y].size(); ++x){
            TerritoryInfo& ti = boardBuffer[y][x];
            if (ti.territory == Go::black)
                ++blackTerritory;
            else if (ti.territory == Go::white)
                ++whiteTerritory;

            if (ti.color == Go::white && ti.territory == Go::black)
                ++dead_w;
            else if (ti.color == Go::black && ti.territory == Go::white)
                ++dead_b;
        }
    }


    emit scoreUpdated(total, alive_b, alive_w, dead_b, dead_w, capturedBlack, capturedWhite, blackTerritory, whiteTerritory);
}

void BoardWidget::setTerritories(int x, int y){
    if (boardBuffer[y][x].isStone() && boardBuffer[y][x].isTerritory() == false)
        return;

    int size = boardBuffer.size() * boardBuffer[0].size();
    char* buf = new char[size];
    memset(buf, 0, size);

    bool black=false, white=false;
    getTerritory(x, y, buf, black, white);
    setTerritories(buf, black && white ? Go::dame : black ? Go::black : Go::white);

    delete[] buf;
}

void BoardWidget::setTerritories(char* buf, Go::Color color){
    int xsize = boardBuffer[0].size();
    for (int y=0; y<boardBuffer.size(); ++y){
        for (int x=0; x<boardBuffer[y].size(); ++x){
            if (buf[y * xsize + x] == 0)
                continue;
            if (boardBuffer[y][x].isStone() == false || color != Go::dame)
                boardBuffer[y][x].territory = color;
            else
                boardBuffer[y][x].territory = Go::empty;
        }
    }
}

void BoardWidget::getTerritory(int x, int y, char* buf, bool& black, bool& white){
    if (black && white)
        return;
    else if (x < 0 || y < 0 || y >= boardBuffer.size() || x >= boardBuffer[y].size())
        return;
    else if ((boardBuffer[y][x].color == Go::black && boardBuffer[y][x].isTerritory() ==false) || boardBuffer[y][x].territory == Go::black){
        black = true;
        return;
    }
    else if ((boardBuffer[y][x].color == Go::white && boardBuffer[y][x].isTerritory() ==false) || boardBuffer[y][x].territory == Go::white){
        white = true;
        return;
    }

    int xsize = boardBuffer[0].size();
    if (buf[y*xsize+x])
        return;
    buf[y*xsize+x] = 1;

    getTerritory(x-1, y, buf, black, white);
    getTerritory(x+1, y, buf, black, white);
    getTerritory(x, y-1, buf, black, white);
    getTerritory(x, y+1, buf, black, white);

    return;
}

void BoardWidget::setDeadStones(int x, int y){
    Go::Color color = boardBuffer[y][x].color;
    Go::Color territory = Go::dame;
    if (boardBuffer[y][x].isTerritory() == false)
        territory = color == Go::black ? Go::white : Go::black;

    int size = boardBuffer.size() * boardBuffer[0].size();
    char* buf = new char[size];
    memset(buf, 0, size);

    setDeadStones(x, y, color, territory, buf);

    delete[] buf;

    createTerritories();
}

void BoardWidget::setDeadStones(int x, int y, Go::Color color, Go::Color territory, char* buf){
    if (x < 0 || y < 0 || y >= boardBuffer.size() || x >= boardBuffer[y].size())
        return;
    else if (boardBuffer[y][x].color != Go::empty && boardBuffer[y][x].color != color && boardBuffer[y][x].isTerritory() == false)
        return;

    int xsize = boardBuffer[0].size();
    if (buf[y*xsize+x])
        return;
    buf[y*xsize+x] = 1;

    if (boardBuffer[y][x].color != Go::empty && (territory == Go::dame || boardBuffer[y][x].color != color))
        boardBuffer[y][x].territory = Go::empty;
    else
        boardBuffer[y][x].territory = territory;

    setDeadStones(x-1, y, color, territory, buf);
    setDeadStones(x+1, y, color, territory, buf);
    setDeadStones(x, y-1, color, territory, buf);
    setDeadStones(x, y+1, color, territory, buf);
}

/**
  create focus item
*/
QAbstractGraphicsShapeItem* BoardWidget::createFocusItem(int x, int y){
    QAbstractGraphicsShapeItem* focus = NULL;
    if (showMoveNumberCount != 0)
        return focus;

    if (focusType == 0){
        focus = scene->addPath( createTrianglePath(x, y) );
        focus->setBrush(QBrush(focusColor));
    }
    else if (focusType == 1)
        focus = scene->addPath( createCirclePath(x, y) );
    else if (focusType == 2)
        focus = scene->addPath( createCrossPath(x, y) );
    else if (focusType == 3)
        focus = scene->addPath( createSquarePath(x, y) );
    else if (focusType == 4)
        focus = scene->addPath( createTrianglePath(x, y) );

    focus->setPen(QPen(focusColor, 2));
    focus->setZValue(5);

    marks.push_back(focus);
    return focus;
}

/**
  create variation label item list
*/
void BoardWidget::createVariationItemList(Go::NodePtr node){
    // return if show no markup
    if ((getShowVariations() != 0 && getShowVariations() != 1) || tutorMode != TutorMode::noTutor)
        return;

    // get variation list
    Go::NodeList variations;
    if (getShowVariations() == 0)   // show children
        variations = node->childNodes;
    else if (getShowVariations() == 1){   // show siblings
        Go::NodePtr p = node->parent();
        if (p)
            variations = p->childNodes;
    }

    // not show variations if variasion count is 0 or 1
    if (variations.size() <= 1)
        return;

    // create variation label
    char label = 'A';
    foreach(const Go::NodePtr& v, variations){
        QString str(label++);

        if (inBoard(v) == false)
            continue;
        else if (v == node)
            continue;

        TerritoryInfo& ti = boardBuffer[v->y()][v->x()];
        if (ti.mark)
            continue;

        GraphicsLabelTextItem* item = new GraphicsLabelTextItem(str);
        item->setZValue(4);
        item->setBrush( QBrush(branchColor) );

        if (ti.color == Go::empty)
            item->setBackgroundBrush(board->brush());

        setTextItemPosition(item, v->x(), v->y());
        scene->addItem(item);
        this->variations.push_back(item);

        marks.push_back(item);
    }
}

/**
  create graphic stone item
*/
QGraphicsItem* BoardWidget::createStoneItem(int x, int y, Go::Color color){
    qreal size = getGridSize();
    qreal xx, yy;
    sgfToBoardCoordinate(x, y, xx, yy);
    int sceneX = xx - size / 2;
    int sceneY = yy - size / 2;

    QGraphicsItem* stone = NULL;
    Preference::ResourceType type = color == Go::white ? whiteStoneType : blackStoneType;
    if (type == Preference::color){
        QColor& c = color == Go::white ? whiteStoneColor : blackStoneColor;
        stone = scene->addEllipse( sceneX, sceneY, size, size, QPen(Qt::black), QBrush(c) );
    }
    else{
        QPixmap& p = color == Go::white ? whiteStonePixmap : blackStonePixmap;
        QGraphicsPixmapItem* item = scene->addPixmap(p);
        item->setOffset(sceneX, sceneY);
        stone = item;
    }

    if (boardBuffer[y][x].territory != Go::empty)
        stone->setOpacity(0.3);

    stone->setZValue(3);

    return stone;
}

/**
 create scaled stone pixmap
*/
void BoardWidget::createStonePixmap(){
    qreal size = getGridSize();

    if (whiteStoneType != Preference::color){
        QString whiteImage = whiteStoneType == Preference::file ? whiteStoneImage : WHITE_STONE_IMAGE;
        QPixmap pixmap(whiteImage);
        whiteStonePixmap = pixmap.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    else
        whiteStonePixmap = QPixmap();

    if (blackStoneType != Preference::color){
        QString blackImage = blackStoneType == Preference::file ? blackStoneImage : BLACK_STONE_IMAGE;
        QPixmap pixmap(blackImage);
        blackStonePixmap = pixmap.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    else
        blackStonePixmap = QPixmap();
}

/**
  create move number item
*/
QGraphicsSimpleTextItem* BoardWidget::createMoveNumberItem(int x, int y, int number){
    QString text = QString::number(number);
    QGraphicsSimpleTextItem* item = scene->addSimpleText(text, QFont(labelFont));
    Go::Color color = boardBuffer[y][x].color;
    item->setBrush(QBrush(color == Go::black ? Qt::white : Qt::black));
    setTextItemPosition(item, x, y);
    item->setZValue(4);
    return item;
}

/**
  create graphic stone item
*/
QGraphicsItem* BoardWidget::createMarkItem(const Go::Mark& mark){
    QGraphicsItem* item = NULL;
    if (mark.type == Go::Mark::cross)
        item = scene->addPath( createCrossPath(mark.position.x, mark.position.y) );
    else if (mark.type == Go::Mark::circle)
        item = scene->addPath( createCirclePath(mark.position.x, mark.position.y) );
    else if (mark.type == Go::Mark::square)
        item = scene->addPath( createSquarePath(mark.position.x, mark.position.y) );
    else if (mark.type == Go::Mark::triangle)
        item = scene->addPath( createTrianglePath(mark.position.x, mark.position.y) );
    else if (mark.type == Go::Mark::character){
//        item = scene->addSimpleText(mark.text);
        item = new GraphicsLabelTextItem(mark.text);
        setTextItemPosition((GraphicsLabelTextItem*)item, mark.position.x, mark.position.y);
        scene->addItem(item);
    }
    else if (mark.type == Go::Mark::dim){
        item = scene->addRect( createRectPath(mark.position.x, mark.position.y), QPen(Qt::NoPen), QBrush(Qt::black) );
        item->setOpacity(0.65);
    }
    else if (mark.type == Go::Mark::select)
        item = scene->addPath( createSelectPath(mark.position.x, mark.position.y) );
    else
        return NULL;

    if (mark.type == Go::Mark::character){
        GraphicsLabelTextItem* text = static_cast<GraphicsLabelTextItem*>(item);
        if (boardBuffer[mark.position.y][mark.position.x].color == Go::black)
            text->setBrush( QBrush(Qt::white) );
        else if (boardBuffer[mark.position.y][mark.position.x].color == Go::white)
            text->setBrush( QBrush(Qt::black) );
        else{
            text->setBrush( QBrush(Qt::black) );
            text->setBackgroundBrush( board->brush() );
        }
        text->setFont(QFont(labelFont));
    }
    else if (mark.type == Go::Mark::dim){
    }
    else if (mark.type == Go::Mark::select){
        QAbstractGraphicsShapeItem* shape = static_cast<QAbstractGraphicsShapeItem*>(item);
        shape->setPen( QPen(Qt::NoPen) );

        if (boardBuffer[mark.position.y][mark.position.x].color == Go::black)
            shape->setBrush( QBrush(Qt::white) );
        else
            shape->setBrush( QBrush(Qt::black) );
    }
    else{
        QAbstractGraphicsShapeItem* shape = static_cast<QAbstractGraphicsShapeItem*>(item);
        if (boardBuffer[mark.position.y][mark.position.x].color == Go::black)
            shape->setPen( QPen(Qt::white, 2) );
        else
            shape->setPen( QPen(Qt::black, 2) );
    }

    item->setZValue(4);
    return item;
}

/**
  create territory item
*/
QGraphicsItem* BoardWidget::createTerritoryItem(int x, int y, Go::Color color){
    QAbstractGraphicsShapeItem* item = scene->addPath( createTerritoryPath(x, y) );
    item->setPen( QPen(Qt::NoPen) );

    if (color == Go::black)
        item->setBrush( QBrush(Qt::black) );
    else if (color == Go::white)
        item->setBrush( QBrush(Qt::white) );
    else if (color == Go::dame)
        item->setBrush( QBrush(Qt::red) );

    item->setZValue(4);
    return item;
}

/**
  create cross path
*/
QPainterPath BoardWidget::createCrossPath(int x, int y){
    qreal size = getGridSize() * 0.4;
    qreal xx, yy;
    sgfToBoardCoordinate(x, y, xx, yy);
    int sceneX = xx - size / 2;
    int sceneY = yy - size / 2;

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
QPainterPath BoardWidget::createCirclePath(int x, int y){
    qreal size = getGridSize() * 0.5;
    qreal xx, yy;
    sgfToBoardCoordinate(x, y, xx, yy);
    int sceneX = xx - size / 2;
    int sceneY = yy - size / 2;

    QPainterPath path;
    path.addEllipse(sceneX, sceneY, size, size);

    return path;
}

/**
  create square path
*/
QPainterPath BoardWidget::createSquarePath(int x, int y){
    qreal size = getGridSize() * 0.45;
    qreal xx, yy;
    sgfToBoardCoordinate(x, y, xx, yy);
    int sceneX = xx - size / 2;
    int sceneY = yy - size / 2;

    QPainterPath path;
    path.addRect(sceneX, sceneY, size, size);

    return path;
}

/**
  create triangle path
*/
QPainterPath BoardWidget::createTrianglePath(int x, int y){
    qreal xsize = getGridSize() * 0.5;
    qreal ysize = getGridSize() * 0.4;
    qreal xx, yy;
    sgfToBoardCoordinate(x, y, xx, yy);
    int sceneX = xx - xsize / 2;
    int sceneY = yy - ysize / 2;

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
QPainterPath BoardWidget::createTerritoryPath(int x, int y){
    qreal size = getGridSize() * 0.35;
    qreal xx, yy;
    sgfToBoardCoordinate(x, y, xx, yy);
    int sceneX = xx - size / 2;
    int sceneY = yy - size / 2;

    QPainterPath path;
    path.addRect(sceneX, sceneY, size, size);

    return path;
}

/**
  create select path
*/
QPainterPath BoardWidget::createSelectPath(int x, int y){
    qreal size = getGridSize() * 0.45;
    qreal xx, yy;
    sgfToBoardCoordinate(x, y, xx, yy);
    int sceneX = xx - size / 2;
    int sceneY = yy - size / 2;

    QPainterPath path;
    path.addRect(sceneX, sceneY, size, size);

    return path;
}

/**
  create line or arrow item
*/
GraphicsArrowItem* BoardWidget::createLineItem(int x1, int y1, int x2, int y2, Go::Line::Type type){
    qreal xx1, yy1, xx2, yy2;
    sgfToBoardCoordinate(x1, y1, xx1, yy1);
    sgfToBoardCoordinate(x2, y2, xx2, yy2);

    GraphicsArrowItem* item;
    GraphicsArrowItem::Shape shape = type == Go::Line::arrow ? GraphicsArrowItem::normal : GraphicsArrowItem::none;
    item = new GraphicsArrowItem(xx1, yy1, xx2, yy2, GraphicsArrowItem::none, shape);

    item->setPen( QPen(Qt::blue, 3) );
    item->setZValue(4);
    scene->addItem(item);

    return item;
}

/**
  create rect path
*/
QRectF BoardWidget::createRectPath(int x, int y){
    qreal size = getGridSize();
    qreal xx, yy;
    sgfToBoardCoordinate(x, y, xx, yy);

    return QRect(QPoint(xx-size/2, yy-size/2), QSize(size, size));
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

    if (ti.color != Go::empty && ti.moveNumber == 0){
        bool removed = removeStone(currentNode->blackStones, p) ||
                       removeStone(currentNode->whiteStones, p) ||
                       removeStone(currentNode->emptyStones, p);
        if (removed)
            return;
    }

    if ((color == Go::empty && ti.color != Go::empty) || (color != Go::empty && ti.color == Go::empty)){
        // if currentNode is stone, create child node and add stone to new node.
        if (currentNode->isStone()){
            Go::NodePtr node(new Go::Node(currentNode));
            addItem(currentNode, node);
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
        int c = labelType == Preference::number ? 0 : labelType == Preference::small ? 'a' - 1 : 'A' - 1;

        QStringList strList;
        foreach(const Go::Mark& mark, currentNode->marks){
            if (mark.type == Go::Mark::character)
                strList.push_back(mark.text);
        }

        do{
            if (labelType == Preference::number)
                str = QString::number(++c);
            else
                str.sprintf("%c", ++c);
        } while(strList.indexOf(str) >= 0);
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
void BoardWidget::setEditMode(EditMode::Mode mode){
    editMode = mode;
}

/**
  set tutor mode
*/
void BoardWidget::setTutorMode(TutorMode::Mode mode){
    tutorMode = mode;
    moveEnemy = false;

    if (tutorMode == TutorMode::noTutor)
        scene->setBackgroundBrush(QBrush(backgroundColor));
    else
        scene->setBackgroundBrush(QBrush(tutorBackgroundColor));

    if (tutorMode == TutorMode::replay){
        replayTimer = new QTimer(this);
        connect(replayTimer, SIGNAL(timeout()), SLOT(on_automaticReplay_timeout()));
        replayTimer->start(replayInterval);
    }
    else if (replayTimer){
        replayTimer->deleteLater();
        replayTimer = NULL;
    }

    createBuffer(false);
}

/**
*/
void BoardWidget::setScoreMode(ScoreMode::Mode mode){
    scoreMode = mode;
    createBuffer(false);
    if (mode == ScoreMode::final)
        createTerritories();
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
void BoardWidget::setResetMoveNumberMode(ResetMoveNumber::Mode mode){
    resetMoveNumberMode = mode;
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
  set show coordinate
*/
void BoardWidget::setShowCoordinate(bool show){
    showCoordinate = show;
    setItemsPosition();
}

/**
  set show coordinate with i
*/
void BoardWidget::setShowCoordinateWithI(bool show){
    document()->showCoordinateWithI = show;
    createBoard();
    document()->modifyGame(currentGame);
}

/**
  set show marker
*/
void BoardWidget::setShowMarker(bool show){
    showMarker = show;
    createBuffer(false);
}

/**
  set show variations
*/
void BoardWidget::setShowVariations(int variation){
    currentGame->gameInformation->variation = variation;
    createBuffer(false);
}

/**
  set board type
*/
void BoardWidget::setBoardType(Preference::ResourceType type){
    boardType = type;
    if (type == Preference::internal)
        board->setBrush(QBrush(QPixmap(BOARD_IMAGE)));
    else if (type == Preference::color)
        board->setBrush(QBrush(boardColor));
    else if (type == Preference::file)
        board->setBrush(QBrush(QPixmap(boardImage)));
}

/**
  set board color
*/
void BoardWidget::setBoardColor(const QColor& color){
    boardColor = color;
    if (boardType == Preference::color)
        board->setBrush(QBrush(boardColor));
}

/**
  set board image
*/
void BoardWidget::setBoardImage(const QString& file){
    boardImage = file;
    if (boardType == Preference::file)
        board->setBrush(QBrush(QPixmap(boardImage)));
}

/**
  set coordinate color
*/
void BoardWidget::setCoordinateColor(const QColor& color){
    QBrush brush(coordinateColor = color);

    foreach(QGraphicsSimpleTextItem* text, coordinateLeft)
        text->setBrush(brush);

    foreach(QGraphicsSimpleTextItem* text, coordinateRight)
        text->setBrush(brush);

    foreach(QGraphicsSimpleTextItem* text, coordinateTop)
        text->setBrush(brush);

    foreach(QGraphicsSimpleTextItem* text, coordinateBottom)
        text->setBrush(brush);
}

/**
  set coordinate font
*/
void BoardWidget::setCoordinateFont(const QString& fontName){
    QFont font(coordinateFont = fontName, 10);

    foreach(QGraphicsSimpleTextItem* text, coordinateLeft)
        text->setFont(font);

    foreach(QGraphicsSimpleTextItem* text, coordinateRight)
        text->setFont(font);

    foreach(QGraphicsSimpleTextItem* text, coordinateTop)
        text->setFont(font);

    foreach(QGraphicsSimpleTextItem* text, coordinateBottom)
        text->setFont(font);
}

/**
  set background color
*/
void BoardWidget::setBackgroundColor(const QColor& color){
    scene->setBackgroundBrush( QBrush(backgroundColor = color) );
}

/**
  set tutor background color
*/
void BoardWidget::setTutorBackgroundColor(const QColor& color){
    tutorBackgroundColor = color;
    if (tutorMode != TutorMode::noTutor)
        scene->setBackgroundBrush( QBrush(tutorBackgroundColor) );
}

/**
  set white stone type
*/
void BoardWidget::setWhiteStoneType(Preference::ResourceType type){
    if (whiteStoneType == type)
        return;

    whiteStoneType = type;
    createStonePixmap();
    createBuffer(true);
}

/**
  set white stone color
*/
void BoardWidget::setWhiteStoneColor(const QColor& color){
    whiteStoneColor = color;
    if (whiteStoneType == Preference::color)
        createBuffer(true);
}

/**
  set white stone image
*/
void BoardWidget::setWhiteStoneImage(const QString& file){
    whiteStoneImage = file;
    if (whiteStoneType == Preference::file){
        createStonePixmap();
        createBuffer(true);
    }
}

/**
  set black stone type
*/
void BoardWidget::setBlackStoneType(Preference::ResourceType type){
    if (blackStoneType == type)
        return;

    blackStoneType = type;
    createStonePixmap();
    createBuffer(true);
}

/**
  set black stone color
*/
void BoardWidget::setBlackStoneColor(const QColor& color){
    blackStoneColor = color;
    if (blackStoneType == Preference::color)
        createBuffer(true);
}

/**
  set black stone image
*/
void BoardWidget::setBlackStoneImage(const QString& file){
    blackStoneImage = file;
    if (blackStoneType == Preference::file){
        createStonePixmap();
        createBuffer(true);
    }
}

/**
  set branch color
*/
void BoardWidget::setBranchColor(const QColor& color){
    branchColor = color;
    createBuffer(true);
}

/**
  set focus color
*/
void BoardWidget::setFocusColor(const QColor& color){
    focusColor = color;
    createBuffer(false);
}

/**
  set focus type
*/
void BoardWidget::setFocusType(int type){
    focusType = type;
    createBuffer(false);
}

/**
  set label type
*/
void BoardWidget::setLabelType(Preference::LabelType type){
    labelType = type;
}

/**
  set label font
*/
void BoardWidget::setLabelFont(const QString& fontName){
    labelFont = fontName;
    createBuffer(false);
}

/**
  set automatic replay interval
*/
void BoardWidget::setAutomaticReplayInterval(int interval){
    replayInterval = interval;
    if (replayTimer)
        replayTimer->setInterval(replayInterval);
}

/**
  set sound file
*/
void BoardWidget::setPlaySound(bool play){
    playSound = play;
}

/**
  set sound file
*/
void BoardWidget::setMoveSoundFile(const QString& file){
    moveSound->setFilePath(file);
}

/**
  rotate view
*/
void BoardWidget::setRotate(int rotate_){
    if (rotate_ < 0 || rotate_ > 3)
        return;
    rotate = rotate_;
    setItemsPosition();
}

/**
  flip view horizontally
*/
void BoardWidget::setFlipHorizontally(bool flip){
    flipHorizontally = flip;
    setItemsPosition();
}

/**
  flip view vertically
*/
void BoardWidget::setFlipVertically(bool flip){
    flipVertically = flip;
    setItemsPosition();
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

    addItem(currentNode, node);
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

/**
  Slot
  automatic replay
*/
void BoardWidget::on_automaticReplay_timeout(){
    forward();
}

/**
  Slot
  move enemy at tutor one side
*/
void BoardWidget::on_tutorOneSide_timeout(){
    QTimer* timer = static_cast<QTimer*>(sender());
    timer->stop();
    timer->deleteLater();

    forward();
    moveEnemy = false;
}
