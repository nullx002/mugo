#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QtGui/QWidget>
#include <QLabel>
#include <QVector>
#include <QList>
//#include <Phonon/phonon>
#include "godata.h"

namespace Ui {
    class BoardWidget;
}

class BoardWidget : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(BoardWidget)
public:
    enum eEditMode{ eAlternateMove, eAddBlack, eAddWhite, eAddEmpty, eLabelMark, eCrossMark, eCircleMark, eSquareMark, eTriangleMark, eDeleteMarker, eCountTerritory };

    struct stoneInfo{
        stoneInfo() : number(0), color(go::empty), node(NULL){}
        bool empty() const{ return color == go::empty; }
        bool black() const{ return color & go::black; }
        bool white() const{ return color & go::white; }
        bool blackTerritory() const{ return color & go::blackTerritory; }
        bool whiteTerritory() const{ return color & go::whiteTerritory; }

        int number;
        int color;
        go::node* node;
    };

    explicit BoardWidget(QWidget *parent = 0);
    virtual ~BoardWidget();

    // draw
    void repaintBoard(bool board=true, bool stones=true);
    void paintBoard(QPaintDevice* pd);
    void paintStones(QPaintDevice* pd);
    void paintTerritories(QPaintDevice* pd);

    // set/get data
    void getData(go::fileBase& data);
    void setData(const go::fileBase& data);
    void clear();
    go::node* findNodeFromMoveNumber(int moveNumber);

    // get node
    go::data& getData(){ return goData; }
    go::node* getCurrentNode(){ return currentNode; }
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
    void setStoneSoundPath(const QString& path){ stoneSoundPath = path; }
    void setCountTerritoryMode(bool countMode);

    QString getXString(int x) const;
    QString getYString(int y) const;
    QString getXYString(int x, int y) const;

public slots:
    void addNode(go::node* parent, go::node* node, bool select=true);
    void deleteNode(go::node* node);
    void modifyNode(go::node* node, bool recreateBoardBuffer=false);
    void setCurrentNode(go::node* node=NULL);

signals:
    void nodeAdded(go::node* parent, go::node* node, bool select=false);
    void nodeDeleted(go::node* node);
    void nodeModified(go::node* node);
    void currentNodeChanged(go::node* node);
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
    void drawCurrentMark(QPainter& p, go::node* node);
    void drawImage(QPainter& p, int x, int y, const QImage& image);
    void eraseBackground(QPainter& p, int x, int y);
    void getStartPosition(QList<int>& star, int size);

    // buffer
    void putStone(go::node* n, int moveNumber);
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
    void createBoardBuffer();
    void addStone(int sgfX, int sgfY, int boardX, int boardY);
    void addMark(int sgfX, int sgfY, int boardX, int boardY);
    void addMark(go::markList& markList, const go::mark& mark);
    void addCharacter(go::markList& markList, const go::point& p);
    void removeMark(go::markList& markList, const go::point& p);
    void addStone(go::node* node, const go::point& sgfP, const go::point& boardP, go::color color);
    void rotateSgf(go::node* node);
    void rotateStoneSgf(go::stoneList& stoneList);
    void rotateMarkSgf(go::markList& markList);
    void flipSgf(go::node* node, int xsize, int ysize);
    void flipStoneSgf(go::stoneList& stoneList, int xsize, int ysize);
    void flipMarkSgf(go::markList& markList, int xsize, int ysize);

    void boardToSgfCoordinate(int boardX, int boardY, int& sgfX, int& sgfY);
    void sgfToBoardCoordinate(int sgfX, int sgfY, int& boardX, int& boardY);

private:
    Ui::BoardWidget *m_ui;

    // data
    bool dirty;
    go::data goData;
    int capturedBlack;
    int capturedWhite;
    bool isBlack;
    go::nodeList nodeList;
    go::node* currentNode;
    int currentMoveNumber;

    // option
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

    int width_;
    int height_;
    int boxSize;
    int xsize;
    int ysize;

    QRect boardRect;
    QList<int> xlines;
    QList<int> ylines;
    QVector< QVector<stoneInfo> > board;

    // Phonon
//    Phonon::MediaObject* mediaObject;
};

#endif // BOARDWIDGET_H
