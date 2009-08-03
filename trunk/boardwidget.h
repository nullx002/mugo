#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QtGui/QWidget>
#include <QVector>
#include <QList>
#include "godata.h"

namespace Ui {
    class BoardWidget;
}

class BoardWidget : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(BoardWidget)
public:
    enum eEditMode{ eAlternateMove, eAddBlack, eAddWhite, eAddEmpty, eLabelMark, eCrossMark, eCircleMark, eSquareMark, eTriangleMark, eDeleteMarker };

    struct stoneInfo{
        stoneInfo() : number(0), color(go::empty), node(NULL){}
        bool empty() const{ return color == go::empty; }
        bool black() const{ return color == go::black; }
        bool white() const{ return color == go::white; }

        int number;
        go::color color;
        go::node* node;
    };

    explicit BoardWidget(QWidget *parent = 0);
    virtual ~BoardWidget();

    // draw
    void paint(QPaintDevice* paintDevice);

    // mouse event
    void onLButtonDown(QMouseEvent* e);
    void onRButtonDown(QMouseEvent* e);

    // set/get data
    void getData(go::fileBase& data);
    void setData(const go::fileBase& data);
    void clear();
    go::node* findNodeFromMoveNumber(int moveNumber);

    // get node
    go::data& getData(){ return goData; }
    go::node* getCurrentNode(){ return currentNode; }
    const go::nodeList& getCurrentNodeList() const{ return nodeList; }

    // dirty flag
    bool isDirty() const{ return dirty; }
    void setDirty(bool dirty){ this->dirty = dirty; }

    // set option
    void setEditMode(eEditMode editMode){ this->editMode = editMode; }
    void setShowMoveNumber(bool visible){ showMoveNumber = visible; repaint(); }
    void setShowMoveNumber(int number){ showMoveNumberCount = number; repaint(); }
    void setShowCoordinates(bool visible){ showCoordinates = visible; repaint(); }
    void setShowCoordinatesWithI(bool withI){ showCoordinatesI = withI; repaint(); }
    void setShowMarker(bool visible){ showMarker = visible; repaint(); }
    void setShowBranchMoves(bool visible){ showBranchMoves = visible; repaint(); }
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

    QString getXString(int x) const;
    QString getYString(int y) const;
    QString getXYString(int x, int y) const;

public slots:
    void addNode(go::node* parent, go::node* node);
    void deleteNode(go::node* node);
    void modifyNode(go::node* node, bool recreateBoardBuffer=false);
    void setCurrentNode(go::node* node=NULL);

signals:
    void nodeAdded(go::node* parent, go::node* node);
    void nodeDeleted(go::node* node);
    void nodeModified(go::node* node);
    void currentNodeChanged(go::node* node);

protected:
    // event
    virtual void changeEvent(QEvent* e);
    virtual void paintEvent(QPaintEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);

    // draw
    void drawBoard(QPainter& p);
    void drawCoordinates(QPainter& p);
    void drawStones(QPainter& p);
    void drawStones2(QPainter& p);
    void drawBranchMoves(QPainter& p, go::nodeList::iterator first, go::nodeList::iterator last);
    void drawMark(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawTerritory(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawCurrentMark(QPainter& p, go::node* node);
    void eraseBackground(QPainter& p, int x, int y);
    void getStartPosition(QList<int>& star, int size);

    // buffer
    void putStone(go::node* n, int moveNumber);
    void removeDeadStones(int x, int y);
    bool isDead(int* tmp, int c, int x, int y);
    bool isDead(int x, int y);
    bool isKill(int x, int y);
    void dead(int* tmp);

    void createNodeList();
    void createBoardBuffer();
    void addStone(int x, int y);
    void addMark(int x, int y);
    void addMark(go::markList& markList, const go::mark& mark);
    void addCharacter(go::markList& markList, const go::point& p);
    void removeMark(go::markList& markList, const go::point& p);
    void addStone(go::stoneList& stoneList, const go::point& p, go::color color);
    void rotateSgf(go::node* node);
    void rotateStoneSgf(go::stoneList& stoneList);
    void rotateMarkSgf(go::markList& markList);
    void flipSgf(go::node* node, int xsize, int ysize);
    void flipStoneSgf(go::stoneList& stoneList, int xsize, int ysize);
    void flipMarkSgf(go::markList& markList, int xsize, int ysize);

    int getBoardX(int x, int y);
    int getBoardY(int x, int y);

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
    QImage black1, black2;
    QImage white1, white2;
    QImage boardImage1, boardImage2;

    int width_;
    int height_;
    int boxSize;
    int xsize;
    int ysize;

    QRect boardRect;
    QList<int> xlines;
    QList<int> ylines;
    QVector< QVector<stoneInfo> > board;
};

#endif // BOARDWIDGET_H
