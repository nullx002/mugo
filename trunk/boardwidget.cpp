#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QList>
#include "boardwidget.h"
#include "mainwindow.h"
#include "ui_boardwidget.h"

BoardWidget::BoardWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::BoardWidget),
    dirty(false),
    size(19),
    black(true),
    currentMoveNumber(0),
    showMoveNumber(0),
    editMode(eAlternateMove),
    black1(":/res/black_64.png"),
    white1(":/res/white_64.png"),
    boardImage1(":/res/bg.png")
{
    m_ui->setupUi(this);

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
    int x = (e->x() - xlines[0] + boxSize / 2) / boxSize;
    int y = (e->y() - ylines[0] + boxSize / 2) / boxSize;

    if (x < 0 || x >= size || y < 0 || y >= size)
        return;

    if (editMode == eAlternateMove)
        addStone(x, y);
    else
        addMark(x, y);
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
    font.setStyleHint(QFont::TypeWriter);
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
}

/**
*/
void BoardWidget::clear(){
    goData.clear();
    setDirty(false);
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
    go::node* parent = node->parent;
    if (parent){
        go::nodeList::iterator iter = qFind(parent->childNodes.begin(), parent->childNodes.end(), node);
        if (iter != parent->childNodes.end())
            parent->childNodes.erase(iter);
    }
    setDirty(true);
    emit nodeDeleted(node);
    delete node;
}

/**
* public slot
*/
void BoardWidget::modifyNode(go::node* node){
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

    if (currentNode == node)
        return;

    currentNode = node;

    board.clear();
    board.resize(size);
    for (int i=0; i<size; ++i)
        board[i].resize(size);

    nodeList.clear();

    while ((node = node->parent) != NULL)
        nodeList.insert(nodeList.begin(), node);

    nodeList.push_back( node = currentNode );
    while (node->childNodes.size() == 1){
        node = node->childNodes.front();
        nodeList.push_back(node);
    }

    int moveNumber = 1;
    go::nodeList::iterator iter = nodeList.begin();
    while (iter != nodeList.end()){
        putStone(*iter, moveNumber);
        if (dynamic_cast<go::stoneNode*>(*iter))
            ++moveNumber;

        if (*iter == currentNode)
            break;

        ++iter;
    }

    repaint();
    emit currentNodeChanged(currentNode);
}

/**
*/
void BoardWidget::drawBoard(QPainter& p){
    p.save();

    int n = qMin(width_, height_);
    boxSize = n / (size + 2);
    int lineLength = boxSize * (size - 1);
    int margin = int(boxSize * 0.6);

    int l = (width_ - lineLength) / 2;
    int r = l + boxSize * (size - 1);
    int t = (height_ - lineLength) / 2;
    int b = t + boxSize * (size - 1);

    // create board and stone image
    boardRect.setRect(l - margin, t - margin, lineLength + margin * 2, lineLength + margin * 2);
    if (boardRect.width() != boardImage2.width()){
        boardImage2 = QImage(boardRect.size(), QImage::Format_RGB32);
        QPainter p2(&boardImage2);
        for (int y=0; y<boardRect.height(); y+=boardImage1.height())
            for (int x=0; x<boardRect.width(); x+=boardImage1.width())
                p2.drawImage(x, y, boardImage1);
        black2 = black1.scaled(boxSize, boxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        white2 = white1.scaled(boxSize, boxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    p.fillRect(boardRect.left()+3, boardRect.top()+3, boardRect.width(), boardRect.height(), Qt::gray);
    p.drawImage(boardRect.topLeft(), boardImage2);

    // 横線を引く
    ylines.clear();
    for (int i=0; i<size; ++i){
        int y = t + i * boxSize;
        p.drawLine(l, y, r, y);
        ylines.push_back(y);
    }

    // 縦線を引く
    xlines.clear();
    for (int i=0; i<size; ++i){
        int x = l + i * boxSize;
        p.drawLine(x, t, x, b);
        xlines.push_back(x);
    }

    // 星に黒丸を描画する
    QList<int> star;
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

    for (int y=0; y<star.size(); ++y){
        for (int x=0; x<star.size(); ++x){
            int cx = xlines[ star[x] ];
            int cy = ylines[ star[y] ];

            QPainterPath path;
            path.addEllipse(cx-3, cy-3, 6, 6);
            p.fillPath(path, QBrush(Qt::black));
        }
    }

    p.restore();
}

/**
*/
void BoardWidget::drawCoordinates(QPainter& p){
    p.save();

    int m = int(boxSize * 2.2);

    for (int i=0; i<size; ++i){
        QString s = getXString(i);
        QRect r = p.boundingRect(xlines[i]-m/2, ylines[0]-m, m, m, Qt::AlignCenter, s);
        p.drawText(r, s);
        r = p.boundingRect(xlines[i]-m/2, ylines[size-1], m, m, Qt::AlignCenter, s);
        p.drawText(r, s);
    }

    for (int i=0; i<size; ++i){
        QString s = getYString(i);
        QRect r = p.boundingRect(xlines[0]-m, ylines[i]-m/2, m, m, Qt::AlignCenter, s);
        p.drawText(r, s);
        r = p.boundingRect(xlines[size-1], ylines[i]-m/2, m, m, Qt::AlignCenter, s);
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
        drawNext(p, currentNode->childNodes.begin(), currentNode->childNodes.end());

    drawMark(p, currentNode->crosses.begin(), currentNode->crosses.end());
    drawMark(p, currentNode->triangles.begin(), currentNode->triangles.end());
    drawMark(p, currentNode->circles.begin(), currentNode->circles.end());
    drawMark(p, currentNode->squares.begin(), currentNode->squares.end());
    drawMark(p, currentNode->characters.begin(), currentNode->characters.end());
    drawTerritory(p, currentNode->blackTerritories.begin(), currentNode->blackTerritories.end());
    drawTerritory(p, currentNode->whiteTerritories.begin(), currentNode->whiteTerritories.end());

    if (showMoveNumber == 0){
        go::stoneNode* stoneNode = dynamic_cast<go::stoneNode*>(currentNode);
        if (stoneNode)
            drawCurrentMark(p, stoneNode);
    }

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
            if (board[y][x].number == 0 || showMoveNumber == 0 || (showMoveNumber != -1 && currentMoveNumber - showMoveNumber + 1 > board[y][x].number))
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
void BoardWidget::drawNext(QPainter& p, go::nodeList::iterator first, go::nodeList::iterator last){
    char s[] = "A";
    while (first != last){
        int x = (*first)->getX();
        int y = (*first)->getY();
        if (x >= 0 && x < size && y >= 0 && y < size){
            eraseBackground(p, x, y);
            p.drawText(xlines[x] - boxSize, ylines[y] - boxSize, boxSize * 2, boxSize * 2, Qt::AlignCenter, s);
            ++s[0];
        }
        ++first;
    }
}

/**
*/
void BoardWidget::drawMark(QPainter& p, go::markList::iterator first, go::markList::iterator last){
    p.save();

    QFont font( p.font() );
    font.setWeight(QFont::Black);
    p.setFont(font);

    while (first != last){
        if (first->p.x >= 0 && first->p.x < size && first->p.y >= 0 && first->p.y < size){
            int x = xlines[first->p.x];
            int y = ylines[first->p.y];

            stoneInfo& info = board[first->p.y][first->p.x];
            if (info.empty()){
                p.setPen( Qt::black );
                eraseBackground(p, first->p.x, first->p.y);
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
    font.setPointSize(int(boxSize * 0.37));
    p.setFont(font);

    while (first != last){
        if (first->p.x >= 0 && first->p.x < size && first->p.y >= 0 && first->p.y < size){
            int x = xlines[first->p.x];
            int y = ylines[first->p.y];

            QColor color = first->t == go::mark::eWhiteTerritory ? QColor(255, 255, 255, 175) : QColor(0, 0, 0, 125);
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

    p.setPen(Qt::red);
    if (node->getX() >= 0 && node->getX() < size && node->getY() >= 0 && node->getY() < size){
        int x = xlines[node->getX()];
        int y = ylines[node->getY()];
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
void BoardWidget::putStone(go::node* n, int moveNumber){
    black = n->isWhite();

    go::stoneNode* stoneNode = dynamic_cast<go::stoneNode*>(n);
    if (stoneNode){
        int x = stoneNode->getX();
        int y = stoneNode->getY();
        if (x >= 0 && x < size && y >= 0 && y < size){
            board[y][x].color = stoneNode->isBlack() ? go::stone::eBlack : go::stone::eWhite;
            board[y][x].number = moveNumber;
            currentMoveNumber  = moveNumber;
            removeDeadStones(x, y);
        }
    }

    go::stoneList::iterator iter = n->stones.begin();
    while (iter != n->stones.end()){
        int  x = iter->p.x;
        int  y = iter->p.y;
        if (x >= 0 && x < size && y >= 0 && y < size){
            board[y][x].color = iter->c;
            removeDeadStones(x, y);
        }
        ++iter;
    }
}

/**
*/
void BoardWidget::removeDeadStones(int x, int y){
    int c = board[y][x].color == go::stone::eBlack ? go::stone::eWhite : go::stone::eBlack;

    int* tmp = new int[size * size];

    memset(tmp, 0, sizeof(int)*size*size);
    if (y > 0 && board[y-1][x].color == c && isDead(tmp, c, x, y - 1))
        dead(tmp);

    memset(tmp, 0, sizeof(int)*size*size);
    if (y < size-1 && board[y+1][x].color == c && isDead(tmp, c, x, y + 1))
        dead(tmp);

    memset(tmp, 0, sizeof(int)*size*size);
    if (x > 0 && board[y][x-1].color == c && isDead(tmp, c, x - 1, y))
        dead(tmp);

    memset(tmp, 0, sizeof(int)*size*size);
    if (x < size-1 && board[y][x+1].color == c && isDead(tmp, c, x + 1, y))
        dead(tmp);

    delete[] tmp;
}

/**
*/
bool BoardWidget::isDead(int* tmp, int c, int x, int y){
    if (tmp[y*size+x])
        return true;
    else if (board[y][x].color == 0)
        return false;
    else if (board[y][x].color != c)
        return true;
    tmp[y*size+x] = board[y][x].color;

    if (y > 0 && !isDead(tmp, c, x, y - 1))
        return false;

    if (y < size-1 && !isDead(tmp, c, x, y + 1))
        return false;

    if (x > 0 && !isDead(tmp, c, x - 1, y))
        return false;

    if (x < size-1 && !isDead(tmp, c, x + 1, y))
        return false;

    return true;
}

/**
*/
bool BoardWidget::isDead(int x, int y){
    int* tmp = new int[size * size];
    memset(tmp, 0, sizeof(int)*size*size);

    go::stone::eColor c = board[y][x].color;
    bool dead = isDead(tmp, c, x, y);

    delete[] tmp;

    return dead;
}

/**
*/
bool BoardWidget::isKill(int x, int y){
    int* tmp = new int[size * size];
    memset(tmp, 0, sizeof(int)*size*size);

    go::stone::eColor c = board[y][x].color == go::stone::eBlack ? go::stone::eWhite : go::stone::eBlack;
    bool dead = (y > 0 && board[y-1][x].color == c && isDead(tmp, c, x, y - 1)) ||
                (y < size-1 && board[y+1][x].color == c && isDead(tmp, c, x, y + 1)) ||
                (x > 0 && board[y][x-1].color == c && isDead(tmp, c, x - 1, y)) ||
                (x < size-1 && board[y][x+1].color == c && isDead(tmp, c, x + 1, y));

    delete[] tmp;

    return dead;
}

/**
*/
void BoardWidget::dead(int* tmp){
    for (int y=0; y<size; ++y){
        for (int x=0; x<size; ++x){
            if (tmp[y*size+x])
                board[y][x].color = go::stone::eEmpty;
        }
    }
}

/**
*/
void BoardWidget::addStone(int x, int y){
    if (board[y][x].empty() == false)
        return;

    go::nodeList::iterator iter = currentNode->childNodes.begin();
    while (iter != currentNode->childNodes.end()){
        if ((*iter)->getX() == x && (*iter)->getY() == y){
            setCurrentNode(*iter);
            return;
        }
        ++iter;
    }

    board[y][x].color = black ? go::stone::eBlack : go::stone::eWhite;
    if (isKill(x, y) == false && isDead(x, y) == true){
        board[y][x].color = go::stone::eEmpty;
        return;
    }

    go::stoneNode* n;
    if (black)
        n = new go::blackNode(currentNode, x, y);
    else
        n = new go::whiteNode(currentNode, x, y);

    addNode(currentNode, n);
    setCurrentNode(n);
}

/**
*/
void BoardWidget::addMark(int x, int y){
    switch (editMode){
        case eAlternateMove:
            return;

        case eAddBlack:
            addStone(currentNode->stones, go::point(x, y), go::stone::eBlack);
            break;

        case eAddWhite:
            addStone(currentNode->stones, go::point(x, y), go::stone::eWhite);
            break;

        case eAddEmpty:
            addStone(currentNode->stones, go::point(x, y), go::stone::eEmpty);
            break;

        case eLabelMark:{
            go::point p(x, y);
            removeMark(currentNode->circles, p);
            removeMark(currentNode->crosses, p);
            removeMark(currentNode->squares, p);
            removeMark(currentNode->triangles, p);
            addCharacter(currentNode->characters, p);
            break;
        }

        case eCircleMark:{
            go::mark mark(x, y, go::mark::eCircle);
            addMark(currentNode->circles, mark);
            removeMark(currentNode->crosses, mark.p);
            removeMark(currentNode->squares, mark.p);
            removeMark(currentNode->triangles, mark.p);
            removeMark(currentNode->characters, mark.p);
            break;
        }

        case eCrossMark:{
            go::mark mark(x, y, go::mark::eCross);
            removeMark(currentNode->circles, mark.p);
            addMark(currentNode->crosses, mark);
            removeMark(currentNode->squares, mark.p);
            removeMark(currentNode->triangles, mark.p);
            removeMark(currentNode->characters, mark.p);
            break;
        }

        case eSquareMark:{
            go::mark mark(x, y, go::mark::eSquare);
            removeMark(currentNode->circles, mark.p);
            removeMark(currentNode->crosses, mark.p);
            addMark(currentNode->squares, mark);
            removeMark(currentNode->triangles, mark.p);
            removeMark(currentNode->characters, mark.p);
            break;
        }

        case eTriangleMark:{
            go::mark mark(x, y, go::mark::eTriangle);
            removeMark(currentNode->circles, mark.p);
            removeMark(currentNode->crosses, mark.p);
            removeMark(currentNode->squares, mark.p);
            addMark(currentNode->triangles, mark);
            removeMark(currentNode->characters, mark.p);
            break;
        }

        case eDeleteMarker:{
            go::point p(x, y);
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

void BoardWidget::addStone(go::stoneList& stoneList, const go::point& p, go::stone::eColor c){
    go::stoneList::iterator iter = stoneList.begin();
    while (iter != stoneList.end()){
        if (iter->p == p){
            iter = stoneList.erase(iter);
            board[p.y][p.x].color = c;
            board[p.y][p.x].number = 0;
            return;
        }
        ++iter;
    }
    currentNode->stones.push_back( go::stone(p, c) );
    board[p.y][p.x].color = c;
    board[p.y][p.x].number = 0;
}

QString BoardWidget::getXString(int x) const{
    int a = x % 25;
    if (a > 7)
        ++a;

    return QString("%1").arg(QChar('A' + a));
}

QString  BoardWidget::getYString(int y) const{
    return QString("%1").arg(size - y);
}

QString  BoardWidget::getXYString(int x, int y) const{
    QString s = getXString(x);
    s.append( getYString(y) );
    return s;
}
