/*
    mugo, sgf editor.
    Copyright (C) 2009-2011 nsase.

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

#include <math.h>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include "godocument.h"

/**
  path item on pixmap
*/
class GraphicsPathItem : public QGraphicsPathItem{
public:
    GraphicsPathItem(QGraphicsItem* parent = 0) : QGraphicsPathItem(parent){}
    GraphicsPathItem(const QPainterPath & path, QGraphicsItem* parent = 0) : QGraphicsPathItem(path, parent){}

    void setBackgroundImage(const QPixmap& pixmap){ backgroundImage_ = pixmap; }
    const QPixmap& backgroundImage() const{ return backgroundImage_; }

protected:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

private:
    QPixmap backgroundImage_;
};

/**
  GraphicsView of Goban
*/
class BoardWidget : public QGraphicsView
{
    Q_OBJECT
public:
    enum EditMode{
        eAlternateMove, eAddBlack, eAddWhite, eAddEmpty, eAddLabel, eAddLabelManually, eAddCircle, eAddCross, eAddTriangle, eAddSquare, eRemoveMarker,
    };

    typedef QSharedPointer<QGraphicsItem> GraphicsItemPtr;
    typedef QSharedPointer<QGraphicsSimpleTextItem> TextItemPtr;

    struct Data{
        Data() : color(Go::eDame), number(-1), focus(false){}

        Go::Color color;
        int number;
        GraphicsItemPtr stoneItem;
        TextItemPtr branchItem;
        TextItemPtr numberItem;
        GraphicsItemPtr markerItem;
        bool focus;
    };

    // constructor
    explicit BoardWidget(QWidget *parent = 0);
    explicit BoardWidget(GoDocument* doc, QWidget *parent = 0);

    // document
    const GoDocument* document() const{ return document_; }
    GoDocument* document(){ return document_; }

    // get current game, node
    Go::NodePtr currentGame() const{ return currentGame_; }
    Go::NodePtr currentNode() const{ return currentNode_ ? currentNode_ : currentGame_; }
    Go::NodeList currentNodeList() const{ return currentNodeList_; }
    Go::InformationPtr rootInformation() const{ return currentGame_->information(); }
    Go::InformationPtr currentInformation() const{ return currentInformation_; }

    // get board size
    int xsize() const { return currentGame_->information()->xsize(); }
    int ysize() const { return currentGame_->information()->ysize(); }

    // Edit Mode
    EditMode editMode() const{ return editMode_; }
    void setEditMode(EditMode mode){ editMode_ = mode; }

    // Move Number
    int moveNumber() const{ return moveNumber_; }
    void setMoveNumber(int number){ moveNumber_ = number; }

    // Captured stones
    int capturedWhite() const{ return capturedWhite_; }
    int capturedBlack() const{ return capturedBlack_; }

    // Coordinate
    int showCoordinate() const{ return showCoordinate_; }
    void setShowCoordinate(bool show){ showCoordinate_ = show; }
    int showCoordinateI() const{ return showCoordinateI_; }
    void setShowCoordinateI(bool show){ showCoordinateI_ = show; }

signals:
    void documentChanged(GoDocument* doc);
    void gameChanged(const Go::NodePtr& game);
    void informationChanged(const Go::InformationPtr& information);
    void nodeChanged(const Go::NodePtr& node);

public slots:
    bool setDocument(GoDocument* doc);
    bool setGame(const Go::NodePtr& game);
    bool setInformation(const Go::InformationPtr& information);
    bool setNode(const Go::NodePtr& node, bool forceChange=false);

protected:
    // event
    void resizeEvent(QResizeEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);
    void onLButtonUp(QMouseEvent* e);
    void onRButtonUp(QMouseEvent* e);

    // initialize
    void initialize();

    // set graphics items position
    void setItemsPosition(const QSize& size);
    void setVLinesPosition(int x, int y, int gridSize);
    void setHLinesPosition(int x, int y, int gridSize);
    void setStarsPosition();
    void setDataPosition();

    // get items position
    qreal getGridSize() const{ return fabs(vLines[1]->line().x1() - vLines[0]->line().x1()); }

    // create buffer
    void createBoardBuffer();
    void killStone(int x, int y);
    void killStone(int x, int y, Go::Color color);
    bool isDeadStone(int x, int y);
    bool isDeadStone(int x, int y, Go::Color color);
    bool isDeadStone(int x, int y, Go::Color color, QVector< QVector<bool> >& checked);
    bool isKillStone(int x, int y);
    void createNodeList(Go::NodePtr node);

    // create stone item
    QGraphicsItem* createStoneItem(Go::Color color, int sgfX, int sgfY);

    // create branch markers
    void createBranchMarkers();
    void createChildBranchMarkers();
    void createSiblingsranchMarkers();

    // create move number
    void createMoveNumber(int sgfX, int sgfY, int number, bool active);

    // create marker
    void createMarkers();
    void createMark(const Go::Mark& m);
    QPixmap createBackgroundImage(const QRectF& r);

    // get star positions
    void getStarPositions(QList<int>& xstarpos, QList<int>& ystarpos) const;

    // transform coordinates
    bool viewToSgfCoordinate(qreal viewX, qreal viewY, int& sgfX, int& sgfY) const;
    bool sgfToViewCoordinate(int sgfX, int sgfY, qreal& viewX, qreal& viewY) const;

    // stone
    bool alternateMove(int sgfX, int sgfY);
    bool addStone(int sgfX, int sgfY, Go::Color color);
    bool removeStone(int sgfX, int sgfY, QList<QPoint>& stones);
    void back(int step=1);
    void forward(int step=1);

    // mark
    void addMark(int sgfX, int sgfY, Go::Mark::Type mark);
    void addLabel(int sgfX, int sgfY);
    void addLabelManually(int sgfX, int sgfY);
    bool removeMark(int sgfX, int sgfY);
    bool removeMark(int sgfX, int sgfY, Go::MarkList& markList);

    GoDocument* document_;
    Go::NodePtr currentGame_;
    Go::InformationPtr currentInformation_;
    Go::NodePtr currentNode_;
    Go::NodeList currentNodeList_;
    int capturedWhite_;
    int capturedBlack_;

    QVector< QVector<Data> > data;
    QVector< QVector<Go::Color> > buffer;

    QGraphicsItem* board;
    QGraphicsRectItem* shadow;
    QList<QGraphicsLineItem*> vLines;
    QList<QGraphicsLineItem*> hLines;
    QList<QGraphicsEllipseItem*> stars;
    QGraphicsItem* blackStone_;
    QGraphicsItem* whiteStone_;

    Go::Color nextColor_;
    EditMode editMode_;
    int moveNumber_;
    bool showMoveNumber_;
    bool showCoordinate_;
    bool showCoordinateI_;
    bool showVariation_;

private slots:
    void on_document_nodeAdded(const Go::NodePtr& game, const Go::NodePtr& node);
    void on_document_nodeDeleted(const Go::NodePtr& game, const Go::NodePtr& node, bool removeChildren);
    void on_document_nodeModified(const Go::NodePtr& game, const Go::NodePtr& node);
};

#endif // BOARDWIDGET_H
