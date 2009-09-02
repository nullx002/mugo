#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
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

#ifdef Q_WS_WIN
#   define usleep(X) Sleep(X/1000)
#else
#   include <unistd.h>
#endif

Sound::Sound(QWidget* parent_) : parent(parent_){
#if defined(Q_WS_WIN)
    lastClock = 0;
#else
    media = Phonon::createPlayer(Phonon::NotificationCategory);
#endif
}

Sound::~Sound(){
#if defined(Q_WS_WIN)
#else
    delete media;
#endif
}

void Sound::setCurrentSource(const QString& source){
#if defined(Q_WS_WIN)
    memset(&mop, 0, sizeof(mop));
    mop.lpstrDeviceType  = L"WaveAudio";
    mop.lpstrElementName = (WCHAR*)source.utf16();
    mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE|MCI_OPEN_ELEMENT, (DWORD)&mop);
#else
    media->setCurrentSource(source);
#endif
}

void Sound::play(){
#if defined(Q_WS_WIN)
    double currentClock = (double)clock() / CLOCKS_PER_SEC;
    if (mop.wDeviceID && currentClock - lastClock > 0.2){
        mciSendCommand(mop.wDeviceID, MCI_STOP, 0, 0);
        mciSendCommand(mop.wDeviceID, MCI_SEEK, MCI_SEEK_TO_START, 0);
        mciSendCommand(mop.wDeviceID, MCI_PLAY, 0, 0);
        lastClock = currentClock;
    }
#else
    if (media->currentTime() == media->totalTime()){
        media->stop();
        media->seek(0);
    }
    media->play();
#endif
}




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
    tutorMode(eNoTutor),
    moveToClicked(false),
    rotateBoard_(0),
    flipBoardHorizontally_(false),
    flipBoardVertically_(false),
    playSound(false),
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

    // board
    boardType  = settings.value("board/boardType").toInt();
    boardColor = settings.value("board/boardColor", BOARD_COLOR).value<QColor>();
    bgColor    = settings.value("board/bgColor", BG_COLOR).value<QColor>();
    tutorColor = settings.value("board/bgTutorColor", BG_TUTOR_COLOR).value<QColor>();
    if (boardType == 0){
        if (boardImage1.load(":/res/bg.png") == false)
            boardType = 2;
    }
    else if (boardType == 1){
        if (boardImage1.load( settings.value("board/boardPath").toString() ) == false)
            boardType = 2;
    }

    // black stone
    whiteType  = settings.value("board/whiteType").toInt();
    whiteColor = settings.value("board/whiteColor", WHITE_COLOR).value<QColor>();
    if (whiteType == 0){
        if (white1.load(":/res/white_128_ds.png") == false)
            whiteType = 2;
    }
    else{
        if (white1.load( settings.value("board/whitePath").toString() ) == false)
            whiteType = 2;
    }

    // white stone
    blackColor = settings.value("board/blackColor", BLACK_COLOR).value<QColor>();
    blackType  = settings.value("board/blackType").toInt();
    if (blackType == 0){
        if (black1.load(":/res/black_128_ds.png") == false)
            blackType = 2;
    }
    else{
        if (black1.load( settings.value("board/blackPath").toString() ) == false)
            blackType = 2;
    }

    // markers
    focusColor  = settings.value("board/focusColor", FOCUS_COLOR).value<QColor>();
    branchColor = settings.value("board/branchColor", BRANCH_COLOR).value<QColor>();

    // sound
    if (settings.value("board/soundType").toInt() == 0){
        QStringList soundPathList;
#if defined(Q_WS_MAC)
        soundPathList.push_back( QFileInfo(qApp->applicationDirPath() + "/../Resources/sounds/").absolutePath() );
#endif
        soundPathList.push_back(qApp->applicationDirPath() + "/sounds");
        soundPathList.push_back("/usr/share/" APPNAME "/sounds");
        soundPathList.push_back("/usr/local/share/" APPNAME "/sounds");
        soundPathList.push_back("./sounds");
        QStringList::iterator iter = soundPathList.begin();
        while (iter != soundPathList.end()){
            QFileInfo finfo( *iter + "/stone.wav" );
            if (finfo.exists()){
                setStoneSoundPath(finfo.filePath());
                break;
            }
            ++iter;
        }
    }
    else{
        QFileInfo finfo( settings.value("board/soundPath").toString() );
        if (finfo.exists())
            setStoneSoundPath(finfo.filePath());
    }
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

    if (editMode == eGtp && gtpStatus != eGtpNone)
        return;

    bool black;
    if (editMode == eAlternateMove || editMode == eGtp || tutorMode == eTutorBossSides || tutorMode == eTutorOneSide)
        black = isBlack;
    else if (editMode == eAddBlack || editMode == eAddWhite)
        black = editMode == eAddBlack;
    else
        return;

    int bx = (int)floor( double(e->x() - xlines[0] + boxSize / 2) / boxSize );
    int by = (int)floor( double(e->y() - ylines[0] + boxSize / 2) / boxSize );

    offscreenBuffer3 = offscreenBuffer2.copy();
    QPainter p(&offscreenBuffer3);

    if (! (bx < 0 || bx >= xlines.size() || by < 0 || by >= ylines.size() || board[by][bx].color != go::empty) ){
        p.setOpacity(0.5);
        drawImage(p, bx, by, black ? black2 : white2);
    }

    repaint();
}

/**
*/
void BoardWidget::wheelEvent(QWheelEvent* e){
    QWidget::wheelEvent(e);

    if (tutorMode != eNoTutor)
        return;

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
    int boardX = (int)floor( double(e->x() - xlines[0] + boxSize / 2) / boxSize );
    int boardY = (int)floor( double(e->y() - ylines[0] + boxSize / 2) / boxSize );

    if (boardX < 0 || boardX >= xsize || boardY < 0 || boardY >= ysize)
        return;

    int sgfX, sgfY;
    boardToSgfCoordinate(boardX, boardY, sgfX, sgfY);

    if (editMode == eGtp)
        gtpLButtonDown(sgfX, sgfY);
    else if (tutorMode == eTutorBossSides)
        tutor(sgfX, sgfY);
    else if (tutorMode == eTutorOneSide){
        if (tutor(sgfX, sgfY)){
            go::nodeList::iterator iter = qFind(nodeList.begin(), nodeList.end(), currentNode);
            if (iter != nodeList.end() && ++iter != nodeList.end()){
                usleep(500000);
                setCurrentNode(*iter);
            }
        }
    }
    else if (editMode == eAlternateMove){
        if (moveToClicked && board[boardY][boardX].node)
            setCurrentNode( board[boardY][boardX].node );
        else
            addStoneNodeCommand(sgfX, sgfY, boardX, boardY);
    }
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

    if (tutorMode != eNoTutor)
        p.fillRect(0, 0, width_, height_, tutorColor);
    else
        p.fillRect(0, 0, width_, height_, bgColor);

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

void BoardWidget::insertData(const go::nodePtr node, const go::fileBase& data){
    go::data d;
    data.get(d);

    node->childNodes.push_back( d.root );
    d.root->parent = node;

    createNodeList();
    setDirty(true);
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
    RotateSgfCommand* command = new RotateSgfCommand(this, tr("Rotate SGF"));
    rotateSgf(goData.root, command);
    undoStack.push(command);

    setDirty(true);
}

void BoardWidget::flipSgfHorizontally(){
    RotateSgfCommand* command = new RotateSgfCommand(this, tr("Flip SGF Horizontally"));
    flipSgf(goData.root, goData.root->xsize, 0, command);
    undoStack.push(command);

    setDirty(true);
}

void BoardWidget::flipSgfVertically(){
    RotateSgfCommand* command = new RotateSgfCommand(this, tr("Flip SGF Vertically"));
    flipSgf(goData.root, 0, goData.root->ysize, command);
    undoStack.push(command);

    setDirty(true);
}

void BoardWidget::rotateSgf(go::nodePtr node, QUndoCommand* command){
    go::point p = node->position;
    p.x = goData.root->ysize - node->position.y - 1;
    p.y = node->position.x;
    new MovePositionCommand(this, node, p, command);

    go::nodeList::iterator iter = node->childNodes.begin();
    while (iter != node->childNodes.end()){
        rotateSgf(*iter, command);
        ++iter;
    }

    rotateStoneSgf(node, node->stones, command);
    rotateMarkSgf(node, node->crosses, command);
    rotateMarkSgf(node, node->squares, command);
    rotateMarkSgf(node, node->triangles, command);
    rotateMarkSgf(node, node->circles, command);
    rotateMarkSgf(node, node->characters, command);
    rotateMarkSgf(node, node->blackTerritories, command);
    rotateMarkSgf(node, node->whiteTerritories, command);
}

void BoardWidget::rotateStoneSgf(go::nodePtr node, go::stoneList& stoneList, QUndoCommand* command){
    go::stoneList::iterator iter = stoneList.begin();
    while (iter != stoneList.end()){
        go::point p = iter->p;
        p.x = goData.root->ysize - iter->p.y - 1;
        p.y = iter->p.x;

        new MoveStoneCommand(this, node, &(*iter), p, command);

        ++iter;
    }
}

void BoardWidget::rotateMarkSgf(go::nodePtr node, go::markList& markList, QUndoCommand* command){
    go::markList::iterator iter = markList.begin();
    while (iter != markList.end()){
        go::point p = iter->p;
        p.x = goData.root->ysize - iter->p.y - 1;
        p.y = iter->p.x;

        new MoveMarkCommand(this, node, &(*iter), p, command);

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
    int xsize = goData.root->xsize;
    int ysize = goData.root->ysize;

    setDirty(false);
    goData.clear();
    goData.root->xsize = xsize;
    goData.root->ysize = ysize;
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
    if (tutorMode != eNoTutor)
        return;

    undoStack.push( new AddNodeCommand(this, parentNode, childNode, select) );
}

/**
* public slot
*/
void BoardWidget::insertNodeCommand(go::nodePtr parentNode, go::nodePtr childNode, bool select){
    if (tutorMode != eNoTutor)
        return;

    undoStack.push( new InsertNodeCommand(this, parentNode, childNode, select) );
}

/**
* public slot
*/
void BoardWidget::deleteNodeCommand(go::nodePtr node, bool deleteChildren){
    if (tutorMode != eNoTutor)
        return;
    else if( node == goData.root )
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
        }
    }

    setCurrentNode(parent);

    setDirty(true);
    emit nodeDeleted(node, deleteChildren);

    createNodeList();
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
        addStoneNodeCommand(-1, -1);
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

    int ps = p.font().pointSize();
    int w = (width_  - (showCoordinates ? ps*5 : 0)) / xsize;
    int h = (height_ - (showCoordinates ? ps*5 : 0)) / ysize;
    boxSize = qMin(w, h);
    w = boxSize * (xsize - 1);
    h = boxSize * (ysize - 1);
    int margin = int(boxSize * 0.5);

    int l = (width_ - w) / 2;
    int r = l + boxSize * (xsize - 1);
    int t = (height_ - h) / 2;
    int b = t + boxSize * (ysize - 1);

    // create board and stone image
    boardRect.setRect(l - margin, t - margin, w + margin * 2, h + margin * 2);
//    if (boardRect.width() != boardImage2.width()){
        boardImage2 = QImage(boardRect.size(), QImage::Format_RGB32);
        QPainter board(&boardImage2);
        if (boardType == 0 || boardType == 1)
            board.fillRect(0, 0, boardRect.width(), boardRect.height(), QBrush(boardImage1));
        else
            board.fillRect(0, 0, boardRect.width(), boardRect.height(), boardColor);

        if (blackType == 0 || blackType == 1)
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

        if (whiteType == 0 || whiteType == 1)
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

    // horizontal line
    ylines.clear();
    for (int i=0; i<ysize; ++i){
        int y = t + i * boxSize;
        p.drawLine(l, y, r, y);
        ylines.push_back(y);
    }

    // vertical line
    xlines.clear();
    for (int i=0; i<xsize; ++i){
        int x = l + i * boxSize;
        p.drawLine(x, t, x, b);
        xlines.push_back(x);
    }

    // draw stars
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

    drawCross(p, currentNode->crosses.begin(), currentNode->crosses.end());
    drawTriangle(p, currentNode->triangles.begin(), currentNode->triangles.end());
    drawCircle(p, currentNode->circles.begin(), currentNode->circles.end());
    drawSquare(p, currentNode->squares.begin(), currentNode->squares.end());
    drawCharacter(p, currentNode->characters.begin(), currentNode->characters.end());
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

    p.save();
    p.setPen( branchColor );

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

    p.restore();
}

/**
*/
void BoardWidget::drawCross(QPainter& p, go::markList::iterator first, go::markList::iterator last){
    QPainterPath path;
    double w = boxSize * 0.18;

    path.moveTo(-w, -w);
    path.lineTo(w, w);

    path.moveTo(w, -w);
    path.lineTo(-w, w);

    drawMark(p, path, first, last);
}

/**
*/
void BoardWidget::drawTriangle(QPainter& p, go::markList::iterator first, go::markList::iterator last){
    QPainterPath path;
    double w = boxSize * 0.22;
    double h = boxSize * 0.18;
    QPolygonF polygon(3);
    polygon[0] = QPointF(0, -h);
    polygon[1] = QPointF(-w, h);
    polygon[2] = QPointF(w, h);
    path.addPolygon(polygon);
    path.closeSubpath();

    drawMark(p, path, first, last);
}

/**
*/
void BoardWidget::drawCircle(QPainter& p, go::markList::iterator first, go::markList::iterator last){
    QPainterPath path;
    double w = boxSize * 0.42;
    path.addEllipse(-w/2, -w/2, w, w);
    drawMark(p, path, first, last);
}

/**
*/
void BoardWidget::drawSquare(QPainter& p, go::markList::iterator first, go::markList::iterator last){
    QPainterPath path;
    double w = boxSize * 0.4;
    path.addRect(-w/2, -w/2, w, w);
    drawMark(p, path, first, last);
}

void BoardWidget::drawCharacter(QPainter& p, go::markList::iterator first, go::markList::iterator last){
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
void BoardWidget::drawMark(QPainter& p, const QPainterPath& path, go::markList::iterator first, go::markList::iterator last){
    if (showMarker == false)
        return;

    while (first != last){
        int boardX, boardY;
        sgfToBoardCoordinate(first->p.x, first->p.y, boardX, boardY);

        if (boardX >= 0 && boardX < xsize && boardY >= 0 && boardY < ysize)
            drawPath(p, path, boardX, boardY);
        ++first;
    }
}

/**
*/
void BoardWidget::drawPath(QPainter& p, const QPainterPath& path, int boardX, int boardY){
    p.save();

    QColor color;
    stoneInfo& info = board[boardY][boardX];
    if (info.empty()){
        p.setPen( QPen(Qt::black, 2) );
        eraseBackground(p, boardX, boardY);
    }
    else
        p.setPen( info.black() ? QPen(Qt::white, 2) : QPen(Qt::black, 2) );

    int x = xlines[boardX];
    int y = ylines[boardY];
    p.translate(x, y);

    p.drawPath(path);

    p.restore();
}

/**
*/
void BoardWidget::drawTerritories(QPainter& p){
    p.save();


    for (int y=0; y<ysize; ++y){
        for (int x=0; x<xsize; ++x){
            if (!board[y][x].blackTerritory() && !board[y][x].whiteTerritory())
                continue;

            int bx = xlines[x];
            int by = ylines[y];

            QColor color = board[y][x].whiteTerritory() ? QColor(255, 255, 255, 110) : QColor(0, 0, 0, 60);
            p.fillRect(bx-boxSize/2, by-boxSize/2, boxSize, boxSize, color);

            color.setAlpha(255);
            p.fillRect(bx-boxSize/6, by-boxSize/6, boxSize/3, boxSize/3, color);
        }
    }

    p.restore();
}

/**
*/
void BoardWidget::drawCurrentMark(QPainter& p, go::nodePtr node){
    if (node->isPass())
        return;

    p.save();

    int boardX, boardY;
    sgfToBoardCoordinate(node->getX(), node->getY(), boardX, boardY);
    int x = xlines[boardX];
    int y = ylines[boardY];

    double w = boxSize * 0.22;
    double h = boxSize * 0.18;
    QPolygonF polygon(3);
    polygon[0] = QPointF(x, y - h);
    polygon[1] = QPointF(x - w, y + h);
    polygon[2] = QPointF(x + w, y + h);

    p.setPen(focusColor);
    p.setBrush( QBrush(focusColor) );
    p.drawPolygon(polygon);

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
void BoardWidget::addStoneNodeCommand(int sgfX, int sgfY){
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
        addStoneNodeCommand(sgfX, sgfY, boardX, boardY);
    }
}

/**
*/
void BoardWidget::addStoneNodeCommand(int sgfX, int sgfY, int boardX, int boardY){
    if (board[boardY][boardX].empty() == false)
        return;

    if (tutor(sgfX, sgfY))
        return;

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
bool BoardWidget::tutor(int sgfX, int sgfY){
    go::nodeList::iterator iter = currentNode->childNodes.begin();
    while (iter != currentNode->childNodes.end()){
        if ((*iter)->getX() == sgfX && (*iter)->getY() == sgfY){
            setCurrentNode(*iter);
            return true;
        }
        ++iter;
    }

    return false;
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
            addEmpty(currentNode, go::point(sgfX, sgfY), go::point(boardX, boardY));
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
            removeMark(currentNode->characters, p);
            removeStone(currentNode->stones, p, go::point(boardX, boardY));
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

bool BoardWidget::removeMark(go::markList& markList, const go::point& p){
    go::markList::iterator iter = markList.begin();
    while (iter != markList.end()){
        if (iter->p == p){
            markList.erase(iter);
            return true;
        }
        ++iter;
    }
    return false;
}

void BoardWidget::addStone(go::nodePtr node, const go::point& sgfPoint, go::color color){
    go::point boardPoint;
    sgfToBoardCoordinate(sgfPoint.x, sgfPoint.y, boardPoint.x, boardPoint.y);
    addStone(node, sgfPoint, boardPoint, color);
}

void BoardWidget::addStone(go::nodePtr node, const go::point& sp, const go::point& bp, go::color c){
    if (removeStone(node->stones, sp, bp)){
        modifyNode(node);
        return;
    }

    if (!board[bp.y][bp.x].empty())
        return;

    go::nodePtr stoneNode( node->isStone() ? go::nodePtr(new go::node(node)) : node );

    stoneNode->stones.push_back( go::stone(sp, c) );
    board[bp.y][bp.x].color = c;
    board[bp.y][bp.x].number = 0;

    if (stoneNode == node)
        modifyNode(node);
    else
        insertNodeCommand(node, stoneNode);
}

void BoardWidget::addEmpty(go::nodePtr node, const go::point& sgfPoint){
    go::point boardPoint;
    sgfToBoardCoordinate(sgfPoint.x, sgfPoint.y, boardPoint.x, boardPoint.y);
    addEmpty(node, sgfPoint, boardPoint);
}

void BoardWidget::addEmpty(go::nodePtr node, const go::point& sp, const go::point& bp){
    if (removeStone(node->stones, sp, bp)){
        modifyNode(node);
        return;
    }

    if (board[bp.y][bp.x].empty())
        return;

    go::nodePtr stoneNode( node->isStone() ? go::nodePtr(new go::node(node)) : node );

    stoneNode->stones.push_back( go::stone(sp, go::empty) );
    board[bp.y][bp.x].color = go::empty;
    board[bp.y][bp.x].number = 0;

    if (stoneNode == node)
        modifyNode(node);
    else
        insertNodeCommand(node, stoneNode);
}

bool BoardWidget::removeStone(go::stoneList& stoneList, const go::point& sp, const go::point& bp){
    go::stoneList::iterator iter = stoneList.begin();
    while (iter != stoneList.end()){
        if (iter->p == sp) {
            stoneList.erase(iter);
            board[bp.y][bp.x].color = go::empty;
            board[bp.y][bp.x].number = 0;
            return true;
        }
        ++iter;
    }
    return false;
}

QString BoardWidget::toString(go::nodePtr node) const{
    if (node->isPass())
        return "Pass";
    else
        return getXYString(node->position.x, node->position.y);
}

QString BoardWidget::getXString(int x, bool showI) const{
    int a = x % 25;
    if (showI == false && a > 7)
        ++a;

    return QString("%1").arg(QChar('A' + a));
}

QString BoardWidget::getXString(int x) const{
    return getXString(x, showCoordinatesI);
}

QString  BoardWidget::getYString(int y) const{
    return QString("%1").arg(goData.root->ysize - y);
}

QString  BoardWidget::getXYString(int x, int y, bool showI) const{
    if (x < 0 || x >= goData.root->xsize || y < 0 || y >= goData.root->ysize)
        return "";

    QString s = getXString(x, showI);
    s.append( getYString(y) );
    return s;
}

QString BoardWidget::getXYString(int x, int y) const{
    return getXYString(x, y, showCoordinatesI);
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

void BoardWidget::whiteFirst(bool whiteFirst){
    if (whiteFirst)
        goData.root->setBlack();
    else
        goData.root->setWhite();
    createBoardBuffer();
}

void BoardWidget::countTerritory(){
    char* tmp = new char[xsize * ysize];

    for (int y=0; y<ysize; ++y){
        for (int x=0; x<xsize; ++x){
            memset(tmp, 0, xsize*ysize);
            int c = go::empty;
            whichTerritory(x, y, tmp, c);
            if (board[y][x].empty() && (c & go::blackTerritory || c & go::whiteTerritory))
                board[y][x].color = c;
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

void BoardWidget::addTerritory(int x, int y){
    if (board[y][x].white() && !board[y][x].blackTerritory())
        setTerritory(x, y, go::blackTerritory);
    else if (board[y][x].black() && !board[y][x].whiteTerritory())
        setTerritory(x, y, go::whiteTerritory);
    else if ( (board[y][x].white() || board[y][x].black()) && (board[y][x].blackTerritory() || board[y][x].whiteTerritory()) )
        unsetTerritory(x, y);

    countTerritory();
    repaintBoard(false);
}

void BoardWidget::setTerritory(int x, int y, int c){
    if ( c & go::blackTerritory && (board[y][x].black() || board[y][x].blackTerritory()) )
        return;
    else if ( c & go::whiteTerritory && (board[y][x].white() || board[y][x].whiteTerritory()) )
        return;

    board[y][x].color |= c;
    if (c == go::blackTerritory)
        board[y][x].color &= ~go::whiteTerritory;
    else
        board[y][x].color &= ~go::blackTerritory;

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
    gtpProcess = proc;
    if (gtpProcess){
        editMode = eGtp;
        gtpStatus = eGtpNone;
        moveToClicked = false;
        connect(gtpProcess, SIGNAL(readyRead()), this, SLOT(gtpReadReady()));

        if (isYourColorBlack == false && goData.root->handicap == 0){
            gtpWrite("genmove black\n");
            gtpStatus = eGtpGen;
        }
        else if (goData.root->handicap > 0){
            whiteFirst(true);
            gtpHandicap();
            if (isYourColorBlack){
                gtpWrite("genmove white\n");
                gtpStatus = eGtpGen;
            }
        }
    }
    else{
        editMode = eAlternateMove;
    }
}

void BoardWidget::gtpWrite(const QString& buf){
    if (gtpProcess == NULL)
        return;

    qDebug() << buf;
    QByteArray ba = buf.toAscii();
    gtpProcess->write( ba );
}

void BoardWidget::gtpPut(int x, int y){
    if (gtpProcess == NULL)
        return;

    gtpX = x;
    gtpY = y;

    QString xy;
    if (x == -1 && y == -1)
        xy = "PASS";
    else
        xy =getXYString(x, y, false);

    if (isBlack){
        gtpWrite( QString("play black %1\n").arg(xy) );
        gtpStatus = eGtpPut;
    }
    else{
        gtpWrite( QString("play white %1\n").arg(xy) );
        gtpStatus = eGtpPut;
    }
}

void BoardWidget::gtpReadReady(){
    if (gtpProcess == NULL)
        return;

    gtpBuf += gtpProcess->readAll();

//qDebug() << gtpBuf.size();
//for (int i=0; i<gtpBuf.size(); ++i)
//    qDebug("%c(%d)", (gtpBuf[i].toAscii() != 10 ? gtpBuf[i].toAscii() : ' '), gtpBuf[i].toAscii());

    if (gtpBuf.size() < 4 || gtpBuf.right(2) != "\n\n")
        return;

    QStringList resList = gtpBuf.split("\n\n");
    resList.pop_back();
    gtpBuf.clear();
    qDebug() << resList;

    QString buf = resList.last();
    buf.remove(0, 2);

    if (gtpStatus == eGtpPut){
        if (buf == "illegal move"){
            gtpStatus = eGtpNone;
            return;
        }
        addStoneNodeCommand(gtpX, gtpY);

        if (isGtpGameEnd()){
            gtpGameEnd();
            return;
        }

        if (isBlack)
            gtpWrite("genmove black\n");
        else
            gtpWrite("genmove white\n");
        gtpStatus = eGtpGen;
    }
    else if (gtpStatus == eGtpGen){
        if (buf == "resign"){
            QMessageBox::information(this, APPNAME, tr("Computer resign."));
            gtpWrite("quit\n");
            return;
        }

        int x, y;
        if (gtpGetCoordinate(buf, x, y)){
            addStoneNodeCommand(x, y);
            gtpStatus = eGtpNone;

            if (isGtpGameEnd()){
                gtpGameEnd();
                return;
            }
        }
    }
    else if (gtpStatus == eGtpGameEnd){
        gtpWrite("quit\n");
        emit gtpGameEnded();

        QStringList deadStones = buf.split(QRegExp("[ \n]"));
        foreach(QString stone, deadStones){
            int sx, sy;  // sgfX, sgfY
            if (gtpGetCoordinate(stone, sx, sy) == false)
                continue;

            int bx, by;  // boardX, boardY
            sgfToBoardCoordinate(sx, sy, bx, by);

            if (!board[by][bx].blackTerritory() && !board[by][bx].whiteTerritory())
                addTerritory(bx, by);
        }

        setCountTerritoryMode(true);
    }
}

bool BoardWidget::gtpGetCoordinate(const QString& buf, int& x, int& y){
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

void BoardWidget::gtpHandicap(){
    int xpos = goData.root->xsize > 9 ? 4 : 3;
    int ypos = goData.root->ysize > 9 ? 4 : 3;

    int x[9] = {
        goData.root->xsize - xpos,
        xpos - 1,
        goData.root->xsize - xpos,
        xpos - 1,
        goData.root->xsize / 2,
        goData.root->xsize - xpos,
        xpos - 1,
        goData.root->xsize / 2,
        goData.root->xsize / 2,
    };
    int y[9] = {
        ypos - 1,
        goData.root->ysize - ypos,
        goData.root->ysize - ypos,
        ypos - 1,
        goData.root->ysize / 2,
        goData.root->ysize / 2,
        goData.root->ysize / 2,
        goData.root->ysize - ypos,
        ypos - 1,
    };

    QList<int> xlist, ylist;
    for (int i=0; i<qMin(4, goData.root->handicap); ++i){
        xlist.push_back( x[i] );
        ylist.push_back( y[i] );
    }

    if (goData.root->handicap > 5){
        xlist.push_back( x[5] );
        xlist.push_back( x[6] );
        ylist.push_back( y[5] );
        ylist.push_back( y[6] );
    }
    if (goData.root->handicap > 7){
        xlist.push_back( x[7] );
        xlist.push_back( x[8] );
        ylist.push_back( y[7] );
        ylist.push_back( y[8] );
    }

    if (goData.root->handicap > 4 && goData.root->handicap % 2 != 0){
        xlist.push_back( x[4] );
        ylist.push_back( y[4] );
    }

    QString msg;
    for (int i=0; i<goData.root->handicap; ++i){
        addStone(goData.root, go::point(xlist[i], ylist[i]), go::black);
        msg += "play black ";
        msg += getXYString(xlist[i], ylist[i], false);
        msg += "\n";
    }
    gtpWrite(msg);
}

void BoardWidget::gtpGameEnd(){
    gtpStatus = eGtpGameEnd;
    gtpWrite("final_status_list dead\n");
}

bool BoardWidget::isGtpGameEnd() const{
    if (gtpStatus != eGtpGameEnd){
        if (nodeList.size() >= 2){
            go::nodeList::const_iterator iter = nodeList.end();
            const go::nodePtr& node1 = *--iter;
            const go::nodePtr& node2 = *--iter;
            return node1->isPass() && node2->isPass();
        }
    }
    return false;
}
