#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QtGui/QWidget>
#include <QVector>
#include <QList>
#include "appdef.h"
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
        stoneInfo() : number(0), color(go::stone::eEmpty){}
        bool empty() const{ return color == go::stone::eEmpty; }
        bool black() const{ return color == go::stone::eBlack; }
        bool white() const{ return color == go::stone::eWhite; }
        int number;
        go::stone::eColor color;
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

    // get node
    go::data& getData(){ return goData; }
    go::node* getCurrentNode(){ return currentNode; }
    const go::nodeList& getCurrentNodeList() const{ return nodeList; }

    // dirty flag
    bool isDirty() const{ return dirty; }
    void setDirty(bool dirty){ this->dirty = dirty; }

    // set option
    void setEditMode(eEditMode editMode){ this->editMode = editMode; }
    void setShowMoveNumber(int number){ showMoveNumber = number; repaint(); }
    void setAnnotation(int annotation){ currentNode->annotation = (go::node::eAnnotation)annotation; modifyNode(currentNode); }
    void setBoardSize(int xsize, int ysize);

    QString getXString(int x) const;
    QString getYString(int y) const;
    QString getXYString(int x, int y) const;

public slots:
    void addNode(go::node* parent, go::node* node);
    void deleteNode(go::node* node);
    void modifyNode(go::node* node);
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
    void drawNext(QPainter& p, go::nodeList::iterator first, go::nodeList::iterator last);
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

    void addStone(int x, int y);
    void addMark(int x, int y);
    void addMark(go::markList& markList, const go::mark& mark);
    void addCharacter(go::markList& markList, const go::point& p);
    void removeMark(go::markList& markList, const go::point& p);
    void addStone(go::stoneList& stoneList, const go::point& p, go::stone::eColor color);

private:
    Ui::BoardWidget *m_ui;

    // data
    bool dirty;
    go::data goData;
    int black;
    go::nodeList nodeList;
    go::node* currentNode;
    int currentMoveNumber;

    // option
    int showMoveNumber;
    eEditMode editMode;

    // draw object
    QImage black1, black2;
    QImage white1, white2;
    QImage boardImage1, boardImage2;

    int width_;
    int height_;
    int boxSize;
    QRect boardRect;
    QList<int> xlines;
    QList<int> ylines;
    QVector< QVector<stoneInfo> > board;
};

#endif // BOARDWIDGET_H
