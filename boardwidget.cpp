#include <QDebug>
#include <QSettings>
#include <QPainter>
#include <QMouseEvent>
#include <QSound>
#include <QList>
#include <math.h>
#include "appdef.h"
#include "boardwidget.h"
#include "mainwindow.h"
#include "command.h"
#include "ui_boardwidget.h"

BoardWidget::BoardWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::BoardWidget),
    dirty(false),
    capturedBlack(0),
    capturedWhite(0),
    isBlack(true),
    currentMoveNumber(0),
    showMoveNumber(true),
    showMoveNumberCount(0),
    showCoordinates(true),
    showCoordinatesI(false),
    showMarker(true),
    showBranchMoves(true),
    editMode(eAlternateMove),
    moveToClicked(false),
    rotateBoard_(0),
    flipBoardHorizontally_(false),
    flipBoardVertically_(false),
    playSound(false),
    black1(":/res/black_64.png"),
    white1(":/res/white_64.png"),
    boardImage1(":/res/bg.png"),
    stoneSound(this)
{
    m_ui->setupUi(this);

//    stoneSound = Phonon::createPlayer(Phonon::NotificationCategory);

    readSettings();

    setCurrentNode(goData.root);
}

BoardWidget::~BoardWidget()
{
    delete m_ui;
}

void BoardWidget::readSettings(){
    QSettings settings;

    boardType = settings.value("boardType").toInt();
    whiteType = settings.value("whiteType").toInt();
    blackType = settings.value("blackType").toInt();

    boardColor = settings.value("boardColor").value<QColor>();
    whiteColor = settings.value("whiteColor").value<QColor>();
    blackColor = settings.value("blackColor").value<QColor>();
}

/**
*/
void BoardWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

/**
*/
void BoardWidget::paintEvent(QPaintEvent* e){
    QWidget::paintEvent(e);

    QPainter p(this);
    p.drawPixmap(0, 0, width(), height(), offscreenBuffer3);
}

/**
*/
void BoardWidget::mouseReleaseEvent(QMouseEvent* e){
    QWidget::mouseReleaseEvent(e);

    if(e->button() & Qt::LeftButton)
        onLButtonDown(e);
    else if (e->button() & Qt::RightButton)
        onRButtonDown(e);
}

/**
*/
void BoardWidget::mouseMoveEvent(QMouseEvent* e){
    QWidget::mouseMoveEvent(e);

    if (editMode != this->eAlternateMove && editMode != this->eGtp)
        return;

    int bx = floor( double(e->x() - xlines[0] + boxSize / 2) / boxSize );
    int by = floor( double(e->y() - ylines[0] + boxSize / 2) / boxSize );

    offscreenBuffer3 = offscreenBuffer2.copy();
    QPainter p(&offscreenBuffer3);

    if (! (bx < 0 || bx >= xlines.size() || by < 0 || by >= ylines.size() || board[by][bx].color != go::empty) ){
        p.setOpacity(0.5);
        drawImage(p, bx, by, isBlack ? black2 : white2);
    }

    repaint();
}

/**
*/
void BoardWidget::wheelEvent(QWheelEvent* e){
    QWidget::wheelEvent(e);

    go::nodeList::iterator iter = qFind(nodeList.begin(), nodeList.end(), currentNode);

    if (e->delta() > 0){
        if (iter != nodeList.begin() && iter != nodeList.end())
            setCurrentNode( *--iter );
    }
    else{
        if (iter != nodeList.end() && ++iter != nodeList.end())
            setCurrentNode( *iter );
    }
}

void BoardWidget::resizeEvent(QResizeEvent* e){
    QWidget::resizeEvent(e);

    offscreenBuffer1 = QPixmap(e->size());
    repaintBoard();
}

/**
*/
void BoardWidget::onLButtonDown(QMouseEvent* e){
    int boardX = (e->x() - xlines[0] + boxSize / 2) / boxSize;
    int boardY = (e->y() - ylines[0] + boxSize / 2) / boxSize;

    if (boardX < 0 || boardX >= xsize || boardY < 0 || boardY >= ysize)
        return;

    int sgfX, sgfY;
    boardToSgfCoordinate(boardX, boardY, sgfX, sgfY);

    if (editMode == eGtp)
        gtpLButtonDown(sgfX, sgfY);
    else if (moveToClicked && board[boardY][boardX].node)
        setCurrentNode( board[boardY][boardX].node );
    else if (editMode == eAlternateMove)
        addStoneCommand(sgfX, sgfY, boardX, boardY);
    else if (editMode == eCountTerritory){
        addTerritory(boardX, boardY);
        int alive_b=0, alive_w=0, dead_b=0, dead_w=0, bt=0, wt=0;
        getCountTerritory(alive_b, alive_w, dead_b, dead_w, bt, wt);
        emit updateTerritory(alive_b, alive_w, dead_b, dead_w, capturedBlack, capturedWhite, bt, wt, goData.root->komi);
    }
    else
        addMark(sgfX, sgfY, boardX, boardY);
}

/**
*/
void BoardWidget::onRButtonDown(QMouseEvent*){
    undoStack.undo();
}

/**
*/
void BoardWidget::gtpLButtonDown(int sgfX, int sgfY){
    if (isBlack == isYourColorBlack)
        gtpPut(sgfX, sgfY);
}

/**
*/
void BoardWidget::repaintBoard(bool board, bool stones){
    if (offscreenBuffer1.isNull())
        return;

    if (board)
        paintBoard(&offscreenBuffer1);

    if (stones){
        offscreenBuffer2 = offscreenBuffer1.copy();
        paintStones(&offscreenBuffer2);
        paintTerritories(&offscreenBuffer2);
    }

    offscreenBuffer3 = offscreenBuffer2.copy();

    repaint();
}

/**
*/
void BoardWidget::paintBoard(QPaintDevice* pd){
    QPainter p(pd);
    p.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    width_  = pd->width();
    height_ = pd->height();

    QFont font;
    font.setPointSize(8);
    font.setStyleHint(QFont::SansSerif);
    font.setWeight(QFont::Normal);

    p.setFont(font);
    p.setPen(Qt::black);

    p.fillRect(0, 0, width_, height_, Qt::white);

    drawBoard(p);
    drawCoordinates(p);
}

/**
*/
void BoardWidget::paintStones(QPaintDevice* pd){
    QPainter p(pd);
    p.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    width_  = pd->width();
    height_ = pd->height();

    QFont font;
    font.setPointSize(8);
    font.setStyleHint(QFont::SansSerif);
    font.setWeight(QFont::Normal);

    p.setFont(font);
    p.setPen(Qt::black);

    drawStones(p);
}

/**
*/
void BoardWidget::paintTerritories(QPaintDevice* pd){
    QPainter p(pd);
    p.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    width_  = pd->width();
    height_ = pd->height();

    drawTerritories(p);
}

/**
*/
void BoardWidget::getData(go::fileBase& data){
    data.set(goData);
}

/**
*/
void BoardWidget::setData(const go::fileBase& data){
    clear();
    data.get(goData);
    nodeList.clear();
    setCurrentNode();
    repaintBoard();
}

void BoardWidget::setBoardSize(int xsize, int ysize){
    clear();
    goData.root->xsize = xsize;
    goData.root->ysize = ysize;
    createBoardBuffer();
    setCurrentNode();

    repaintBoard();
}

void BoardWidget::rotateSgf(){
    undoStack.beginMacro( tr("Rotate SGF") );

    rotateSgf(goData.root);

    undoStack.endMacro();

    createBoardBuffer();
    setDirty(true);
    repaintBoard();
}

void BoardWidget::flipSgfHorizontally(){
    FlipSGFHorizontallyCommand* command = new FlipSGFHorizontallyCommand(this);
    flipSgf(goData.root, goData.root->xsize, 0, command);
    undoStack.push(command);

    setDirty(true);
}

void BoardWidget::flipSgfVertically(){
    FlipSGFVerticallyCommand* command = new FlipSGFVerticallyCommand(this);
    flipSgf(goData.root, 0, goData.root->ysize, command);
    undoStack.push(command);

    setDirty(true);
}

void BoardWidget::rotateSgf(go::nodePtr node){
    int tmpX = goData.root->ysize - node->position.y - 1;
    int tmpY = node->position.x;
    node->position.x = tmpX;
    node->position.y = tmpY;

    emit nodeModified(node);

    go::nodeList::iterator iter = node->childNodes.begin();
    while (iter != node->childNodes.end()){
        rotateSgf(*iter);
        ++iter;
    }

    rotateStoneSgf(node->stones);
    rotateMarkSgf(node->crosses);
    rotateMarkSgf(node->squares);
    rotateMarkSgf(node->triangles);
    rotateMarkSgf(node->circles);
    rotateMarkSgf(node->characters);
    rotateMarkSgf(node->blackTerritories);
    rotateMarkSgf(node->whiteTerritories);
}

void BoardWidget::rotateStoneSgf(go::stoneList& stoneList){
    go::stoneList::iterator iter = stoneList.begin();
    while (iter != stoneList.end()){
        int tmpX = goData.root->ysize - iter->p.y - 1;
        int tmpY = iter->p.x;
        iter->p.x = tmpX;
        iter->p.y = tmpY;
        ++iter;
    }
}

void BoardWidget::rotateMarkSgf(go::markList& markList){
    go::markList::iterator iter = markList.begin();
    while (iter != markList.end()){
        int tmpX = goData.root->ysize - iter->p.y - 1;
        int tmpY = iter->p.x;
        iter->p.x = tmpX;
        iter->p.y = tmpY;
        ++iter;
    }
}

void BoardWidget::flipSgf(go::nodePtr node, int xsize, int ysize, QUndoCommand* command){
    go::point p = node->position;
    if (xsize)
        p.x = xsize - p.x - 1;
    if (ysize)
        p.y = ysize - p.y - 1;
    new MovePositionCommand(this, node, p, command);

    go::nodeList::iterator iter = node->childNodes.begin();
    while (iter != node->childNodes.end()){
        flipSgf(*iter, xsize, ysize, command);
        ++iter;
    }

    flipStoneSgf(node, node->stones, xsize, ysize, command);
    flipMarkSgf(node, node->crosses, xsize, ysize, command);
    flipMarkSgf(node, node->squares, xsize, ysize, command);
    flipMarkSgf(node, node->triangles, xsize, ysize, command);
    flipMarkSgf(node, node->circles, xsize, ysize, command);
    flipMarkSgf(node, node->characters, xsize, ysize, command);
    flipMarkSgf(node, node->blackTerritories, xsize, ysize, command);
    flipMarkSgf(node, node->whiteTerritories, xsize, ysize, command);
}

void BoardWidget::flipStoneSgf(go::nodePtr node, go::stoneList& stoneList, int xsize, int ysize, QUndoCommand* command){
    go::stoneList::iterator iter = stoneList.begin();
    while (iter != stoneList.end()){
        go::point pos = iter->p;
        if (xsize)
            pos.x = xsize - pos.x - 1;
        if (ysize)
            pos.y = ysize - pos.y - 1;

        new MoveStoneCommand(this, node, &(*iter), pos, command);

        ++iter;
    }
}

void BoardWidget::flipMarkSgf(go::nodePtr node, go::markList& markList, int xsize, int ysize, QUndoCommand* command){
    go::markList::iterator iter = markList.begin();
    while (iter != markList.end()){
        go::point pos = iter->p;
        if (xsize)
            pos.x = xsize - pos.x - 1;
        if (ysize)
            pos.y = ysize - pos.y - 1;

        new MoveMarkCommand(this, node, &(*iter), pos, command);

        ++iter;
    }
}

int  BoardWidget::rotateBoard(){
    if (++rotateBoard_ > 3)
        rotateBoard_ = 0;

    createBoardBuffer();
    repaintBoard();

    return rotateBoard_;
}

void BoardWidget::flipBoardHorizontally(bool flip){
    flipBoardHorizontally_ = flip;

    createBoardBuffer();
    repaintBoard();
}

void BoardWidget::flipBoardVertically(bool flip){
    flipBoardVertically_ = flip;

    createBoardBuffer();
    repaintBoard();
}

void BoardWidget::resetBoard(){
    rotateBoard_ = 0;
    flipBoardHorizontally_ = false;
    flipBoardVertically_ = false;

    createBoardBuffer();
    repaintBoard();
}

/**
*/
void BoardWidget::clear(){
    setDirty(false);
    goData.clear();
    nodeList.clear();
    capturedBlack = 0;
    capturedWhite = 0;
    setCurrentNode();
    repaintBoard();
    undoStack.clear();
}

/**
*/
go::nodePtr BoardWidget::findNodeFromMoveNumber(int moveNumber){
    go::nodeList::iterator iter = nodeList.begin();

    int number = 0;
    while (iter != nodeList.end()){
        if ((*iter)->isStone() && ++number == moveNumber)
            return *iter;
        ++iter;
    }

    return go::nodePtr();
}

/**
* public slot
*/
void BoardWidget::addNodeCommand(go::nodePtr parentNode, go::nodePtr childNode, bool select){
    undoStack.push( new AddNodeCommand(this, parentNode, childNode, select) );
}

/**
* public slot
*/
void BoardWidget::insertNodeCommand(go::nodePtr parentNode, go::nodePtr childNode, bool select){
    undoStack.push( new InsertNodeCommand(this, parentNode, childNode, select) );
}

/**
* public slot
*/
void BoardWidget::deleteNodeCommand(go::nodePtr node, bool deleteChildren){
    if( node == goData.root )
        return;

    undoStack.push( new DeleteNodeCommand(this, node, deleteChildren) );
}

/**
* public slot
*/
void BoardWidget::setMoveNumberCommand(go::nodePtr node, int moveNumber){
    if( node->moveNumber == moveNumber )
        return;

    undoStack.push( new SetMoveNumberCommand(this, node, moveNumber) );
}

/**
* public slot
*/
void BoardWidget::unsetMoveNumberCommand(go::nodePtr node){
    if( node->moveNumber == -1 )
        return;

    undoStack.push( new UnsetMoveNumberCommand(this, node) );
}

/**
* public slot
*/
void BoardWidget::setNodeNameCommand(go::nodePtr node, const QString& nodeName){
    if( node->name == nodeName)
        return;

    undoStack.push( new SetNodeNameCommand(this, node, nodeName) );
}

/**
* public slot
*/
void BoardWidget::setCommentCommand(go::nodePtr node, const QString& comment){
    if( node->comment == comment)
        return;

//    undoStack.push( new SetCommentCommand(this, node, comment) );
    node->comment = comment;
    modifyNode(node);
}

/**
* public slot
*/
void BoardWidget::addNode(go::nodePtr parent, go::nodePtr node, bool select){
    parent->childNodes.push_back(node);
    node->parent = parent;

    setDirty(true);
    emit nodeAdded(parent, node, select);

    if (select)
        setCurrentNode(node);
}

/**
* public slot
*/
void BoardWidget::deleteNode(go::nodePtr node, bool deleteChildren){
    if( node == goData.root )
        return;

    go::nodePtr parent = node->parent;
    if (parent){
        go::nodeList::iterator iter = qFind(parent->childNodes.begin(), parent->childNodes.end(), node);
        if (iter != parent->childNodes.end())
            parent->childNodes.erase(iter);

        if (deleteChildren == false){
            parent->childNodes += node->childNodes;
            go::nodeList::iterator iter = node->childNodes.begin();
            while (iter != node->childNodes.end()){
                (*iter)->parent = parent;
                ++iter;
            }
            node->childNodes.clear();
        }
    }

    setCurrentNode(parent);

    setDirty(true);
    emit nodeDeleted(node, deleteChildren);

    go::nodeList::iterator iter = qFind(nodeList.begin(), nodeList.end(), node);
    if (deleteChildren)
        nodeList.erase(iter, nodeList.end());
    else
        nodeList.erase(iter);
}

/**
* public slot
*/
void BoardWidget::modifyNode(go::nodePtr node, bool recreateBoardBuffer){
    if (recreateBoardBuffer)
        createBoardBuffer();
    repaintBoard(false);
    setDirty(true);
    emit nodeModified(node);
}

/**
* public slot
*/
void BoardWidget::pass(){
    if (editMode == eGtp)
        gtpPut(-1, -1);
    else
        addStoneCommand(-1, -1);
}

/**
* public slot
*/
void BoardWidget::setCurrentNode(go::nodePtr node){
    if (editMode == eCountTerritory)
        return;

    if (node == NULL)
        node = goData.root;

    if (currentNode == node && !nodeList.empty())
        return;

    currentNode  = node;
    go::nodeList::iterator iter = qFind(nodeList.begin(), nodeList.end(), node);
    if (iter == nodeList.end())
        createNodeList();

    createBoardBuffer();

    if (playSound && node->isStone())
        stoneSound.play();

    repaintBoard(false);
    emit currentNodeChanged(currentNode);
}

/**
*/
void BoardWidget::createNodeList(){
    nodeList.clear();

    go::nodePtr node = currentNode;
    while ((node = node->parent) != NULL)
        nodeList.push_front(node);

    nodeList.push_back( node = currentNode );
    while (!node->childNodes.empty()){
        node = node->childNodes.front();
        nodeList.push_back(node);
    }
}

/**
*/
void BoardWidget::createBoardBuffer(){
    xsize = (rotateBoard_ == 0 || rotateBoard_ == 2) ? goData.root->xsize : goData.root->ysize;
    ysize = (rotateBoard_ == 0 || rotateBoard_ == 2) ? goData.root->ysize : goData.root->xsize;

    capturedBlack = 0;
    capturedWhite = 0;

    board.clear();
    board.resize(ysize);
    for (int i=0; i<ysize; ++i)
        board[i].resize(xsize);

    currentMoveNumber = 0;
    go::nodeList::iterator iter = nodeList.begin();
    while (iter != nodeList.end()){
        if ((*iter)->moveNumber > 0)
            currentMoveNumber = (*iter)->moveNumber;
        else if ((*iter)->isStone())
            ++currentMoveNumber;
        putStone(*iter, currentMoveNumber);

        if (*iter == currentNode)
            break;

        ++iter;
    }

    go::markList::iterator iter2 = currentNode->blackTerritories.begin();
    while (iter2 != currentNode->blackTerritories.end()){
        int boardX, boardY;
        sgfToBoardCoordinate(iter2->p.x, iter2->p.y, boardX, boardY);
        if (boardX >= 0 && boardX < xsize && boardY >= 0 && boardY < ysize){
            board[boardY][boardX].color |= go::blackTerritory;
        }
        ++iter2;
    }

    iter2 = currentNode->whiteTerritories.begin();
    while (iter2 != currentNode->whiteTerritories.end()){
        int boardX, boardY;
        sgfToBoardCoordinate(iter2->p.x, iter2->p.y, boardX, boardY);
        if (boardX >= 0 && boardX < xsize && boardY >= 0 && boardY < ysize){
            board[boardY][boardX].color |= go::whiteTerritory;
        }
        ++iter2;
    }
}

/**
*/
void BoardWidget::drawBoard(QPainter& p){
    p.save();

    int w = width_  / (xsize + (showCoordinates ? 2 : 0));
    int h = height_ / (ysize + (showCoordinates ? 2 : 0));
    boxSize = qMin(w, h);
    w = boxSize * (xsize - 1);
    h = boxSize * (ysize - 1);
    int margin = int(boxSize * 0.6);

    int l = (width_ - w) / 2;
    int r = l + boxSize * (xsize - 1);
    int t = (height_ - h) / 2;
    int b = t + boxSize * (ysize - 1);

    // create board and stone image
    boardRect.setRect(l - margin, t - margin, w + margin * 2, h + margin * 2);
//    if (boardRect.width() != boardImage2.width()){
        boardImage2 = QImage(boardRect.size(), QImage::Format_RGB32);
        QPainter board(&boardImage2);
        if (boardType == 0)
            board.fillRect(0, 0, boardRect.width(), boardRect.height(), QBrush(boardImage1));
        else
            board.fillRect(0, 0, boardRect.width(), boardRect.height(), boardColor);

        if (blackType == 0)
            black2 = black1.scaled(boxSize, boxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        else{
            black2 = QImage(boxSize, boxSize, QImage::Format_ARGB32);
            black2.fill(0);
            QPainter p2(&black2);
            p2.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);
            p2.setPen(Qt::black);
            p2.setBrush(blackColor);
            p2.drawEllipse(1, 1, boxSize-2, boxSize-2);
        }

        if (whiteType == 0)
            white2 = white1.scaled(boxSize, boxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        else{
            white2 = QImage(boxSize, boxSize, QImage::Format_ARGB32);
            white2.fill(0);
            QPainter p2(&white2);
            p2.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);
            p2.setPen(Qt::black);
            p2.setBrush(whiteColor);
            p2.drawEllipse(1, 1, boxSize-2, boxSize-2);
        }
//    }

    p.fillRect(boardRect.left()+3, boardRect.top()+3, boardRect.width(), boardRect.height(), Qt::gray);
    p.drawImage(boardRect.topLeft(), boardImage2);

    // 横線を引く
    ylines.clear();
    for (int i=0; i<ysize; ++i){
        int y = t + i * boxSize;
        p.drawLine(l, y, r, y);
        ylines.push_back(y);
    }

    // 縦線を引く
    xlines.clear();
    for (int i=0; i<xsize; ++i){
        int x = l + i * boxSize;
        p.drawLine(x, t, x, b);
        xlines.push_back(x);
    }

    // 星に黒丸を描画する
    QList<int> xstar, ystar;
    getStartPosition(xstar, xsize);
    getStartPosition(ystar, ysize);
    for (int y=0; y<ystar.size(); ++y){
        for (int x=0; x<xstar.size(); ++x){
            int cx = xlines[ xstar[x] ];
            int cy = ylines[ ystar[y] ];

            QPainterPath path;
            path.addEllipse(cx-3, cy-3, 6, 6);
            p.fillPath(path, QBrush(Qt::black));
        }
    }

    p.restore();
}

void BoardWidget::getStartPosition(QList<int>& star, int size){
    if (size >= 7 && size <= 9){
        star.push_back(2);
        star.push_back(size-3);
    }
    else if (size > 9){
        star.push_back(3);
        star.push_back(size-4);
        if (size % 2)
            star.push_back(size / 2);
    }
}

/**
*/
void BoardWidget::drawCoordinates(QPainter& p){
    if (showCoordinates == false)
        return;

    p.save();

    int ps = p.font().pointSize();

    for (int i=0; i<xsize; ++i){
        QString s;
        if (rotateBoard_ == 0)
            s = getXString(flipBoardHorizontally_ ? xsize-i-1 : i);
        else if (rotateBoard_ == 1)
            s = getYString(flipBoardHorizontally_ ? i : xsize-i-1);
        else if (rotateBoard_ == 2)
            s = getXString(flipBoardHorizontally_ ? i : xsize-i-1);
        else if (rotateBoard_ == 3)
            s = getYString(flipBoardHorizontally_ ? xsize-i-1 : i);

        QRect r = p.boundingRect(xlines[i]-ps, boardRect.top()-ps-2, ps*2, ps, Qt::AlignCenter, s);
        p.drawText(r, s);
        r = p.boundingRect(xlines[i]-ps, boardRect.bottom()+7, ps*2, ps, Qt::AlignCenter, s);
        p.drawText(r, s);
    }

    for (int i=0; i<ysize; ++i){
        QString s;
        if (rotateBoard_ == 0)
            s = getYString(flipBoardVertically_ ? ysize-i-1 : i);
        else if (rotateBoard_ == 1)
            s = getXString(flipBoardVertically_ ? ysize-i-1 : i);
        else if (rotateBoard_ == 2)
            s = getYString(flipBoardVertically_ ? i : ysize-i-1);
        else if (rotateBoard_ == 3)
            s = getXString(flipBoardVertically_ ? i : ysize-i-1);

        QRect r = p.boundingRect(boardRect.left()-ps-3, ylines[i]-ps, ps, ps*2, Qt::AlignCenter, s);
        p.drawText(r, s);
        r = p.boundingRect(boardRect.right()+7, ylines[i]-ps, ps, ps*2, Qt::AlignCenter, s);
        p.drawText(r, s);
    }

    p.restore();
}

/**
*/
void BoardWidget::drawStones(QPainter& p){
    p.save();

    QFont font(p.font());
    font.setPointSize(int(boxSize * 0.5));
    p.setFont(font);

    drawStones2(p);

    if (currentNode->childNodes.size() > 1)
        drawBranchMoves(p, currentNode->childNodes.begin(), currentNode->childNodes.end());

    drawMark(p, currentNode->crosses.begin(), currentNode->crosses.end());
    drawMark(p, currentNode->triangles.begin(), currentNode->triangles.end());
    drawMark(p, currentNode->circles.begin(), currentNode->circles.end());
    drawMark(p, currentNode->squares.begin(), currentNode->squares.end());
    drawMark(p, currentNode->characters.begin(), currentNode->characters.end());
//    drawTerritories(p, currentNode->blackTerritories.begin(), currentNode->blackTerritories.end());
//    drawTerritories(p, currentNode->whiteTerritories.begin(), currentNode->whiteTerritories.end());

    if (showMoveNumber && showMoveNumberCount == 0)
        if (currentNode->isStone())
            drawCurrentMark(p, currentNode);

    p.restore();
}

void BoardWidget::drawStones2(QPainter& p){
    p.save();

    QFont font( p.font() );

    for (int y=0; y<board.size(); ++y){
        for (int x=0; x<board[y].size(); ++x){
            if (board[y][x].empty())
                continue;

            // draw stone
            if (board[y][x].black())
                drawImage(p, x, y, black2);
            else if (board[y][x].white())
                drawImage(p, x, y, white2);

            // draw move number
            if (showMoveNumber == false || showMoveNumberCount == 0 || board[y][x].number == 0 || (showMoveNumberCount != -1 && currentMoveNumber - showMoveNumberCount + 1 > board[y][x].number))
                continue;

            if (board[y][x].number < 10)
                font.setPointSize(int(boxSize * 0.41));
            else if (board[y][x].number < 99)
                font.setPointSize(int(boxSize * 0.38));
            else
                font.setPointSize(int(boxSize * 0.35));

            font.setWeight(board[y][x].number == currentMoveNumber ? QFont::Black: QFont::Normal);
            p.setFont(font);

            QString s = QString("%1").arg(board[y][x].number);
            p.setPen( board[y][x].number == currentMoveNumber ? Qt::red : board[y][x].black() ? Qt::white : Qt::black );
            p.drawText(xlines[x] - boxSize, ylines[y] - boxSize, boxSize * 2, boxSize * 2, Qt::AlignCenter, s);
        }
    }

    p.restore();
}

/**
*/
void BoardWidget::drawBranchMoves(QPainter& p, go::nodeList::iterator first, go::nodeList::iterator last){
    if (showBranchMoves == false)
        return;

    char s[] = "A";
    while (first != last){
        int boardX, boardY;
        sgfToBoardCoordinate((*first)->getX(), (*first)->getY(), boardX, boardY);
        if (boardX >= 0 && boardX < xsize && boardY >= 0 && boardY < ysize){
            eraseBackground(p, boardX, boardY);
            p.drawText(xlines[boardX] - boxSize, ylines[boardY] - boxSize, boxSize * 2, boxSize * 2, Qt::AlignCenter, s);
            ++s[0];
        }
        ++first;
    }
}

/**
*/
void BoardWidget::drawMark(QPainter& p, go::markList::iterator first, go::markList::iterator last){
    if (showMarker == false)
        return;

    p.save();

    QFont font( p.font() );
    font.setWeight(QFont::Black);
    p.setFont(font);

    while (first != last){
        int boardX, boardY;
        sgfToBoardCoordinate(first->p.x, first->p.y, boardX, boardY);

        if (boardX >= 0 && boardX < xsize && boardY >= 0 && boardY < ysize){
            int x = xlines[boardX];
            int y = ylines[boardY];

            stoneInfo& info = board[boardY][boardX];
            if (info.empty()){
                p.setPen( Qt::black );
                eraseBackground(p, boardX, boardY);
            }
            else
                p.setPen( info.black() ? Qt::white : Qt::black );

            p.drawText(x-boxSize, y-boxSize, boxSize*2, boxSize*2, Qt::AlignCenter, first->s);
        }
        ++first;
    }

    p.restore();
}

/**
*/
void BoardWidget::drawTerritories(QPainter& p){
    p.save();

    QFont font( p.font() );
    font.setPointSize(int(boxSize * 0.38));
    p.setFont(font);

    for (int y=0; y<ysize; ++y){
        for (int x=0; x<xsize; ++x){
            if (!board[y][x].blackTerritory() && !board[y][x].whiteTerritory())
                continue;

            int bx = xlines[x];
            int by = ylines[y];

            QColor color = board[y][x].whiteTerritory() ? QColor(255, 255, 255, 110) : QColor(0, 0, 0, 80);
            p.fillRect(bx-boxSize/2, by-boxSize/2, boxSize, boxSize, color);
            p.setPen( board[y][x].whiteTerritory() ? Qt::white : Qt::black );
            p.drawText(bx-boxSize, by-boxSize, boxSize*2, boxSize*2, Qt::AlignCenter, "■");
        }
    }

    p.restore();
}

/**
*/
/*
void BoardWidget::drawTerritories(QPainter& p, go::markList::iterator first, go::markList::iterator last){
    p.save();

    QFont font( p.font() );
    font.setPointSize(int(boxSize * 0.38));
    p.setFont(font);

    while (first != last){
        int boardX, boardY;
        sgfToBoardCoordinate(first->p.x, first->p.y, boardX, boardY);

        if (boardX >= 0 && boardX < xsize && boardY >= 0 && boardY < ysize){
            int x = xlines[boardX];
            int y = ylines[boardY];

            QColor color = first->t == go::mark::eWhiteTerritory ? QColor(255, 255, 255, 160) : QColor(0, 0, 0, 110);
            p.fillRect(x-boxSize/2, y-boxSize/2, boxSize, boxSize, color);
            p.setPen( first->t == go::mark::eWhiteTerritory ? Qt::white : Qt::black );
            p.drawText(x-boxSize, y-boxSize, boxSize*2, boxSize*2, Qt::AlignCenter, first->s);
        }
        ++first;
    }

    p.restore();
}
*/

/**
*/
void BoardWidget::drawCurrentMark(QPainter& p, go::nodePtr node){
    p.save();

    QFont font(p.font());
    font.setPointSize(int(boxSize * 0.45));
    p.setFont(font);
    p.setPen(Qt::red);
    if (!node->isPass()){
        int boardX, boardY;
        sgfToBoardCoordinate(node->getX(), node->getY(), boardX, boardY);
        int x = xlines[boardX];
        int y = ylines[boardY];
        p.drawText(x-boxSize, y-boxSize, boxSize*2, boxSize*2, Qt::AlignCenter, "▲");
    }

    p.restore();
}

/**
*/
void BoardWidget::drawImage(QPainter& p, int x, int y, const QImage& image){
    p.drawImage(xlines[x]-boxSize/2, ylines[y]-boxSize/2, image);
}

/**
*/
void BoardWidget::eraseBackground(QPainter& p, int x, int y){
    int dx = xlines[x] - boxSize / 2;
    int dy = ylines[y] - boxSize / 2;
    p.drawImage(dx, dy, boardImage2, dx - boardRect.left(), dy - boardRect.top(), boxSize, boxSize);
}

/**
*/
void BoardWidget::putStone(go::nodePtr node, int moveNumber){
    isBlack = node->isWhite();

    if (node->isStone()){
        int boardX, boardY;
        sgfToBoardCoordinate(node->getX(), node->getY(), boardX, boardY);

        if (boardX >= 0 && boardX < xsize && boardY >= 0 && boardY < ysize){
            board[boardY][boardX].color  = node->isBlack() ? go::black : go::white;
            board[boardY][boardX].number = moveNumber;
            board[boardY][boardX].node   = node;
            removeDeadStones(boardX, boardY);
        }
    }

    go::stoneList::iterator iter = node->stones.begin();
    while (iter != node->stones.end()){
        int boardX, boardY;
        sgfToBoardCoordinate(iter->p.x, iter->p.y, boardX, boardY);
        if (boardX >= 0 && boardX < xsize && boardY >= 0 && boardY < ysize){
            board[boardY][boardX].color  = iter->c;
            board[boardY][boardX].number = 0;
            board[boardY][boardX].node   = node;
            removeDeadStones(boardX, boardY);
        }
        ++iter;
    }
}

/**
*/
void BoardWidget::removeDeadStones(int x, int y){
    int c = board[y][x].color == go::black ? go::white : go::black;

    int* tmp = new int[xsize * ysize];

    memset(tmp, 0, sizeof(int) * xsize * ysize);
    if (y > 0 && board[y-1][x].color == c && isDead(tmp, c, x, y - 1))
        dead(tmp);

    memset(tmp, 0, sizeof(int) * xsize * ysize);
    if (y < ysize-1 && board[y+1][x].color == c && isDead(tmp, c, x, y + 1))
        dead(tmp);

    memset(tmp, 0, sizeof(int) * xsize * ysize);
    if (x > 0 && board[y][x-1].color == c && isDead(tmp, c, x - 1, y))
        dead(tmp);

    memset(tmp, 0, sizeof(int) * xsize * ysize);
    if (x < xsize-1 && board[y][x+1].color == c && isDead(tmp, c, x + 1, y))
        dead(tmp);

    delete[] tmp;
}

/**
*/
bool BoardWidget::isDead(int* tmp, int c, int x, int y){
    if (tmp[y*xsize+x])
        return true;
    else if (board[y][x].empty())
        return false;
    else if ((board[y][x].color & c) == 0)
        return true;
    tmp[y*xsize+x] = board[y][x].color;

    if (y > 0 && !isDead(tmp, c, x, y - 1))
        return false;

    if (y < ysize-1 && !isDead(tmp, c, x, y + 1))
        return false;

    if (x > 0 && !isDead(tmp, c, x - 1, y))
        return false;

    if (x < xsize-1 && !isDead(tmp, c, x + 1, y))
        return false;

    return true;
}

/**
*/
bool BoardWidget::isDead(int x, int y){
    int size = xsize * ysize;
    int* tmp = new int[size];
    memset(tmp, 0, sizeof(int)*size);

    int c = board[y][x].color;
    bool dead = isDead(tmp, c, x, y);

    delete[] tmp;

    return dead;
}

/**
*/
bool BoardWidget::isKill(int x, int y){
    int* tmp = new int[xsize * ysize];

    go::color c = board[y][x].color == go::black ? go::white : go::black;

    memset(tmp, 0, sizeof(int)* xsize * ysize);
    bool dead = (y > 0 && board[y-1][x].color == c && isDead(tmp, c, x, y - 1));

    memset(tmp, 0, sizeof(int)* xsize * ysize);
    dead = !dead ? (y < ysize-1 && board[y+1][x].color == c && isDead(tmp, c, x, y + 1)) : true;

    memset(tmp, 0, sizeof(int)* xsize * ysize);
    dead = !dead ? (x > 0 && board[y][x-1].color == c && isDead(tmp, c, x - 1, y)) : true;

    memset(tmp, 0, sizeof(int)* xsize * ysize);
    dead = !dead ? (x < xsize-1 && board[y][x+1].color == c && isDead(tmp, c, x + 1, y)) : true;

    delete[] tmp;

    return dead;
}

/**
*/
void BoardWidget::dead(int* tmp){
    for (int y=0; y<ysize; ++y){
        for (int x=0; x<xsize; ++x){
            if (tmp[y*xsize+x]){
                if (board[y][x].color == go::black)
                    ++capturedBlack;

                if (board[y][x].color == go::white)
                    ++capturedWhite;

                board[y][x].color = go::empty;
                board[y][x].node.reset();
            }
        }
    }
}

/**
*/
void BoardWidget::addStoneCommand(int sgfX, int sgfY){
    if (sgfX == -1 && sgfY == -1){
        go::nodePtr node;
        if (isBlack)
            node = go::createBlackNode(currentNode);
        else
            node = go::createWhiteNode(currentNode);

        addNodeCommand(currentNode, node);
    }
    else{
        int boardX, boardY;
        sgfToBoardCoordinate(sgfX, sgfY, boardX, boardY);
        addStoneCommand(sgfX, sgfY, boardX, boardY);
    }
}

/**
*/
void BoardWidget::addStoneCommand(int sgfX, int sgfY, int boardX, int boardY){
    if (board[boardY][boardX].empty() == false)
        return;

    go::nodeList::iterator iter = currentNode->childNodes.begin();
    while (iter != currentNode->childNodes.end()){
        if ((*iter)->getX() == sgfX && (*iter)->getY() == sgfY){
            setCurrentNode(*iter);
            return;
        }
        ++iter;
    }

    board[boardY][boardX].color = isBlack ? go::black : go::white;
    if (isKill(boardX, boardY) == false && isDead(boardX, boardY) == true){
        board[boardY][boardX].color = go::empty;
        return;
    }

    go::nodePtr n;
    if (isBlack)
        n = go::createBlackNode(currentNode, sgfX, sgfY);
    else
        n = go::createWhiteNode(currentNode, sgfX, sgfY);

    addNodeCommand(currentNode, n);
}

/**
*/
void BoardWidget::addMark(int sgfX, int sgfY, int boardX, int boardY){
    switch (editMode){
        case eAlternateMove:
            return;

        case eAddBlack:
            addStone(currentNode, go::point(sgfX, sgfY), go::point(boardX, boardY), go::black);
            break;

        case eAddWhite:
            addStone(currentNode, go::point(sgfX, sgfY), go::point(boardX, boardY), go::white);
            break;

        case eAddEmpty:
            addStone(currentNode, go::point(sgfX, sgfY), go::point(boardX, boardY), go::empty);
            break;

        case eLabelMark:{
            go::point p(sgfX, sgfY);
            removeMark(currentNode->circles, p);
            removeMark(currentNode->crosses, p);
            removeMark(currentNode->squares, p);
            removeMark(currentNode->triangles, p);
            addCharacter(currentNode->characters, p);
            modifyNode(currentNode);
            break;
        }

        case eCircleMark:{
            go::mark mark(sgfX, sgfY, go::mark::eCircle);
            addMark(currentNode->circles, mark);
            removeMark(currentNode->crosses, mark.p);
            removeMark(currentNode->squares, mark.p);
            removeMark(currentNode->triangles, mark.p);
            removeMark(currentNode->characters, mark.p);
            modifyNode(currentNode);
            break;
        }

        case eCrossMark:{
            go::mark mark(sgfX, sgfY, go::mark::eCross);
            removeMark(currentNode->circles, mark.p);
            addMark(currentNode->crosses, mark);
            removeMark(currentNode->squares, mark.p);
            removeMark(currentNode->triangles, mark.p);
            removeMark(currentNode->characters, mark.p);
            modifyNode(currentNode);
            break;
        }

        case eSquareMark:{
            go::mark mark(sgfX, sgfY, go::mark::eSquare);
            removeMark(currentNode->circles, mark.p);
            removeMark(currentNode->crosses, mark.p);
            addMark(currentNode->squares, mark);
            removeMark(currentNode->triangles, mark.p);
            removeMark(currentNode->characters, mark.p);
            modifyNode(currentNode);
            break;
        }

        case eTriangleMark:{
            go::mark mark(sgfX, sgfY, go::mark::eTriangle);
            removeMark(currentNode->circles, mark.p);
            removeMark(currentNode->crosses, mark.p);
            removeMark(currentNode->squares, mark.p);
            addMark(currentNode->triangles, mark);
            removeMark(currentNode->characters, mark.p);
            modifyNode(currentNode);
            break;
        }

        case eDeleteMarker:{
            go::point p(sgfX, sgfY);
            removeMark(currentNode->circles, p);
            removeMark(currentNode->crosses, p);
            removeMark(currentNode->squares, p);
            removeMark(currentNode->triangles, p);
            modifyNode(currentNode);
            break;
        }

        default:
            return;
    };
}

void BoardWidget::addMark(go::markList& markList, const go::mark& mark){
    go::markList::iterator iter = markList.begin();
    while (iter != markList.end()){
        if (iter->p == mark.p){
            markList.erase(iter);
            return;
        }
        ++iter;
    }

    markList.push_back(mark);
}

void BoardWidget::addCharacter(go::markList& markList, const go::point& p){
    go::markList::iterator iter = markList.begin();
    while (iter != markList.end()){
        if (iter->p == p){
            markList.erase(iter);
            return;
        }

        ++iter;
    }

    char c = 'A';
    iter = markList.begin();
    while (iter != markList.end()){
        QString s = QChar(c);
        QString s2 = iter->s;
        if (s != s2)
            break;

        ++iter;
        ++c;
    }

    QString s = QChar(c);
    markList.push_back(go::mark(p, s));
}

void BoardWidget::removeMark(go::markList& markList, const go::point& p){
    go::markList::iterator iter = markList.begin();
    while (iter != markList.end()){
        if (iter->p == p){
            markList.erase(iter);
            return;
        }
        ++iter;
    }
}

void BoardWidget::addStone(go::nodePtr node, const go::point& sp, const go::point& bp, go::color c){
    go::nodePtr stoneNode( node->isStone() ? go::nodePtr(new go::node(node)) : node );
    go::stoneList::iterator iter = stoneNode->stones.begin();
    while (iter != stoneNode->stones.end()){
        if (iter->p == sp) {
            go::stone stone = *iter;
            iter = stoneNode->stones.erase(iter);
            board[bp.y][bp.x].color = go::empty;
            board[bp.y][bp.x].number = 0;

            if (stone.c == c || c == go::empty)
                return;
            else
                break;
        }
        ++iter;
    }

    stoneNode->stones.push_back( go::stone(sp, c) );
    board[bp.y][bp.x].color = c;
    board[bp.y][bp.x].number = 0;

    if (stoneNode == node)
        modifyNode(node);
    else
        insertNodeCommand(node, stoneNode);
}

QString BoardWidget::toString(go::nodePtr node) const{
    if (node->isPass())
        return "Pass";
    else
        return getXYString(node->position.x, node->position.y);
}

QString BoardWidget::getXString(int x) const{
    int a = x % 25;
    if (showCoordinatesI == false && a > 7)
        ++a;

    return QString("%1").arg(QChar('A' + a));
}

QString  BoardWidget::getYString(int y) const{
    return QString("%1").arg(goData.root->ysize - y);
}

QString  BoardWidget::getXYString(int x, int y) const{
    if (x < 0 || x >= goData.root->xsize || y < 0 || y >= goData.root->ysize)
        return "";

    QString s = getXString(x);
    s.append( getYString(y) );
    return s;
}

void BoardWidget::boardToSgfCoordinate(int boardX, int boardY, int& sgfX, int& sgfY){
    if (rotateBoard_ == 0){
        sgfX = boardX;
        sgfY = boardY;
    }
    else if (rotateBoard_ == 1){
        sgfX = boardY;
        sgfY = xsize - boardX - 1;
    }
    else if (rotateBoard_ == 2){
        sgfX = xsize - boardX - 1;
        sgfY = ysize - boardY - 1;
    }
    else{
        sgfX = ysize - boardY - 1;
        sgfY = boardX;
    }

    if (flipBoardHorizontally_){
        if (rotateBoard_ == 0 || rotateBoard_ == 2)
            sgfX = goData.root->xsize - sgfX - 1;
        else
            sgfY = goData.root->ysize - sgfY - 1;
    }

    if (flipBoardVertically_){
        if (rotateBoard_ == 0 || rotateBoard_ == 2)
            sgfY = goData.root->ysize - sgfY - 1;
        else
            sgfX = goData.root->xsize - sgfX - 1;
    }
}

void BoardWidget::sgfToBoardCoordinate(int sgfX, int sgfY, int& boardX, int& boardY){
    if (rotateBoard_ == 0){
        boardX = sgfX;
        boardY = sgfY;
    }
    else if (rotateBoard_ == 1){
        boardX = xsize - sgfY - 1;
        boardY = sgfX;
    }
    else if (rotateBoard_ == 2){
        boardX = xsize - sgfX - 1;
        boardY = ysize - sgfY - 1;
    }
    else{
        boardX = sgfY;
        boardY = ysize - sgfX - 1;
    }

    if (flipBoardHorizontally_)
        boardX = xsize - boardX - 1;

    if (flipBoardVertically_)
        boardY = ysize - boardY - 1;
}

void BoardWidget::setCountTerritoryMode(bool countMode){
    if (countMode){
        editMode = eCountTerritory;
        countTerritory();
        int alive_b=0, alive_w=0, dead_b=0, dead_w=0, bt=0, wt=0;
        getCountTerritory(alive_b, alive_w, dead_b, dead_w, bt, wt);
        emit updateTerritory(alive_b, alive_w, dead_b, dead_w, capturedBlack, capturedWhite, bt, wt, goData.root->komi);
    }
    else{
        editMode = eAlternateMove;
        createBoardBuffer();
    }

    repaintBoard();
}

void BoardWidget::countTerritory(){
    char* tmp = new char[xsize * ysize];

    for (int y=0; y<ysize; ++y){
        for (int x=0; x<xsize; ++x){
            memset(tmp, 0, xsize*ysize);
            int c = go::empty;
            whichTerritory(x, y, tmp, c);
            if (board[y][x].empty() && (c & go::blackTerritory || c & go::whiteTerritory))
                board[y][x].color |= c;
//            if (c != go::empty)
//                updateTerritory(x, y);
        }
    }

    delete[] tmp;
}

void BoardWidget::whichTerritory(int x, int y, char* tmp, int& c){
    if (tmp[y*xsize+x] != 0)
        return;
    tmp[y*xsize+x] = 1;

    if (c == go::dame)
        return;
    else if (board[y][x].whiteTerritory()){
        c = go::whiteTerritory;
        return;
    }
    else if (board[y][x].blackTerritory()){
        c = go::blackTerritory;
        return;
    }
    else if ((board[y][x].black() && c == go::whiteTerritory) || (board[y][x].white() && c == go::blackTerritory)){
        c = go::dame;
        return;
    }
    else if (board[y][x].black()){
        c = go::blackTerritory;
        return;
    }
    else if (board[y][x].white()){
        c = go::whiteTerritory;
        return;
    }

    if (y > 0)
        whichTerritory(x, y-1, tmp, c);
    if (y < ysize-1)
        whichTerritory(x, y+1, tmp, c);
    if (x > 0)
        whichTerritory(x-1, y, tmp, c);
    if (x < xsize-1)
        whichTerritory(x+1, y, tmp, c);
}

void BoardWidget::updateTerritory(int, int){
}

void BoardWidget::addTerritory(int x, int y){
    if (board[y][x].white() && !board[y][x].blackTerritory())
        setTerritory(x, y, go::blackTerritory);
    else if (board[y][x].black() && !board[y][x].whiteTerritory())
        setTerritory(x, y, go::whiteTerritory);
    else if ( (board[y][x].white() || board[y][x].black()) && (board[y][x].blackTerritory() || board[y][x].whiteTerritory()) )
        unsetTerritory(x, y);

    repaintBoard(false);
}

void BoardWidget::setTerritory(int x, int y, int c){
    if ( (c & go::blackTerritory && board[y][x].black()) || (c & go::whiteTerritory && board[y][x].white()) )
        return;
    else if(board[y][x].blackTerritory() || board[y][x].whiteTerritory())
        return;

    board[y][x].color |= c;

    if (y > 0)
        setTerritory(x, y-1, c);
    if (y < ysize-1)
        setTerritory(x, y+1, c);
    if (x > 0)
        setTerritory(x-1, y, c);
    if (x < xsize-1)
        setTerritory(x+1, y, c);
}

void BoardWidget::unsetTerritory(int x, int y){
    if ( !board[y][x].blackTerritory() && !board[y][x].whiteTerritory() )
        return;

    board[y][x].color &= go::black | go::white;

    if (y > 0)
        unsetTerritory(x, y-1);
    if (y < ysize-1)
        unsetTerritory(x, y+1);
    if (x > 0)
        unsetTerritory(x-1, y);
    if (x < xsize-1)
        unsetTerritory(x+1, y);
}

void BoardWidget::getCountTerritory(int& alive_b, int& alive_w, int& dead_b, int& dead_w, int& bt, int& wt){
    for (int y=0; y<ysize; ++y){
        for (int x=0; x<xsize; ++x){
            if (board[y][x].blackTerritory()){
                ++bt;
                if (board[y][x].white())
                    ++dead_w;
            }
            else if (board[y][x].whiteTerritory()){
                ++wt;
                if (board[y][x].black())
                    ++dead_b;
            }
            else if (board[y][x].black())
                ++alive_b;
            else if (board[y][x].white())
                ++alive_w;
        }
    }
}

void BoardWidget::playWithComputer(QProcess* proc, bool isYourColorBlack){
    this->isYourColorBlack = isYourColorBlack;
    comProcess = proc;
    if (comProcess){
        editMode = eGtp;
        moveToClicked = false;
        connect(comProcess, SIGNAL(readyRead()), this, SLOT(gtpReadReady()));

        if (isYourColorBlack == false){
            gtpStatus = eGtpGen;
            gtpWrite("genmove black\n");
        }
/*
        if (isYourColorBlack == false && handicap == 0)
            gtpWrite("genmove black\n");
        else if (isYourColorBlack && handicap > 0)
            gtpWrite("genmove white\n");
*/
    }
    else{
        editMode = eAlternateMove;
    }
}

void BoardWidget::gtpWrite(const QString& buf){
    if (comProcess == NULL)
        return;

    qDebug() << buf;
    QByteArray ba = buf.toAscii();
    comProcess->write( ba );
}

void BoardWidget::gtpPut(int x, int y){
    if (comProcess == NULL)
        return;

    gtpX = x;
    gtpY = y;

    QString xy;
    if (x == -1 && y == -1)
        xy = "PASS";
    else
        xy =getXYString(x, y);

    if (isBlack){
        gtpWrite( QString("black %1\n").arg(xy) );
        gtpStatus = eGtpPut;
    }
    else{
        gtpWrite( QString("white %1\n").arg(xy) );
        gtpStatus = eGtpPut;
    }
}

void BoardWidget::gtpReadReady(){
    if (comProcess == NULL)
        return;

    gtpBuf += comProcess->readAll();
    if (gtpBuf.size() < 3 || gtpBuf.right(2) != "\n\n")
        return;
//    if ((gtpBuf.size() < 5 || gtpBuf.right(4) != "\r\n\r\n") || (gtpBuf.size() < 3 || gtpBuf.right(2) != "\n\n"))
//        return;

    QString buf =gtpBuf.mid(2, gtpBuf.size()-4);
    gtpBuf.clear();
    qDebug() << buf;

    if (gtpStatus == eGtpPut){
        if (buf == "illegal move")
            return;
        addStoneCommand(gtpX, gtpY);

        if (isBlack)
            gtpWrite("genmove black\n");
        else
            gtpWrite("genmove white\n");
        gtpStatus = eGtpGen;
    }
    else if (gtpStatus == eGtpGen){
        int x, y;
        if (getCoordinate(buf, x, y)){
            addStoneCommand(x, y);
            gtpStatus = eGtpNone;
        }
    }
}

bool BoardWidget::getCoordinate(const QString& buf, int& x, int& y){
    if (buf.size() < 2)
        return false;

    if (buf == "PASS"){
        x = y = -1;
        return true;
    }

    x = buf[0].toAscii() - 'A';
    if (x > 7)
        --x;
    y = goData.root->ysize - buf.mid(1).toInt();

    return x >= 0 && x < goData.root->xsize && y >= 0 && y < goData.root->ysize;
}
