#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QSound>
#include <QList>
#include "boardwidget.h"
#include "mainwindow.h"
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
    boardImage1(":/res/bg.png")
{
    m_ui->setupUi(this);

//    mediaObject = new Phonon::MediaObject(this);

    setCurrentNode(&goData.root);
}

BoardWidget::~BoardWidget()
{
    delete m_ui;
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
    paint(this);
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
void BoardWidget::onLButtonDown(QMouseEvent* e){
    int boardX = (e->x() - xlines[0] + boxSize / 2) / boxSize;
    int boardY = (e->y() - ylines[0] + boxSize / 2) / boxSize;

    if (boardX < 0 || boardX >= xsize || boardY < 0 || boardY >= ysize)
        return;

    int sgfX, sgfY;
    boardToSgfCoordinate(boardX, boardY, sgfX, sgfY);

    if (moveToClicked && board[boardY][boardX].node)
        setCurrentNode( board[boardY][boardX].node );
    else if (editMode == eAlternateMove)
        addStone(sgfX, sgfY, boardX, boardY);
    else
        addMark(sgfX, sgfY, boardX, boardY);
}

/**
*/
void BoardWidget::onRButtonDown(QMouseEvent*){
}

/**
*/
void BoardWidget::paint(QPaintDevice* paintDevice){
    QPainter p(paintDevice);
    p.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    width_  = paintDevice->width();
    height_ = paintDevice->height();

    QFont font;
    font.setPointSize(8);
    font.setStyleHint(QFont::SansSerif);
    font.setWeight(QFont::Normal);

    p.setFont(font);
    p.setPen(Qt::black);

    p.fillRect(0, 0, width_, height_, Qt::white);

    drawBoard(p);
    drawCoordinates(p);
    drawStones(p);
}

/**
*/
void BoardWidget::getData(go::fileBase& data){
    data.set(goData);
}

/**
*/
void BoardWidget::setData(const go::fileBase& data){
    data.get(goData);
    nodeList.clear();
    setCurrentNode();
}

void BoardWidget::setBoardSize(int xsize, int ysize){
    goData.clear();
    goData.root.xsize = xsize;
    goData.root.ysize = ysize;

    nodeList.clear();
    setCurrentNode();

    repaint();
}

void BoardWidget::rotateSgf(){
    rotateSgf(&goData.root);
    createBoardBuffer();
    setDirty(true);
    repaint();
}

void BoardWidget::flipSgfHorizontally(){
    flipSgf(&goData.root, goData.root.xsize, 0);
    createBoardBuffer();
    setDirty(true);
    repaint();
}

void BoardWidget::flipSgfVertically(){
    flipSgf(&goData.root, 0, goData.root.ysize);
    createBoardBuffer();
    setDirty(true);
    repaint();
}

void BoardWidget::rotateSgf(go::node* node){
    int tmpX = goData.root.ysize - node->position.y - 1;
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
        int tmpX = goData.root.ysize - iter->p.y - 1;
        int tmpY = iter->p.x;
        iter->p.x = tmpX;
        iter->p.y = tmpY;
        ++iter;
    }
}

void BoardWidget::rotateMarkSgf(go::markList& markList){
    go::markList::iterator iter = markList.begin();
    while (iter != markList.end()){
        int tmpX = goData.root.ysize - iter->p.y - 1;
        int tmpY = iter->p.x;
        iter->p.x = tmpX;
        iter->p.y = tmpY;
        ++iter;
    }
}

void BoardWidget::flipSgf(go::node* node, int xsize, int ysize){
    if (xsize)
        node->position.x = xsize - node->position.x - 1;
    if (ysize)
        node->position.y = ysize - node->position.y - 1;

    emit nodeModified(node);

    go::nodeList::iterator iter = node->childNodes.begin();
    while (iter != node->childNodes.end()){
        flipSgf(*iter, xsize, ysize);
        ++iter;
    }

    flipStoneSgf(node->stones, xsize, ysize);
    flipMarkSgf(node->crosses, xsize, ysize);
    flipMarkSgf(node->squares, xsize, ysize);
    flipMarkSgf(node->triangles, xsize, ysize);
    flipMarkSgf(node->circles, xsize, ysize);
    flipMarkSgf(node->characters, xsize, ysize);
    flipMarkSgf(node->blackTerritories, xsize, ysize);
    flipMarkSgf(node->whiteTerritories, xsize, ysize);
}

void BoardWidget::flipStoneSgf(go::stoneList& stoneList, int xsize, int ysize){
    go::stoneList::iterator iter = stoneList.begin();
    while (iter != stoneList.end()){
        if (xsize)
            iter->p.x = xsize - iter->p.x - 1;
        if (ysize)
            iter->p.y = ysize - iter->p.y - 1;
        ++iter;
    }
}

void BoardWidget::flipMarkSgf(go::markList& markList, int xsize, int ysize){
    go::markList::iterator iter = markList.begin();
    while (iter != markList.end()){
        if (xsize)
            iter->p.x = xsize - iter->p.x - 1;
        if (ysize)
            iter->p.y = ysize - iter->p.y - 1;
        ++iter;
    }
}

int  BoardWidget::rotateBoard(){
    if (++rotateBoard_ > 3)
        rotateBoard_ = 0;

    createBoardBuffer();
    repaint();

    return rotateBoard_;
}

void BoardWidget::flipBoardHorizontally(bool flip){
    flipBoardHorizontally_ = flip;

    createBoardBuffer();
    repaint();
}

void BoardWidget::flipBoardVertically(bool flip){
    flipBoardVertically_ = flip;

    createBoardBuffer();
    repaint();
}

void BoardWidget::resetBoard(){
    rotateBoard_ = 0;
    flipBoardHorizontally_ = false;
    flipBoardVertically_ = false;

    createBoardBuffer();
    repaint();
}

/**
*/
void BoardWidget::clear(){
    setDirty(false);
    goData.clear();
    nodeList.clear();
    setCurrentNode();
}

/**
*/
go::node* BoardWidget::findNodeFromMoveNumber(int moveNumber){
    go::nodeList::iterator iter = nodeList.begin();

    int number = 0;
    while (iter != nodeList.end()){
        if ((*iter)->isStone() && ++number == moveNumber)
            return *iter;
        ++iter;
    }

    return NULL;
}

/**
* public slot
*/
void BoardWidget::addNode(go::node* parent, go::node* node){
    parent->childNodes.push_back(node);
    node->parent = parent;
    setDirty(true);
    emit nodeAdded(parent, node);
}

/**
* public slot
*/
void BoardWidget::deleteNode(go::node* node){
    if( node == &goData.root )
        return;

    go::node* parent = node->parent;
    if (parent){
        go::nodeList::iterator iter = qFind(parent->childNodes.begin(), parent->childNodes.end(), node);
        if (iter != parent->childNodes.end())
            parent->childNodes.erase(iter);
    }

    setDirty(true);
    emit nodeDeleted(node);

    go::nodeList::iterator iter = qFind(nodeList.begin(), nodeList.end(), node);
    nodeList.erase(iter, nodeList.end());
    setCurrentNode(node->parent);

    delete node;
}

/**
* public slot
*/
void BoardWidget::modifyNode(go::node* node, bool recreateBoardBuffer){
    if (recreateBoardBuffer)
        createBoardBuffer();
    repaint();
    setDirty(true);
    emit nodeModified(node);
}

/**
* public slot
*/
void BoardWidget::setCurrentNode(go::node* node){
    if (node == NULL)
        node = &goData.root;

    if (currentNode == node && !nodeList.empty())
        return;

    currentNode  = node;
    go::nodeList::iterator iter = qFind(nodeList.begin(), nodeList.end(), node);
    if (iter == nodeList.end())
        createNodeList();

    createBoardBuffer();

    if (playSound && node->isStone())
        QSound::play(stoneSoundPath);

    repaint();
    emit currentNodeChanged(currentNode);
}

/**
*/
void BoardWidget::createNodeList(){
    nodeList.clear();

    go::node* node = currentNode;
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
    xsize = (rotateBoard_ == 0 || rotateBoard_ == 2) ? goData.root.xsize : goData.root.ysize;
    ysize = (rotateBoard_ == 0 || rotateBoard_ == 2) ? goData.root.ysize : goData.root.xsize;

    capturedBlack = 0;
    capturedWhite = 0;

    board.clear();
    board.resize(ysize);
    for (int i=0; i<ysize; ++i)
        board[i].resize(xsize);

    int moveNumber = 1;
    go::nodeList::iterator iter = nodeList.begin();
    while (iter != nodeList.end()){
        if ((*iter)->moveNumber > 0)
            moveNumber = (*iter)->moveNumber;
        putStone(*iter, moveNumber);
        if ((*iter)->isStone())
            ++moveNumber;

        if (*iter == currentNode)
            break;

        ++iter;
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
    if (boardRect.width() != boardImage2.width()){
        boardImage2 = QImage(boardRect.size(), QImage::Format_RGB32);
        QPainter p2(&boardImage2);
        p2.fillRect(0, 0, boardRect.width(), boardRect.height(), QBrush(boardImage1));
        black2 = black1.scaled(boxSize, boxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        white2 = white1.scaled(boxSize, boxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

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

    int m = int(boxSize * 2.2);

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

        QRect r = p.boundingRect(xlines[i]-m/2, ylines[0]-m, m, m, Qt::AlignCenter, s);
        p.drawText(r, s);
        r = p.boundingRect(xlines[i]-m/2, ylines[ysize-1], m, m, Qt::AlignCenter, s);
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

        QRect r = p.boundingRect(xlines[0]-m, ylines[i]-m/2, m, m, Qt::AlignCenter, s);
        p.drawText(r, s);
        r = p.boundingRect(xlines[xsize-1], ylines[i]-m/2, m, m, Qt::AlignCenter, s);
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
    drawTerritory(p, currentNode->blackTerritories.begin(), currentNode->blackTerritories.end());
    drawTerritory(p, currentNode->whiteTerritories.begin(), currentNode->whiteTerritories.end());

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
                p.drawImage(xlines[x]-boxSize/2, ylines[y]-boxSize/2, black2);
            else if (board[y][x].white())
                p.drawImage(xlines[x]-boxSize/2, ylines[y]-boxSize/2, white2);

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
void BoardWidget::drawTerritory(QPainter& p, go::markList::iterator first, go::markList::iterator last){
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

/**
*/
void BoardWidget::drawCurrentMark(QPainter& p, go::node* node){
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
void BoardWidget::eraseBackground(QPainter& p, int x, int y){
    int dx = xlines[x] - boxSize / 2;
    int dy = ylines[y] - boxSize / 2;
    p.drawImage(dx, dy, boardImage2, dx - boardRect.left(), dy - boardRect.top(), boxSize, boxSize);
}

/**
*/
void BoardWidget::putStone(go::node* node, int moveNumber){
    isBlack = node->isWhite();

    if (node->isStone()){
        int boardX, boardY;
        sgfToBoardCoordinate(node->getX(), node->getY(), boardX, boardY);

        if (boardX >= 0 && boardX < xsize && boardY >= 0 && boardY < ysize){
            board[boardY][boardX].color  = node->isBlack() ? go::black : go::white;
            board[boardY][boardX].number = moveNumber;
            board[boardY][boardX].node   = node;
            currentMoveNumber  = moveNumber;
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
    else if (board[y][x].color == 0)
        return false;
    else if (board[y][x].color != c)
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

    go::color c = board[y][x].color;
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
                board[y][x].node  = NULL;
            }
        }
    }
}

/**
*/
void BoardWidget::addStone(int sgfX, int sgfY, int boardX, int boardY){
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

    go::node* n;
    if (isBlack)
        n = go::createBlackNode(currentNode, sgfX, sgfY);
    else
        n = go::createWhiteNode(currentNode, sgfX, sgfY);

    addNode(currentNode, n);
    setCurrentNode(n);
}

/**
*/
void BoardWidget::addMark(int sgfX, int sgfY, int boardX, int boardY){
    switch (editMode){
        case eAlternateMove:
            return;

        case eAddBlack:
            addStone(currentNode->stones, go::point(sgfX, sgfY), go::point(boardX, boardY), go::black);
            break;

        case eAddWhite:
            addStone(currentNode->stones, go::point(sgfX, sgfY), go::point(boardX, boardY), go::white);
            break;

        case eAddEmpty:
            addStone(currentNode->stones, go::point(sgfX, sgfY), go::point(boardX, boardY), go::empty);
            break;

        case eLabelMark:{
            go::point p(sgfX, sgfY);
            removeMark(currentNode->circles, p);
            removeMark(currentNode->crosses, p);
            removeMark(currentNode->squares, p);
            removeMark(currentNode->triangles, p);
            addCharacter(currentNode->characters, p);
            break;
        }

        case eCircleMark:{
            go::mark mark(sgfX, sgfY, go::mark::eCircle);
            addMark(currentNode->circles, mark);
            removeMark(currentNode->crosses, mark.p);
            removeMark(currentNode->squares, mark.p);
            removeMark(currentNode->triangles, mark.p);
            removeMark(currentNode->characters, mark.p);
            break;
        }

        case eCrossMark:{
            go::mark mark(sgfX, sgfY, go::mark::eCross);
            removeMark(currentNode->circles, mark.p);
            addMark(currentNode->crosses, mark);
            removeMark(currentNode->squares, mark.p);
            removeMark(currentNode->triangles, mark.p);
            removeMark(currentNode->characters, mark.p);
            break;
        }

        case eSquareMark:{
            go::mark mark(sgfX, sgfY, go::mark::eSquare);
            removeMark(currentNode->circles, mark.p);
            removeMark(currentNode->crosses, mark.p);
            addMark(currentNode->squares, mark);
            removeMark(currentNode->triangles, mark.p);
            removeMark(currentNode->characters, mark.p);
            break;
        }

        case eTriangleMark:{
            go::mark mark(sgfX, sgfY, go::mark::eTriangle);
            removeMark(currentNode->circles, mark.p);
            removeMark(currentNode->crosses, mark.p);
            removeMark(currentNode->squares, mark.p);
            addMark(currentNode->triangles, mark);
            removeMark(currentNode->characters, mark.p);
            break;
        }

        case eDeleteMarker:{
            go::point p(sgfX, sgfY);
            removeMark(currentNode->circles, p);
            removeMark(currentNode->crosses, p);
            removeMark(currentNode->squares, p);
            removeMark(currentNode->triangles, p);
            break;
        }

        default:
            return;
    };

    modifyNode(currentNode);
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

void BoardWidget::addStone(go::stoneList& stoneList, const go::point& sp, const go::point& bp, go::color c){
    go::stoneList::iterator iter = stoneList.begin();
    while (iter != stoneList.end()){
        if (iter->p == sp) {
            go::stone stone = *iter;
            iter = stoneList.erase(iter);
            board[bp.y][bp.x].color = go::empty;
            board[bp.y][bp.x].number = 0;

            if (stone.c == c || c == go::empty)
                return;
            else
                break;
        }
        ++iter;
    }
    currentNode->stones.push_back( go::stone(sp, c) );
    board[bp.y][bp.x].color = c;
    board[bp.y][bp.x].number = 0;
}

QString BoardWidget::getXString(int x) const{
    int a = x % 25;
    if (showCoordinatesI == false && a > 7)
        ++a;

    return QString("%1").arg(QChar('A' + a));
}

QString  BoardWidget::getYString(int y) const{
    return QString("%1").arg(goData.root.ysize - y);
}

QString  BoardWidget::getXYString(int x, int y) const{
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
            sgfX = goData.root.xsize - sgfX - 1;
        else
            sgfY = goData.root.ysize - sgfY - 1;
    }

    if (flipBoardVertically_){
        if (rotateBoard_ == 0 || rotateBoard_ == 2)
            sgfY = goData.root.ysize - sgfY - 1;
        else
            sgfX = goData.root.xsize - sgfX - 1;
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
