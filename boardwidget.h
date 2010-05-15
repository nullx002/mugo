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

#include <QtGui/QWidget>
#include <QUndoStack>
#include <QLabel>
#include <QVector>
#include <QList>
#include <QProcess>
#include <QTimer>


#if defined(Q_WS_WIN)
#   include <windows.h>
#else
#   include <phonon>
#endif

#include "godata.h"
#include "playgame.h"


namespace Ui {
    class BoardWidget;
}

/**
* class Sound
* play sound using phonon or MCI(windows).
*/
class Sound{
public:
    Sound(QWidget* parent_);
    ~Sound();

    void setCurrentSource(const QString& source);
    void play();

    QObject* parent;

#if defined(Q_WS_WIN)
    double lastClock;
    static QString fileName;
    static MCI_OPEN_PARMSW mopw;
    static MCI_OPEN_PARMSA mopa;
    OSVERSIONINFO versionInfo;
#else
    Phonon::MediaObject* media;
#endif
};

/**
* class BoardWidget
* Qt Widget for goban.
*/
class BoardWidget : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(BoardWidget)
public:
    enum eEditMode{ eAlternateMove, eAddBlack, eAddWhite, eAddEmpty, eLabelMark, eManualMark, eCrossMark, eCircleMark, eSquareMark, eTriangleMark, eDeleteMarker, eFinalScore, ePlayGame };
    enum eTutorMode{ eNoTutor, eTutorBothSides, eTutorOneSide, eAutoReplay };
    enum eMoveNumberMode{ eSequential, eResetInBranch, eResetInVariation };

    struct stoneInfo{
        stoneInfo() : number(0), color(go::empty), dim(false){}
        bool empty() const{ return (color & (go::black | go::white)) == 0; }
        bool black() const{ return color & go::black; }
        bool white() const{ return color & go::white; }
        bool territory() const{ return color & (go::blackTerritory | go::whiteTerritory); }
        bool blackTerritory() const{ return color & go::blackTerritory; }
        bool whiteTerritory() const{ return color & go::whiteTerritory; }
        bool dame() const{ return color & go::dame; }

        int number;
        int color;
        bool dim;
        go::nodePtr node;
    };
    typedef QVector< QVector<stoneInfo> > BoardBuffer;


    explicit BoardWidget(QWidget *parent = 0);
    virtual ~BoardWidget();

    // undo stack
    QUndoStack* getUndoStack(){ return &undoStack; }

    // preference
    void readSettings();

    // property
//    bool isReadOnly() const{ return readOnly; }
//    void setReadOnly(bool v){ readOnly = v; }

    // draw
    void paintBoard();
    void paintBoard(QPaintDevice* pd, bool showCoordinate, bool monochrome);

    // print
    void print(QPrinter& printer, QPainter& p, BoardBuffer& buf);
    void setPrintOption(int type, int movesPerPage, bool showCoordinate, bool includeComments, const QFont& font, const QString& fileName, const QString& headerLeftFormat, const QString& headerCenterFormat, const QString& headerRightFormat, const QString& footerLeftFormat, const QString& footerCenterFormat, const QString& footerRightFormat);

    // set/get data
    void clear();
    void getData(go::fileBase& data);
    void setData(const go::fileBase& data);
    void addData(const go::fileBase& data);
    void insertData(const go::nodePtr node, const go::fileBase& data);
    void setRoot(go::informationPtr& info);

    // get node
    go::data& getData(){ return goData; }
    const go::data& getData() const{ return goData; }
    go::nodePtr getCurrentNode(){ return currentNode; }
    go::nodePtr findNodeFromMoveNumber(int moveNumber);
    const go::nodeList& getCurrentNodeList() const{ return nodeList; }
    BoardBuffer& getBuffer(){ return board; }
    void getCaptured(int& black, int& white) const{ black = capturedBlack; white = capturedWhite; }
    int  getMoveNumber() const{ return currentMoveNumber; }
    bool forward(int n);
    go::color getColor() const{ return color; }

    // command
    void undo();

    // create stone node and insert after current node
    void addStoneNodeCommand(int sgfX, int sgfY);
    void insertStoneNodeCommand(int index, int sgfX, int sgfY);
    bool moveNextStone(int sgfX, int sgfY);

    void addNodeCommand(go::nodePtr parent, go::nodePtr node, bool select=true);
    void insertNodeCommand(go::nodePtr parent, int index, go::nodePtr node, bool select=true);
    void deleteNodeCommand(go::nodePtr node, bool deleteChildren=true);
    void setMoveNumberCommand(go::nodePtr node, int moveNumber);
    void unsetMoveNumberCommand(go::nodePtr node);
    void setNodeNameCommand(go::nodePtr node, const QString& nodeName);
    void setCommentCommand(go::nodePtr node, const QString& comment);
    void rotateSgfCommand();
    void flipSgfHorizontallyCommand();
    void flipSgfVerticallyCommand();

    void addNode(go::nodePtr parent, go::nodePtr node, bool select=true);
    void insertNode(go::nodePtr parent, int index, go::nodePtr node, bool select=true);
    void deleteNode(go::nodePtr node, bool deleteChildren=true);
    void modifyNode(go::nodePtr node, bool recreateBoardBuffer=false);
    void pass();
    void setCurrentNode(go::nodePtr node = go::nodePtr());
    void addStone(go::nodePtr node, const go::point& sgfPoint, go::color color);
    void addStone(go::nodePtr node, const go::point& sgfPoint, const go::point& boardPoint, go::color color);
    void addEmpty(go::nodePtr node, const go::point& sgfPoint);
    void addEmpty(go::nodePtr node, const go::point& sgfPoint, const go::point& boardPoint);

    // dirty flag
    bool isDirty() const{ return dirty; }
    void setDirty(bool dirty){ this->dirty = dirty; }

    // set option
    void setEditMode(eEditMode editMode){ this->editMode = backupEditMode = editMode; }
    void setTutorMode(eTutorMode tutorMode){ this->tutorMode = tutorMode; paintBoard(); }
    void setShowMoveNumber(bool visible){ showMoveNumber = visible; paintBoard(); }
    void setShowMoveNumberCount(int number){ showMoveNumberCount = number; paintBoard(); }
    void setMoveNumberMode(eMoveNumberMode mode){ moveNumberMode = mode; createBoardBuffer(); paintBoard(); }
    void setShowCoordinates(bool visible){ showCoordinates = visible; paintBoard(); }
    void setShowCoordinatesWithI(bool withI){ showCoordinatesI = withI; paintBoard(); }
    void setShowMarker(bool visible){ showMarker = visible; paintBoard(); }
    void setShowBranchMoves(bool visible){ showBranchMoves = visible; paintBoard(); }
    void setAnnotation(int annotation){ currentNode->annotation = annotation; modifyNode(currentNode); }
    void setMoveAnnotation(int annotation){ currentNode->moveAnnotation = annotation; modifyNode(currentNode); }
    void setNodeAnnotation(int annotation){ currentNode->nodeAnnotation = annotation; modifyNode(currentNode); }
    void setBoardSize(int xsize, int ysize);
    void setMoveToClicked(bool moveMode = true){ moveToClicked = moveMode; }
    int  rotateBoard();
    void flipBoardHorizontally(bool flip);
    void flipBoardVertically(bool flip);
    void resetBoard();
    void setPlaySound(bool play){ playSound = play; }
    void setStoneSoundPath(const QString& path){ stoneSound.setCurrentSource(path); }
    void setFinalScoreMode(bool mode);
    void whiteFirst(bool whiteFirst);

    // get option
    eEditMode  getEditMode() const{ return editMode; }
    eTutorMode getTutorMode() const{ return tutorMode; }
    bool getShowMoveNumber() const{ return showMoveNumber; }
    int  getShowMoveNumberCount() const{ return showMoveNumberCount; }
    eMoveNumberMode getMoveNumberMode() const{ return moveNumberMode; }
    bool getShowCoordinates() const{ return showCoordinates; }
    bool getShowCoordinatesWithI() const{ return showCoordinatesI; }
    bool getShowMarker() const{ return showMarker; }
    bool getShowBranchMoves() const{ return showBranchMoves; }
    int  getRotateBoard() const{ return rotateBoard_; }
    bool getFlipBoardHorizontally() const{ return flipBoardHorizontally_; }
    bool getFlipBoardVertically() const{ return flipBoardVertically_; }
    bool whiteFirst() const{ return goData.root->nextColor == go::white; }

    void createBoardBuffer();
    QString toString(go::nodePtr node) const;
    QString getXString(int x) const;
    QString getXString(int x, bool showI) const;
    QString getYString(int y) const;
    QString getXYString(int x, int y) const;
    QString getXYString(int x, int y, bool showI) const;
    void boardToSgfCoordinate(int boardX, int boardY, int& sgfX, int& sgfY);
    void sgfToBoardCoordinate(int sgfX, int sgfY, int& boardX, int& boardY);
    void reverseTerritory(int x, int y);

    void playWithComputer(PlayGame* game);
    void autoReplay();
    bool isAutoReplay() const{ return autoReplayTimer.isActive(); }
    void estimateScore();

public slots:
    void print(QPrinter* printer);
    void autoReplayTimer_timeout();  //< auto replay

signals:
    void cleared();
    void nodeAdded(go::nodePtr parent, go::nodePtr node, bool select=false);
    void nodeDeleted(go::nodePtr node, bool deleteChildren);
    void nodeModified(go::nodePtr node);
    void currentNodeChanged(go::nodePtr node);
    void updateTerritory(int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory, double komi);
    void automaticReplayEnded();

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
    void playGameLButtonDown(int sgfX, int sgfY);

    // draw
    void drawBoard(QPainter& p, qreal pointSize, bool showCoordinates);
    void drawBoardImage(QPainter& p, bool showCoordinates);
    void drawCoordinates(QPainter& p, bool showCoordinates);
    void drawStonesAndMarkers(QPainter& p);
    void drawStones(QPainter& p);
    void drawBranchMoves(QPainter& p, go::nodeList::iterator first, go::nodeList::iterator last);
    void drawCross(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawTriangle(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawCircle(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawSquare(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawSelect(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawCharacter(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawMark(QPainter& p, const QPainterPath& path, go::markList::iterator first, go::markList::iterator last, bool fill=false);
    void drawPath(QPainter& p, const QPainterPath& path, int boardX, int boardY);
    void fillPath(QPainter& p, const QPainterPath& path, int boardX, int boardY);
    void drawTerritories(QPainter& p);
    void drawCurrentMark(QPainter& p, go::nodePtr node);
    void drawStone(QPainter& p, int boardX, int boardY, go::color, qreal opacity=1.0);
    void drawDim(QPainter& p, int boardX, int boardY);
    void eraseImage(QPainter& p, int boardX, int boardY);
    void getStartPosition(QList<int>& star, int size);
    QPainterPath createFocusTrianglePath() const;
    QPainterPath createCirclePath() const;
    QPainterPath createCrossPath() const;
    QPainterPath createSquarePath() const;
    QPainterPath createTrianglePath() const;

    // print
    void printNodeList(QPrinter& printer, QPainter& p, const go::nodeList& node, int& page, int& fig, int& startNumber, int& endNumber, int& moveNumberInPage, BoardBuffer& buf, QString& rangai, QStringList& comments);
    void printBranch(QPrinter& printer, QPainter& p, go::nodePtr node, int& page, int& fig, int& startNumber, int& endNumber, int& moveNumberInPage, BoardBuffer& buf, QString& rangai, QStringList& comments);
    void printNode(QPrinter& printer, QPainter& p, go::nodePtr node, int page, int& moveNumber, int& moveNumberInPage, BoardBuffer& buf, QString& rangai, QStringList& comments);
    void printHeader(QPrinter& printer, QPainter& p, int& page);
    void printFooter(QPrinter& printer, QPainter& p, int& page);
    void printTitle(QPrinter& printer, QPainter& p, int& page);
    void printCaption(QPrinter& printer, QPainter& p, int& fig, int startNumber, int endNumber, bool draw);
    void printRangai(QPrinter& printer, QPainter& p, int& page, int& fig, int& startNumber, int& endNumber, int& moveNumberInPage, QString& rangai, QStringList& comments);
    void printBoard(QPrinter& printer, QPainter& p, BoardBuffer& buf, int& page, int& fig);
    void newPage(QPrinter& printer, QPainter& p, int& page, int& fig, int& moveNumberInPage);

    // buffer
    void putStone(go::nodePtr n, int moveNumber);
    void putDim(go::nodePtr node);
    void removeDeadStones(int x, int y);
    bool isDead(int* tmp, int c, int x, int y);
    bool isDead(int x, int y);
    bool isKill(int x, int y);
    void dead(int* tmp);

    // Score
    void setTerritory(int x, int y, int c);
    void unsetTerritory(int x, int y);
    bool isDame(int x, int  y);
    bool isDame(int c, int x1, int  y1, int x2, int  y2);

    // Final Score
    void finalScore();
    void whichTerritory(int x, int y, char* tmp, int& c);
    void getFinalScore(int& alive_b, int& alive_w, int& dead_b, int& dead_w, int& bt, int& wt);
    bool hasTerritory(int x1, int  y1, int x2, int  y2);
    bool hasTerritory(go::color c1, go::color c2, char* tmp, int x, int  y);

    void setParent(go::nodePtr& parent, go::nodeList& childNodes);
    void createNodeList();
    void addMark(int sgfX, int sgfY, int boardX, int boardY, bool ctrl);
    void addMark(go::markList& markList, const go::mark& mark);
    void addCharacter(go::markList& markList, const go::point& p);
    void addManualEntry(go::markList& markList, const go::point& p);
    bool removeMark(go::markList& markList, const go::point& p);
    bool removeStone(go::stoneList& stoneList, const go::point& sp, const go::point& bp);
    void rotateSgf(go::nodePtr node, QUndoCommand* command);
    void rotateStoneSgf(go::nodePtr node, go::stoneList& stoneList, QUndoCommand* command);
    void rotateMarkSgf(go::nodePtr node, go::markList& markList, QUndoCommand* command);
    void flipSgf(go::nodePtr node, int xsize, int ysize, QUndoCommand* command);
    void flipStoneSgf(go::nodePtr node, go::stoneList& stoneList, int xsize, int ysize, QUndoCommand* command);
    void flipMarkSgf(go::nodePtr node, go::markList& markList, int xsize, int ysize, QUndoCommand* command);

private:
    Ui::BoardWidget *m_ui;

    // undo
    QUndoStack undoStack;

    // property
//    bool readOnly;

    // data
    bool dirty;
    go::data goData;
    int capturedBlack;
    int capturedWhite;
    go::color color;
    go::nodeList nodeList;
    go::nodePtr currentNode;
    int currentMoveNumber;

    // option
    int boardType, whiteType, blackType, focusType, labelType;
    bool showMoveNumber;
    int  showMoveNumberCount;
    bool showCoordinates;
    bool showCoordinatesI;
    bool showMarker;
    bool showBranchMoves;
    int  autoReplayInterval;
    eEditMode editMode;
    eEditMode backupEditMode;
    eTutorMode tutorMode;
    bool moveToClicked;
    int  rotateBoard_;
    bool flipBoardHorizontally_;
    bool flipBoardVertically_;
    bool playSound;
    eMoveNumberMode moveNumberMode;

    // draw object
    QPixmap offscreenBuffer1, offscreenBuffer2;
    QPixmap black1, black2;
    QPixmap white1, white2;
    QPixmap boardImage1, boardImage2;
    QColor  boardColor, blackColor, whiteColor, bgColor, tutorColor;
    QColor  focusWhiteColor, focusBlackColor, branchColor;

    int paintWidth;
    int paintHeight;
    int boxSize;
    int xsize;
    int ysize;

    QRect boardRect, coordinatesRect, headerRect, footerRect;
    QList<int> xlines;
    QList<int> ylines;
    BoardBuffer board;

    // sound
    Sound stoneSound;

    // timer
    QTimer autoReplayTimer;

    // play a game
    PlayGame* playGame;

    // print option;
    int printType;
    int printMovesPerPage;
    bool    printShowCoordinate;
    bool    printIncludeComments;
    QFont   printFont;
    QString printFileName;
    QString headerLeftFormat;
    QString headerCenterFormat;
    QString headerRightFormat;
    QString footerLeftFormat;
    QString footerCenterFormat;
    QString footerRightFormat;
};




#endif // BOARDWIDGET_H
