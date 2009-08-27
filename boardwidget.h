#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QtGui/QWidget>
#include <QUndoStack>
#include <QLabel>
#include <QVector>
#include <QList>
#include <QProcess>


#ifdef Q_WS_X11
#   include <phonon>
#else
#   include <QSound>
#   include <windows.h>
#endif

#include "godata.h"


namespace Ui {
    class BoardWidget;
}


class Sound{
public:
    Sound(QWidget* parent_) : parent(parent_){
#ifdef Q_WS_X11
        media = Phonon::createPlayer(Phonon::NotificationCategory);
//        media = new Phonon::MediaObject(parent);
//        audioOutput = new Phonon::AudioOutput(Phonon::NotificationCategory, parent);
//        Phonon::createPath(media, audioOutput);
#elif defined(Q_WS_WIN)
#else
            media = NULL;
#endif
    }
    ~Sound(){
#ifdef Q_WS_X11
        delete media;
#elif defined(Q_WS_WIN)
#else
        delete media;
#endif
    }
    void setCurrentSource(const QString& source){
#ifdef Q_WS_X11
        media->setCurrentSource(source);
#elif defined(Q_WS_WIN)
        filename = source;
        mop.lpstrDeviceType  = L"WaveAudio";
        mop.lpstrElementName = (WCHAR*)filename.utf16();
        mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE|MCI_OPEN_ELEMENT, (DWORD)&mop);
#else
        delete media;
        media = new QSound(source, parent);
#endif
    }

    void play(){
        static double lastClock = 0;
        double currentClock = clock() / (double)CLOCKS_PER_SEC;

#ifdef Q_WS_X11
        if (media->currentTime() == media->totalTime()){
            media->stop();
            media->seek(0);
        }
        if (media && currentClock - lastClock > 0.2){
            media->play();
            lastClock = currentClock;
        }
#elif defined(Q_WS_WIN)
        if (mop.wDeviceID && currentClock - lastClock > 0.2){
            mciSendCommand(mop.wDeviceID, MCI_STOP, 0, 0);
            mciSendCommand(mop.wDeviceID, MCI_SEEK, MCI_SEEK_TO_START, 0);
            mciSendCommand(mop.wDeviceID, MCI_PLAY, 0, 0);
            lastClock = currentClock;
        }
#else
        if (media && media->isFinished() && currentClock - lastClock > 0.2){
            media->play();
            lastClock = currentClock;
        }
#endif
    }

#ifdef Q_WS_X11
    Phonon::MediaObject* media;
//    Phonon::AudioOutput* audioOutput;
#elif defined(Q_WS_WIN)
    MCI_OPEN_PARMS mop;
    QString filename;
#else
    QSound* media;
#endif
    QObject* parent;
};


class BoardWidget : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(BoardWidget)
public:
    enum eEditMode{ eAlternateMove, eAddBlack, eAddWhite, eAddEmpty, eLabelMark, eCrossMark, eCircleMark, eSquareMark, eTriangleMark, eDeleteMarker, eCountTerritory, eGtp };
    enum eGtpStatus{ eGtpNone, eGtpPut, eGtpGen, eGtpHandicap, eGtpGameEnd };

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
    typedef QVector< QVector<stoneInfo> > BoardBuffer;


    explicit BoardWidget(QWidget *parent = 0);
    virtual ~BoardWidget();

    // undo stack
    QUndoStack* getUndoStack(){ return &undoStack; }

    // preference
    void readSettings();

    // draw
    void repaintBoard(bool board=true, bool stones=true);
    void paintBoard(QPaintDevice* pd);
    void paintStones(QPaintDevice* pd);
    void paintTerritories(QPaintDevice* pd);

    // set/get data
    void clear();
    void getData(go::fileBase& data);
    void setData(const go::fileBase& data);
    void insertData(const go::nodePtr node, const go::fileBase& data);

    // get node
    go::data& getData(){ return goData; }
    go::nodePtr getCurrentNode(){ return currentNode; }
    go::nodePtr findNodeFromMoveNumber(int moveNumber);
    const go::nodeList& getCurrentNodeList() const{ return nodeList; }
    const BoardBuffer& getBuffer(){ return board; }

    void getCaptured(int& black, int& white) const{ black = capturedBlack; white = capturedWhite; }
    int  getMoveNumber() const{ return currentMoveNumber; }

    // create stone node and insert after current node
    void addStoneNodeCommand(int sgfX, int sgfY);
    void addStoneNodeCommand(int sgfX, int sgfY, int boardX, int boardY);

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
    void setAnnotation(int annotation){ currentNode->annotation = annotation; modifyNode(currentNode); }
    void setMoveAnnotation(int annotation){ currentNode->moveAnnotation = annotation; modifyNode(currentNode); }
    void setNodeAnnotation(int annotation){ currentNode->nodeAnnotation = annotation; modifyNode(currentNode); }
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
    void whiteFirst(bool whiteFirst);

    void createBoardBuffer();
    QString toString(go::nodePtr node) const;
    QString getXString(int x) const;
    QString getXString(int x, bool showI) const;
    QString getYString(int y) const;
    QString getXYString(int x, int y) const;
    QString getXYString(int x, int y, bool showI) const;

    void playWithComputer(QProcess* proc, bool isYourColorBlack);

public slots:
    void addNodeCommand(go::nodePtr parent, go::nodePtr node, bool select=true);
    void insertNodeCommand(go::nodePtr parent, go::nodePtr node, bool select=true);
    void deleteNodeCommand(go::nodePtr node, bool deleteChildren=true);
    void setMoveNumberCommand(go::nodePtr node, int moveNumber);
    void unsetMoveNumberCommand(go::nodePtr node);
    void setNodeNameCommand(go::nodePtr node, const QString& nodeName);
    void setCommentCommand(go::nodePtr node, const QString& comment);
    void addNode(go::nodePtr parent, go::nodePtr node, bool select=true);
    void deleteNode(go::nodePtr node, bool deleteChildren=true);
    void modifyNode(go::nodePtr node, bool recreateBoardBuffer=false);
    void pass();
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
    void gtpLButtonDown(int sgfX, int sgfY);

    // draw
    void drawBoard(QPainter& p);
    void drawCoordinates(QPainter& p);
    void drawStones(QPainter& p);
    void drawStones2(QPainter& p);
    void drawBranchMoves(QPainter& p, go::nodeList::iterator first, go::nodeList::iterator last);
    void drawCross(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawTriangle(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawCircle(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawSquare(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawCharacter(QPainter& p, go::markList::iterator first, go::markList::iterator last);
    void drawMark(QPainter& p, const QPainterPath& path, go::markList::iterator first, go::markList::iterator last);
    void drawPath(QPainter& p, const QPainterPath& path, int boardX, int boardY);
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
    void addTerritory(int x, int y);
    void setTerritory(int x, int y, int c);
    void unsetTerritory(int x, int y);
    void getCountTerritory(int& alive_b, int& alive_w, int& dead_b, int& dead_w, int& bt, int& wt);

    void setParent(go::nodePtr& parent, go::nodeList& childNodes);
    void createNodeList();
    void addMark(int sgfX, int sgfY, int boardX, int boardY);
    void addMark(go::markList& markList, const go::mark& mark);
    void addCharacter(go::markList& markList, const go::point& p);
    bool removeMark(go::markList& markList, const go::point& p);
    void addStone(go::nodePtr node, const go::point& sgfPoint, go::color color);
    void addStone(go::nodePtr node, const go::point& sgfPoint, const go::point& boardPoint, go::color color);
    void addEmpty(go::nodePtr node, const go::point& sgfPoint);
    void addEmpty(go::nodePtr node, const go::point& sgfPoint, const go::point& boardPoint);
    bool removeStone(go::stoneList& stoneList, const go::point& sp, const go::point& bp);
    void rotateSgf(go::nodePtr node, QUndoCommand* command);
    void rotateStoneSgf(go::nodePtr node, go::stoneList& stoneList, QUndoCommand* command);
    void rotateMarkSgf(go::nodePtr node, go::markList& markList, QUndoCommand* command);
    void flipSgf(go::nodePtr node, int xsize, int ysize, QUndoCommand* command);
    void flipStoneSgf(go::nodePtr node, go::stoneList& stoneList, int xsize, int ysize, QUndoCommand* command);
    void flipMarkSgf(go::nodePtr node, go::markList& markList, int xsize, int ysize, QUndoCommand* command);

    void boardToSgfCoordinate(int boardX, int boardY, int& sgfX, int& sgfY);
    void sgfToBoardCoordinate(int sgfX, int sgfY, int& boardX, int& boardY);

    void gtpWrite(const QString& buf);
    void gtpPut(int x, int y);
    bool gtpGetCoordinate(const QString& buf, int& x, int& y);
    void gtpHandicap();
    void gtpGameEnd();

private slots:
    void gtpReadReady();

private:
    Ui::BoardWidget *m_ui;

    // undo
    QUndoStack undoStack;

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
    BoardBuffer board;

    // sound
    Sound stoneSound;

    // play with computer
    QProcess* comProcess;
    QString   gtpBuf;
    bool isYourColorBlack;
    int gtpStatus;
    int gtpX;
    int gtpY;
};




#endif // BOARDWIDGET_H
