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
class QAbstractGraphicsShapeItem;
class GraphicsArrowItem;
class Sound;
class GameInterface;


/**
  BoardWidget
*/
class BoardWidget : public QGraphicsView {
    Q_OBJECT
public:
    struct EditMode{
        enum Mode{ alternateMove, addBlack, addWhite, addEmpty, addLabel, addLabelManually, addCircle, addCross, addTriangle, addSquare, removeMarker };
    };

    struct TutorMode{
        enum Mode{ noTutor, tutorBothSides, tutorOneSide, replay };
    };

    struct ScoreMode{
        enum Mode{ noScore, final, estimate };
    };

    struct ResetMoveNumber{
        enum Mode{ noReset, branch, allBranch };
    };

    struct Preference{
        enum ResourceType{ internal, color, file };
        enum LabelType{ large, small, number };
    };

    class TerritoryInfo{
        public:
            TerritoryInfo() : color(Go::empty), territory(Go::empty), number(0), moveNumber(0), mark(NULL), dim(NULL), stoneItem(NULL){}

            bool isStone() const{ return color != Go::empty; }
            bool isBlack() const{ return color == Go::black; }
            bool isWhite() const{ return color == Go::white; }
            bool isTerritory() const{ return territory != Go::empty; }
            bool isBlackTerritory() const{ return territory != Go::black; }
            bool isWhiteTerritory() const{ return territory != Go::white; }
            bool isDame() const{ return territory != Go::dame; }

            Go::Color color;
            Go::Color territory;
            int number;
            int moveNumber;
            const Go::Mark* mark;
            const Go::Mark* dim;
            Go::LineList lineList;
            QGraphicsItem* stoneItem;
    };
    typedef QVector< QVector<TerritoryInfo> > BoardBuffer;

    BoardWidget(SgfDocument* doc, QWidget *parent = 0);
    ~BoardWidget();

    // coordinate
    void getGridLinePosition(int x, int y, int gridSize, int x1, int y1, int x2, int y2, QLineF& line) const;
    qreal getGridSize() const;
    void sgfToViewCoordinate(int sgfX, int sgfY, qreal& viewX, qreal& viewY) const;
    void viewToSgfCoordinate(qreal viewX, qreal viewY, int& sgfX, int& sgfY) const;
    void setVCoordinatePosition(int pos, const QRectF& rect, QGraphicsSimpleTextItem* left, QGraphicsSimpleTextItem* right);
    void setHCoordinatePosition(int pos, const QRectF& rect, QGraphicsSimpleTextItem* top, QGraphicsSimpleTextItem* bottom);

    // get
    const SgfDocument* document() const{ return document_; }
    SgfDocument* document(){ return document_; }
    const Go::NodePtr& getCurrentGame() const{ return currentGame; }
    const Go::NodePtr& getCurrentNode() const{ return currentNode; }
    const Go::NodeList& getCurrentNodeList() const{ return currentNodeList; }
    const Go::GameInformationPtr& getGameInformation() const{ return gameInformation; }

    BoardBuffer& getBoardBuffer(){ return boardBuffer; }

    int getMoveNumber() const{ return moveNumber; }
    int getCapturedBlack() const{ return capturedBlack; }
    int getCapturedWhite() const{ return capturedWhite; }
    Go::Color getNextColor() const;

    // set
    void setDocument(SgfDocument* doc);
    void setCurrentGame(Go::NodePtr node, bool forceChange=false);
    void setCurrentNode(Go::NodePtr node, bool forceChange=false);
    void forward(int step=1);
    void back(int step=1);

    // get edit mode
    EditMode::Mode getEditMode() const{ return editMode; }
    TutorMode::Mode getTutorMode() const{ return tutorMode; }
    ScoreMode::Mode getScoreMode() const{ return scoreMode; }
    bool isJumpToClicked() const{ return jumpToClicked; }

    // set edit mode
    void setEditMode(EditMode::Mode mode);
    void setTutorMode(TutorMode::Mode mode);
    void setScoreMode(ScoreMode::Mode mode);
    void setJumpToClicked(bool flag){ jumpToClicked = flag; }

    // get view mode
    bool getShowMoveNumber() const{ return showMoveNumber; }
    ResetMoveNumber::Mode getResetMoveNumberMode() const{ return resetMoveNumberMode; }
    int  getShowMoveNumberCount() const{ return showMoveNumberCount; }
    bool getShowCoordinate() const{ return showCoordinate; }
    bool getShowCoordinateWithI() const{ return document()->showCoordinateWithI; }
    bool getShowMarker() const{ return showMarker; }
    int  getShowVariations() const{ return currentGame->gameInformation->variation; }
    bool getMonochrome() const{ return monochrome; }
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
    void setMonochrome(bool monochrome);
    void setRotate(int rotate);
    void setFlipHorizontally(bool flip);
    void setFlipVertically(bool flip);

    // set preferences
    void setBoardType(Preference::ResourceType type);
    void setBoardColor(const QColor& color);
    void setBoardImage(const QString& file);
    void setCoordinateColor(const QColor& color);
    void setCoordinateFont(const QString& fontName);
    void setBackgroundColor(const QColor& color);
    void setTutorBackgroundColor(const QColor& color);
    void setWhiteStoneType(Preference::ResourceType type);
    void setWhiteStoneColor(const QColor& color);
    void setWhiteStoneImage(const QString& file);
    void setBlackStoneType(Preference::ResourceType type);
    void setBlackStoneColor(const QColor& color);
    void setBlackStoneImage(const QString& file);
    void setBranchColor(const QColor& color);
    void setFocusColor(const QColor& color);
    void setFocusType(int type);
    void setLabelType(Preference::LabelType type);
    void setLabelFont(const QString& fontName);
    void setAutomaticReplayInterval(int interval);
    void setMoveSoundFile(const QString& file);
    void setPlaySound(bool play);

    // get preferences
    bool isPlaySound() const{ return playSound; }

    // add
    void addItem(Go::NodePtr parent, Go::NodePtr node, int index = -1);

    // draw image
    void drawImage(QImage& image);

    // play game
    void play(GameInterface* game);

signals:
    void currentGameChanged(Go::NodePtr currentGame);
    void currentNodeChanged(Go::NodePtr currentNode);
    void scoreUpdated(int total, int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory);

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
    void createBoardItemList();
    void createStoneItemList();
    void createNumberItemList();
    void createMarkItemList();

    // create territory
    void createTerritories();
    void setTerritories(int x, int y);
    void setTerritories(char* buf, Go::Color color);
    void getTerritory(int x, int y, char* buf, bool& black, bool& white);
    void setDeadStones(int x, int y);
    void setDeadStones(int x, int y, Go::Color color, Go::Color territory, char* buf);

    // set graphics item position
    void setItemsPosition();
    void setItemsPosition(const QRectF& r);
    void setTextItemPosition(QGraphicsSimpleTextItem* text, int x, int y);

    // create graphics item
    void createVariationItemList(Go::NodePtr node);
    QAbstractGraphicsShapeItem* createFocusItem(int x, int y);
    QGraphicsItem* createStoneItem(int x, int y, Go::Color color);
    void createStonePixmap();
    QGraphicsSimpleTextItem* createMoveNumberItem(int x, int y, int number);
    QGraphicsItem* createMarkItem(const Go::Mark& mark);
    QGraphicsItem* createTerritoryItem(int x, int y, Go::Color color);
    QPainterPath createCrossPath(int x, int y);
    QPainterPath createCirclePath(int x, int y);
    QPainterPath createSquarePath(int x, int y);
    QPainterPath createTrianglePath(int x, int y);
    QPainterPath createTerritoryPath(int x, int y);
    QPainterPath createSelectPath(int x, int y);
    QRectF createRectPath(int x, int y);
    GraphicsArrowItem* createLineItem(int x1, int y1, int x2, int y2, Go::Line::Type type);

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
    void on_automaticReplay_timeout();
    void on_tutorOneSide_timeout();

private:
    // document / node
    SgfDocument* document_;
    Go::NodePtr  currentGame;
    Go::NodePtr currentNode;
    Go::NodeList currentNodeList;
    Go::GameInformationPtr gameInformation;

    // graphics view
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
    QList<QGraphicsItem*> marks;
//    QList<QGraphicsRectItem*> dims;
//    QList<GraphicsArrowItem*> lines;
    QList<QGraphicsSimpleTextItem*> variations;

    // board buffer
    BoardBuffer boardBuffer;
    int number;
    int moveNumber;
    int capturedBlack;
    int capturedWhite;

    // edit mode
    EditMode::Mode editMode;
    TutorMode::Mode tutorMode;
    ScoreMode::Mode scoreMode;
    bool jumpToClicked;

    // view options
    bool showMoveNumber;
    ResetMoveNumber::Mode resetMoveNumberMode;
    int  showMoveNumberCount;
    bool showCoordinate;
    bool showMarker;
    bool monochrome;

    // roate / flip view
    int  rotate;
    bool flipHorizontally;
    bool flipVertically;

    // automatic replay / tutor mode
    QTimer* replayTimer;
    int replayInterval;
    bool moveEnemy;

    // preferences
    Sound* moveSound;
    Preference::ResourceType boardType;
    QColor  boardColor;
    QString boardImage;
    QColor  backgroundColor;
    QColor  tutorBackgroundColor;
    QColor  coordinateColor;
    QString coordinateFont;
    Preference::ResourceType whiteStoneType;
    QColor  whiteStoneColor;
    QString whiteStoneImage;
    Preference::ResourceType blackStoneType;
    QColor  blackStoneColor;
    QString blackStoneImage;
    QColor  branchColor;
    QColor  focusColor;
    int focusType;
    Preference::LabelType labelType;
    QString labelFont;
    bool playSound;
};

#endif // BOARDWIDGET_H