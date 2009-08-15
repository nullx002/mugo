#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QtGui/QWidget>
#include <QUndoStack>
#include <QLabel>
#include <QVector>
#include <QList>
#include <QUndoCommand>

#ifdef Q_WS_X11
#   include <phonon>
#else
#   include <QSound>
#endif

#include "godata.h"

namespace Ui {
    class BoardWidget;
}


class Sound{
public:
    Sound(QWidget* /*parent*/) : media(NULL){
#ifdef Q_WS_X11
        media = Phonon::createPlayer(Phonon::NotificationCategory);
//        media = new Phonon::MediaObject(parent);
//        audioOutput = new Phonon::AudioOutput(Phonon::NotificationCategory, parent);
//        Phonon::createPath(media, audioOutput);
#endif
    }
    ~Sound(){
        delete media;
    }
    void setCurrentSource(const QString& source){
#ifdef Q_WS_X11
        media->setCurrentSource(source);
#else
        delete media;
        media = new QSound(source, NULL);
#endif
    }

    void play(){
#ifdef Q_WS_X11
        if (media->currentTime() == media->totalTime()){
            media->stop();
            media->seek(0);
        }
#endif
        if (media)
            media->play();
    }

#ifdef Q_WS_X11
    Phonon::MediaObject* media;
//    Phonon::AudioOutput* audioOutput;
#else
    QSound* media;
#endif
};


class BoardWidget : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(BoardWidget)
public:
    enum eEditMode{ eAlternateMove, eAddBlack, eAddWhite, eAddEmpty, eLabelMark, eCrossMark, eCircleMark, eSquareMark, eTriangleMark, eDeleteMarker, eCountTerritory };

    struct stoneInfo{
        stoneInfo() : number(0), color(go::empty){}
        bool empty() const{ return (color & (go::black | go::white)) == 0; }
        bool black() const{ return color & go::black; }
        bool white() const{ return color & go::white; }
        bool blackTerritory() const{ return color & go::blackTerritory; }
        bool whiteTerritory() const{ return color & go::whiteTerritory; }

        int number;
        int color;
        go::nodePtr node;
    };

    explicit BoardWidget(QWidget *parent = 0);
    virtual ~BoardWidget();

    void readSettings();

    // draw
    void repaintBoard(bool board=true, bool stones=true);
    void paintBoard(QPaintDevice* pd);
    void paintStones(QPaintDevice* pd);
    void paintTerritories(QPaintDevice* pd);

    // set/get data
    void getData(go::fileBase& data);
    void setData(const go::fileBase& data);
    void clear();
    go::nodePtr findNodeFromMoveNumber(int moveNumber);

    // get node
    go::data& getData(){ return goData; }
    go::nodePtr getCurrentNode(){ return currentNode; }
    const go::nodeList& getCurrentNodeList() const{ return nodeList; }

    void getCaptured(int& black, int& white) const{ black = capturedBlack; white = capturedWhite; }
    int  getMoveNumber() const{ return currentMoveNumber; }

    // dirty flag
    bool isDirty() const{ return dirty; }
    void setDirty(bool dirty){ this->dirty = dirty; }

    // set option
    void setEditMode(eEditMode editMode){ this->editMode = editMode; }
    void setShowMoveNumber(bool visible){ showMoveNumber = visible; repaintBoard(); }
    void setShowMoveNumber(int number){ showMoveNumberCount = number; repaintBoard(); }
    void setShowCoordinates(bool visible){ showCoordinates = visible; repaintBoard(); }
    void setShowCoordinatesWithI(bool withI){ showCoordinatesI = withI; repaintBoard(); }
    void setShowMarker(bool visible){ showMarker = visible; repaintBoard(); }
    void setShowBranchMoves(bool visible){ showBranchMoves = visible; repaintBoard(); }
    void setAnnotation(int annotation){ currentNode->annotation = (go::node::eAnnotation)annotation; modifyNode(currentNode); }
    void setBoardSize(int xsize, int ysize);
    void setMoveToClicked(bool moveMode = true){ moveToClicked = moveMode; }
    void rotateSgf();
    void flipSgfHorizontally();
    void flipSgfVertically();
    int  rotateBoard();
    void flipBoardHorizontally(bool flip);
    void flipBoardVertically(bool flip);
    void resetBoard();
    void setPlaySound(bool play){ playSound = play; }
    void setStoneSoundPath(const QString& path){ stoneSoundPath = path; stoneSound.setCurrentSource(path); }
    void setCountTerritoryMode(bool countMode);

    void createBoardBuffer();
    QString toString(go::nodePtr node) const;
    QString getXString(int x) const;
    QString getYString(int y) const;
    QString getXYString(int x, int y) const;

public slots:
    void addNodeCommand(go::nodePtr parent, go::nodePtr node, bool select=true);
    void deleteNodeCommand(go::nodePtr node, bool deleteChildren=true);
    void setMoveNumberCommand(go::nodePtr node, int moveNumber);
    void unsetMoveNumberCommand(go::nodePtr node);
    void setNodeNameCommand(go::nodePtr node, const QString& nodeName);
    void setCommentCommand(go::nodePtr node, const QString& comment);
    void addNode(go::nodePtr parent, go::nodePtr node, bool select=true);
    void deleteNode(go::nodePtr node, bool deleteChildren=true);
    void modifyNode(go::nodePtr node, bool recreateBoardBuffer=false);
    void setCurrentNode(go::nodePtr node = go::nodePtr());

signals:
    void nodeAdded(go::nodePtr parent, go::nodePtr node, bool select=false);
    void nodeDeleted(go::nodePtr node, bool deleteChildren);
    void nodeModified(go::nodePtr node);
    void currentNodeChanged(go::nodePtr node);
    void updateTerritory(int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory, double komi);

protected:
    // event
    virtual void changeEvent(QEvent* e);
    virtual void paintEvent(QPaintEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void wheelEvent(QWheelEvent* e);
    virtual void resizeEvent( QResizeEvent* e);

    // mouse event
    void onLButtonDown(QMouseEvent* e);
    void onRButtonDown(QMouseEvent* e);

    // draw
    void drawBoard(QPainter& p);
    void drawCoordinates(QPainter& p);
    void drawStones(QPainter& p);
    void drawStones2(QPainter& p);
    void drawBranchMoves(QPainter& p, go::nodeList::iterator first, go::nodeList::iterator last);
    void drawMark(QPainter& p, go::markList::iterator first, go::markList::iterator last);
//    void drawTerritories(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawTerritories(QPainter& p);
    void drawCurrentMark(QPainter& p, go::nodePtr node);
    void drawImage(QPainter& p, int x, int y, const QImage& image);
    void eraseBackground(QPainter& p, int x, int y);
    void getStartPosition(QList<int>& star, int size);

    // buffer
    void putStone(go::nodePtr n, int moveNumber);
    void removeDeadStones(int x, int y);
    bool isDead(int* tmp, int c, int x, int y);
    bool isDead(int x, int y);
    bool isKill(int x, int y);
    void dead(int* tmp);

    // territory
    void countTerritory();
    void whichTerritory(int x, int y, char* tmp, int& c);
    void updateTerritory(int x, int y);
    void addTerritory(int x, int y);
    void setTerritory(int x, int y, int c);
    void unsetTerritory(int x, int y);
    void getCountTerritory(int& alive_b, int& alive_w, int& dead_b, int& dead_w, int& bt, int& wt);

    void createNodeList();
    void addStone(int sgfX, int sgfY, int boardX, int boardY);
    void addMark(int sgfX, int sgfY, int boardX, int boardY);
    void addMark(go::markList& markList, const go::mark& mark);
    void addCharacter(go::markList& markList, const go::point& p);
    void removeMark(go::markList& markList, const go::point& p);
    void addStone(go::nodePtr node, const go::point& sgfP, const go::point& boardP, go::color color);
    void rotateSgf(go::nodePtr node);
    void rotateStoneSgf(go::stoneList& stoneList);
    void rotateMarkSgf(go::markList& markList);
    void flipSgf(go::nodePtr node, int xsize, int ysize, QUndoCommand* command);
    void flipStoneSgf(go::nodePtr node, go::stoneList& stoneList, int xsize, int ysize, QUndoCommand* command);
    void flipMarkSgf(go::nodePtr node, go::markList& markList, int xsize, int ysize, QUndoCommand* command);

    void boardToSgfCoordinate(int boardX, int boardY, int& sgfX, int& sgfY);
    void sgfToBoardCoordinate(int sgfX, int sgfY, int& boardX, int& boardY);

public:
    QUndoStack undoStack;

private:
    Ui::BoardWidget *m_ui;

    // data
    bool dirty;
    go::data goData;
    int capturedBlack;
    int capturedWhite;
    bool isBlack;
    go::nodeList nodeList;
    go::nodePtr currentNode;
    int currentMoveNumber;

    // option
    int boardType, whiteType, blackType;
    bool showMoveNumber;
    int  showMoveNumberCount;
    bool showCoordinates;
    bool showCoordinatesI;
    bool showMarker;
    bool showBranchMoves;
    eEditMode editMode;
    bool moveToClicked;
    int  rotateBoard_;
    bool flipBoardHorizontally_;
    bool flipBoardVertically_;

    bool playSound;
    QString stoneSoundPath;

    // draw object
    QPixmap offscreenBuffer1, offscreenBuffer2, offscreenBuffer3;
    QImage  black1, black2;
    QImage  white1, white2;
    QImage  boardImage1, boardImage2;
    QColor  boardColor, blackColor, whiteColor;

    int width_;
    int height_;
    int boxSize;
    int xsize;
    int ysize;

    QRect boardRect;
    QList<int> xlines;
    QList<int> ylines;
    QVector< QVector<stoneInfo> > board;

    // sound
    Sound stoneSound;
    // Phonon
//    Phonon::MediaObject* stoneSound;
};


class AddNodeCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(AddNodeCommand)

public:
    AddNodeCommand(BoardWidget* boardWidget, go::nodePtr parentNode, go::nodePtr childNode, bool select, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr parentNode;
    go::nodePtr childNode;
    bool select;
};

class DeleteNodeCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(DeleteNodeCommand)

public:
    DeleteNodeCommand(BoardWidget* boardWidget, go::nodePtr node, bool deleteChildren, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
    bool deleteChildren;
};

class SetMoveNumberCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetMoveNumberCommand)

public:
    SetMoveNumberCommand(BoardWidget* boardWidget, go::nodePtr node, int moveNumber, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
    int moveNumber;
    int oldMoveNumber;
};

class UnsetMoveNumberCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(UnsetMoveNumberCommand)

public:
    UnsetMoveNumberCommand(BoardWidget* boardWidget, go::nodePtr node, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
    int oldMoveNumber;
};

class SetNodeNameCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetNodeNameCommand)

public:
    SetNodeNameCommand(BoardWidget* boardWidget, go::nodePtr node, const QString& nodeName, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
    QString nodeName;
    QString oldNodeName;
};

class SetCommentCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetCommentCommand)

public:
    SetCommentCommand(BoardWidget* boardWidget, go::nodePtr node, const QString& comment, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
    QString comment;
    QString oldComment;
};

class MovePositionCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(MovePositionCommand)

public:
    MovePositionCommand(BoardWidget* boardWidget, go::nodePtr node, const go::point& pos, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr  node;
    go::point    pos;
    go::point    oldPos;
};

class MoveStoneCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(MoveStoneCommand)

public:
    MoveStoneCommand(BoardWidget* boardWidget, go::nodePtr node, go::stone* stone, const go::point& pos, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr  node;
    go::stone*   stone;
    go::point    pos;
    go::point    oldPos;
};

class MoveMarkCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(MoveMarkCommand)

public:
    MoveMarkCommand(BoardWidget* boardWidget, go::nodePtr node, go::mark* mark, const go::point& pos, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr  node;
    go::mark*    mark;
    go::point    pos;
    go::point    oldPos;
};

class FlipSGFVerticallyCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(FlipSGFVerticallyCommand)

public:
    FlipSGFVerticallyCommand(BoardWidget* boardWidget, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
};

class FlipSGFHorizontallyCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(FlipSGFHorizontallyCommand)

public:
    FlipSGFHorizontallyCommand(BoardWidget* boardWidget, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
};


#endif // BOARDWIDGET_H
