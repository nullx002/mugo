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
#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QGraphicsView>
#include "godata.h"
#include "sgfdocument.h"


class GraphicsArrowItem;



/**
  BoardWidget
*/
class BoardWidget : public QGraphicsView {
    Q_OBJECT
public:
    struct EditMode{
        enum Mode{ alternateMove, addBlack, addWhite, addEmpty, addLabel, addLabelManually, addCircle, addCross, addTriangle, addSquare, removeMarker };
    };

    class TerritoryInfo{
        public:
            TerritoryInfo() : color(Go::empty), territory(Go::empty), moveNumber(0), stone(NULL), mark(NULL), dim(NULL), number(NULL){}

            bool isStone() const{ return color != Go::empty; }
            bool isBlack() const{ return color == Go::black; }
            bool isWhite() const{ return color == Go::white; }
            bool isTerritory() const{ return territory != Go::empty; }
            bool isBlackTerritory() const{ return territory != Go::black; }
            bool isWhiteTerritory() const{ return territory != Go::white; }

            Go::Color color;
            Go::Color territory;
            int moveNumber;
            QGraphicsItem* stone;
            QGraphicsItem* mark;
            QGraphicsItem* dim;
            QGraphicsSimpleTextItem* number;

    };
    typedef QVector< QVector<TerritoryInfo> > BoardBuffer;

    BoardWidget(SgfDocument* doc, QWidget *parent = 0);
    ~BoardWidget();


    // get
    const SgfDocument* document() const{ return document_; }
    SgfDocument* document(){ return document_; }
    const Go::NodePtr& getCurrentGame() const{ return currentGame; }
    const Go::NodePtr& getCurrentNode() const{ return currentNode; }
    const Go::NodeList& getCurrentNodeList() const{ return currentNodeList; }
    int getMoveNumber() const{ return moveNumber; }
    int getCapturedBlack() const{ return capturedBlack; }
    int getCapturedWhite() const{ return capturedWhite; }
    BoardBuffer& getBoardBuffer(){ return boardBuffer; }
    Go::Color getNextColor() const;

    // set
    void setDocument(SgfDocument* doc);
    void setCurrentGame(Go::NodePtr node, bool forceChange=false);
    void setCurrentNode(Go::NodePtr node, bool forceChange=false);
    void forward(int step=1);
    void back(int step=1);

    // get edit mode
    EditMode::Mode getEditMode() const{ return editMode; }
    bool isJumpToClicked() const{ return jumpToClicked; }

    // set edit mode
    void setEditMode(EditMode::Mode editMode);
    void setJumpToClicked(bool flag){ jumpToClicked = flag; }

    // get view mode
    bool getShowMoveNumber() const{ return showMoveNumber; }
    bool getResetMoveNumberInBranch() const{ return resetMoveNumberInBranch; }
    int  getShowMoveNumberCount() const{ return showMoveNumberCount; }
    bool getShowCoordinate() const{ return showCoordinate; }
    bool getShowCoordinateWithI() const{ return document()->showCoordinateWithI; }

    // set view mode
    void setShowMoveNumber(bool show);
    void setResetMoveNumberInBranch(bool reset);
    void setShowMoveNumberCount(int cnt);
    void setShowCoordinate(bool show);
    void setShowCoordinateWithI(bool show);

    // add
    void addItem(Go::NodePtr parent, Go::NodePtr node, int index);

signals:
    void currentGameChanged(Go::NodePtr currentGame);
    void currentNodeChanged(Go::NodePtr currentNode);

protected:
    // event
    void resizeEvent(QResizeEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);
    void onLButtonDown(QMouseEvent* e);
    void onRButtonDown(QMouseEvent* e);

    // create board
    void createBoard();

    // create buffer
    void createBuffer(bool erase);
    void eraseBuffer();

    // set graphics item position
    void setItemsPosition();
    void setStoneItemPosition(QGraphicsItem* item, int x, int y);
    void setMarkItemPosition(QGraphicsItem* item, const Go::Mark& mark);
    void setLineItemPosition(GraphicsArrowItem* item, const Go::Line& mark);
    void setTextItemPosition(QGraphicsSimpleTextItem* text, int x, int y);

    // create graphics item
    QGraphicsItem* createStoneItem(int x, int y, Go::Color color);
    QGraphicsItem* createMarkItem(const Go::Mark& mark);
    QPainterPath createCrossPath(const Go::Mark& mark);
    QPainterPath createCirclePath(const Go::Mark& mark);
    QPainterPath createSquarePath(const Go::Mark& mark);
    QPainterPath createTrianglePath(const Go::Mark& mark);
    QPainterPath createTerritoryPath(const Go::Mark& mark);
    QPainterPath createSelectPath(const Go::Mark& mark);
    GraphicsArrowItem* createLineItem(const Go::Line& line);
    TerritoryInfo& addStoneToBuffer(int x, int y, Go::Color color, int moveNumber, QGraphicsItem* stone, QGraphicsSimpleTextItem* number);
    TerritoryInfo& addMarkToBuffer(const Go::Mark& mark, QGraphicsItem* item);
    TerritoryInfo& removeMarkFromBuffer(const Go::Mark& mark, QGraphicsItem* item);
    void addLineToBuffer(const Go::Line& line, GraphicsArrowItem* item);
    void removeLineFromBuffer(const Go::Line& line, GraphicsArrowItem* item);

    // add stone or marker
    void alternateMove(int x, int y);
    void addStone(int x, int y, Go::Color color);
    void addLabel(int x, int y, bool autoLabel);
    void addMarker(int x, int y, Go::Mark::Type mark);
    void removeMarker(int x, int y);
    bool removeStone(const Go::StoneList& stoneList, const Go::Point& p);

    void getStarPosition(QList<int>& xpos, QList<int>& ypos);
    void killStones(int x, int y);
    void killStones(char* buf);
    bool canKillStones(int x, int y);
    bool isDeadStones(int x, int y);
    bool isDeadStones(int x, int y, Go::Color color, char* buf);
    bool inBoard(Go::NodePtr node);
    bool inBoard(int x, int y);
    bool moveToChildItem(int x, int y);
    bool createChildItem(int x, int y);

private slots:
    // Document
//    void on_sgfdocument_nodeAdded(Go::NodePtr node);
    void on_sgfdocument_nodeDeleted(Go::NodePtr node, bool removeChild);
    void on_sgfdocument_nodeModified(Go::NodePtr node, bool needRecreateBoard);
    void on_sgfdocument_gameModified(Go::NodePtr game);

private:
    SgfDocument* document_;
    Go::NodePtr  currentGame;
    Go::GameInformationPtr gameInformation;
    Go::NodePtr currentNode;
    Go::NodeList currentNodeList;
    QGraphicsScene* scene;
    QGraphicsRectItem* board;
    QGraphicsRectItem* shadow;
    QList<QGraphicsSimpleTextItem*> coordinateLeft, coordinateRight, coordinateTop, coordinateBottom;
    QList<QGraphicsLineItem*> hLines;
    QList<QGraphicsLineItem*> vLines;
    QList<QGraphicsEllipseItem*> stars;
    QList<QGraphicsItem*> stones;
    QList< QList<QGraphicsItem*> > blackStones;
    QList< QList<QGraphicsItem*> > whiteStones;
    QList< QList<QGraphicsItem*> > marks;
    QList< QList<QGraphicsItem*> > territories;
    QList< QList<QGraphicsRectItem*> > dims;
    QList< QList<GraphicsArrowItem*> > lines;
    QList<QGraphicsSimpleTextItem*> numbers;
    QList<QGraphicsSimpleTextItem*> showNumbers;
    BoardBuffer boardBuffer;
    int moveNumber;
    int capturedBlack;
    int capturedWhite;
    EditMode::Mode editMode;
    bool jumpToClicked;
    bool showMoveNumber;
    bool resetMoveNumberInBranch;
    int  showMoveNumberCount;
    bool showCoordinate;
};

#endif // BOARDWIDGET_H
