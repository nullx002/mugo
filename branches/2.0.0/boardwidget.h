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


class QFileInfo;
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

    struct ResetMoveNumber{
        enum Mode{ noReset, branch, allBranch };
    };

    struct Preference{
        enum ResourceType{ internal, color, file };
    };

    class TerritoryInfo{
        public:
            TerritoryInfo() : color(Go::empty), territory(Go::empty), moveNumber(0), stoneItem(NULL), numberItem(NULL), markItem(NULL), dimItem(NULL), variationItem(NULL){}

            bool isStone() const{ return color != Go::empty; }
            bool isBlack() const{ return color == Go::black; }
            bool isWhite() const{ return color == Go::white; }
            bool isTerritory() const{ return territory != Go::empty; }
            bool isBlackTerritory() const{ return territory != Go::black; }
            bool isWhiteTerritory() const{ return territory != Go::white; }

            Go::Color color;
            Go::Color territory;
            int moveNumber;
            QGraphicsItem* stoneItem;
            QGraphicsSimpleTextItem* numberItem;
            QGraphicsItem* markItem;
            QGraphicsItem* dimItem;
            QGraphicsSimpleTextItem* variationItem;
            QList<GraphicsArrowItem*> lineItemList;
            Go::Mark  mark;
            Go::Mark  dim;
            Go::LineList lineList;
    };
    typedef QVector< QVector<TerritoryInfo> > BoardBuffer;

    BoardWidget(SgfDocument* doc, QWidget *parent = 0);
    ~BoardWidget();

    // coordinate
    void getGridLinePosition(int x, int y, int gridSize, int x1, int y1, int x2, int y2, QLineF& line) const;
    qreal getGridSize() const;
    void sgfToBoardCoordinate(int sgfX, int sgfY, qreal& viewX, qreal& viewY) const;
    void boardToSgfCoordinate(qreal viewX, qreal viewY, int& sgfX, int& sgfY) const;
    void setVCoordinatePosition(int pos, const QRectF& rect, QGraphicsSimpleTextItem* left, QGraphicsSimpleTextItem* right);
    void setHCoordinatePosition(int pos, const QRectF& rect, QGraphicsSimpleTextItem* top, QGraphicsSimpleTextItem* bottom);

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
    ResetMoveNumber::Mode getResetMoveNumberMode() const{ return resetMoveNumberMode; }
    int  getShowMoveNumberCount() const{ return showMoveNumberCount; }
    bool getShowCoordinate() const{ return showCoordinate; }
    bool getShowCoordinateWithI() const{ return document()->showCoordinateWithI; }
    bool getShowMarker() const{ return showMarker; }
    int  getShowVariations() const{ return currentGame->gameInformation->variation; }
    int  getRotate() const{ return rotate; }
    bool getFlipHorizntally() const{ return flipHorizontally; }
    bool getFlipVertically() const{ return flipVertically; }

    // set view mode
    void setShowMoveNumber(bool show);
    void setResetMoveNumberMode(ResetMoveNumber::Mode mode);
    void setShowMoveNumberCount(int cnt);
    void setShowCoordinate(bool show);
    void setShowCoordinateWithI(bool show);
    void setShowMarker(bool show);
    void setShowVariations(int variation);
    void setRotate(int rotate);
    void setFlipHorizontally(bool flip);
    void setFlipVertically(bool flip);

    // set preferences
    void setBoardType(Preference::ResourceType type);
    void setBoardColor(const QColor& color);
    void setBoardImage(const QString& file);
    void setCoordinateColor(const QColor& color);
    void setBackgroundColor(const QColor& color);
    void setWhiteStoneType(Preference::ResourceType type);
    void setWhiteStoneColor(const QColor& color);
    void setWhiteStoneImage(const QString& file);
    void setBlackStoneType(Preference::ResourceType type);
    void setBlackStoneColor(const QColor& color);
    void setBlackStoneImage(const QString& file);

    // add
    void addItem(Go::NodePtr parent, Go::NodePtr node, int index = -1);

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
    void setStoneItemPosition(QGraphicsItem* item, int x, int y, Go::Color color);
    void setMarkItemPosition(QGraphicsItem* item, const Go::Mark& mark);
    void setLineItemPosition(GraphicsArrowItem* item, const Go::Line& line);
    void setTextItemPosition(QGraphicsSimpleTextItem* text, int x, int y);

    // create graphics item
    void createMarkItemList(const Go::MarkList& markList);
    void createLineItemList(const Go::LineList& lineList);
    void createVariationItemList(Go::NodePtr node);
    QGraphicsItem* createStoneItem(int x, int y, Go::Color color);
    void createStonePixmap();
    QGraphicsItem* createMarkItem(const Go::Mark& mark);
    QPainterPath createCrossPath(const Go::Mark& mark);
    QPainterPath createCirclePath(const Go::Mark& mark);
    QPainterPath createSquarePath(const Go::Mark& mark);
    QPainterPath createTrianglePath(const Go::Mark& mark);
    QPainterPath createTerritoryPath(const Go::Mark& mark);
    QPainterPath createSelectPath(const Go::Mark& mark);
    QRectF createRectPath(const Go::Mark& mark);
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
    QPixmap whiteStonePixmap;
    QPixmap blackStonePixmap;
    QList<QGraphicsSimpleTextItem*> coordinateLeft, coordinateRight, coordinateTop, coordinateBottom;
    QList<QGraphicsLineItem*> hLines;
    QList<QGraphicsLineItem*> vLines;
    QList<QGraphicsEllipseItem*> stars;
    QList<QGraphicsItem*> stones;
    QList<QGraphicsSimpleTextItem*> numbers;
    QList<QGraphicsSimpleTextItem*> showNumbers;
    QList<QGraphicsItem*> addStones;
    QList<QGraphicsItem*> marks;
    QList<QGraphicsRectItem*> dims;
    QList<GraphicsArrowItem*> lines;
    QList<QGraphicsSimpleTextItem*> variations;
    BoardBuffer boardBuffer;
    int moveNumber;
    int capturedBlack;
    int capturedWhite;
    EditMode::Mode editMode;
    bool jumpToClicked;
    bool showMoveNumber;
    ResetMoveNumber::Mode resetMoveNumberMode;
    int  showMoveNumberCount;
    bool showCoordinate;
    bool showMarker;
    int  rotate;
    bool flipHorizontally;
    bool flipVertically;

    // preferences
    QColor  boardColor;
    QString boardImage;
    QColor  backgroundColor;
    QColor  coordinateColor;
    Preference::ResourceType whiteStoneType;
    QColor  whiteStoneColor;
    QString whiteStoneImage;
    Preference::ResourceType blackStoneType;
    QColor  blackStoneColor;
    QString blackStoneImage;
};

#endif // BOARDWIDGET_H
