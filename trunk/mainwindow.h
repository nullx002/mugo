#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QUndoGroup>
#include <QUrl>
#include <QActionGroup>
#include "boardwidget.h"
#include "countterritorydialog.h"

class QTextCodec;
class QTreeWidget;
class QTreeWidgetItem;
class QProgressDialog;
class QHttp;
class QHttpResponseHeader;
class QProcess;


namespace Ui
{
    class MainWindow;
}

/**
* Main Window
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef QMap<go::nodePtr, QTreeWidgetItem*> NodeToTreeWidgetType;

    struct TabData{
        TabData() : branchMode(false), countTerritoryDialog(NULL), playGame(NULL){}

        QAction* menuAction;

        NodeToTreeWidgetType nodeToTree;
        QTreeWidget* branchWidget;

        QString fileName;
        QString documentName;
        QUrl url;

        QTextCodec* codec;
        QAction* encode;

        bool branchMode;
        QVector<bool> countTerritoryMenuStatus;
        QVector<bool> playWithComputerMenuStatus;

        CountTerritoryDialog* countTerritoryDialog;

        PlayGame* playGame;
    };

    typedef QMap<BoardWidget*, TabData> TabDataMap;

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool fileNew(int xsize=19, int ysize=19, int handicap=0, double komi=6.5);
    bool fileOpen();
    bool fileOpen(const QString& fname, bool guessCodec=true, bool newTab=true, bool forceOpen=false);
    bool urlOpen(const QUrl& url);
    go::fileBase* readFile(const QString& fname, QTextCodec*& codec, bool guessCodec);
    bool fileSave(BoardWidget* board);
    bool fileSaveAs(BoardWidget* board);
    bool fileSaveAs(BoardWidget* board, const QString& fname);
    bool closeTab(int index);
    bool closeAllTab();
    bool maybeSave(BoardWidget* board);
    bool stopGame(BoardWidget* boardWidget);

protected:
    virtual void closeEvent(QCloseEvent* e);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dropEvent(QDropEvent* event);

private:
    BoardWidget* currentBoard();
    const BoardWidget* currentBoard() const;
    void addDocument(BoardWidget* board);
    void setDocument(BoardWidget* board);

    void setFileName(BoardWidget* boardWidget, const QString& fname);
    void updateRecentFileActions();

    void setCaption();
    void updateMenu();
    void updateCollection();

    void setEditMode(QAction* action, BoardWidget::eEditMode editMode);
    void setAnnotation(int annotation, int moveAnnotation, int nodeAnnotation);
    void setAnnotation(QAction* action, int annotation);
    void setMoveAnnotation(QAction* action, int annotation);
    void setNodeAnnotation(QAction* action, int annotation);

    void setTreeData(BoardWidget* board);
    QTreeWidgetItem* addTreeWidget(BoardWidget* board, go::nodePtr node, bool needRemake = false);
    QTreeWidgetItem* createTreeWidget(BoardWidget* board, go::nodePtr node);
    QTreeWidgetItem* remakeTreeWidget(BoardWidget* board, QTreeWidgetItem* currentWidget);
    void deleteNode(bool deleteNode);
    void deleteTreeWidget(BoardWidget* board, go::nodePtr node, bool deleteChildren);
    void deleteTreeWidgetForMap(BoardWidget* board, go::nodePtr node);
    void setTreeWidget(BoardWidget* board, go::nodePtr n);
    QString createTreeText(BoardWidget* board, const go::nodePtr node);

    go::nodePtr getNode(QTreeWidgetItem* treeWidget);

    void setCountTerritoryMode(BoardWidget* board, bool on);
    void setPlayWithComputerMode(BoardWidget* board, bool on);
    void endGame(BoardWidget* board);

    void alertLanguageChanged();
    QString getDefaultSaveName() const;

    void readSettings();

    Ui::MainWindow *ui;
    TabDataMap tabDatas;
    QActionGroup tabMenuGroups;
    int docIndex;

    enum { MaxRecentFiles = 5 };
    QAction* recentFileActs[MaxRecentFiles];
    QLabel* moveNumberLabel;
    QLabel* capturedLabel;

    QUndoGroup undoGroup;
    QAction*   undoAction;
    QAction*   redoAction;

    bool countTerritoryMode;
    bool playWithComputerMode;

    QProgressDialog* progressDialog;
    QHttp* http;
    QByteArray downloadBuff;

    QList<QAction*> codecActions;
    QList<const char*> codecNames;
    QTextCodec* defaultCodec;

    int stepsOfFastMove;

    QString OPEN_FILTER;

private slots:
    // File menu
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionOpenURL_triggered();
    void on_actionReload_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSaveBoardAsPicture_triggered();
    void on_actionExportAsciiToClipboard_triggered();
    void on_actionCollectionExtract_triggered();
    void on_actionCollectionImport_triggered();
    void on_actionCloseTab_triggered();
    void on_actionCloseAllTabs_triggered();
    void on_actionPrint_triggered();
    void on_actionExit_triggered();
    void openRecentFile();

    // Edit menu
    void on_actionCopySgfToClipboard_triggered();
    void on_actionCopyCurrentSgfToClipboard_triggered();
    void on_actionPasteSgfToNewTab_triggered();
    void on_actionPasteSgfToCollection_triggered();
//    void on_actionPasteSgfAsBranchFromClipboard_triggered();
    void on_actionGameInformation_triggered();
    void on_actionDeleteAfterCurrent_triggered();
    void on_actionDeleteOnlyCurrent_triggered();
    void on_actionPass_triggered();
    void on_actionEditNodeName_triggered();
    void on_actionWhiteFirst_triggered();

    // Edit menu -> Stone & Marker
    void on_actionAlternateMove_triggered();
    void on_actionAddBlackStone_triggered();
    void on_actionAddWhiteStone_triggered();
    void on_actionAddEmpty_triggered();
    void on_actionAddLabel_triggered();
    void on_actionAddLabelManually_triggered();
    void on_actionAddCross_triggered();
    void on_actionAddCircle_triggered();
    void on_actionAddSquare_triggered();
    void on_actionAddTriangle_triggered();
    void on_actionDeleteMarker_triggered();

    // Edit menu -> Annotation
    void on_actionGoodMove_triggered();
    void on_actionVeryGoodMove_triggered();
    void on_actionBadMove_triggered();
    void on_actionVeryBadMove_triggered();
    void on_actionDoubtfulMove_triggered();
    void on_actionInterestingMove_triggered();
    void on_actionEven_triggered();
    void on_actionGoodForBlack_triggered();
    void on_actionVeryGoodForBlack_triggered();
    void on_actionGoodForWhite_triggered();
    void on_actionVeryGoodForWhite_triggered();
    void on_actionUnclear_triggered();
    void on_actionHotspot_triggered();

    // Edit menu -> Move Number
    void on_actionSetMoveNumber_triggered();
    void on_actionUnsetMoveNumber_triggered();

    // Edit menu (rotate/flip)
    void on_actionRotateSgfClockwise_triggered();
    void on_actionFlipSgfHorizontally_triggered();
    void on_actionFlipSgfVertically_triggered();

    // Edit menu -> Encoding
    void setEncoding();
    void setEncoding(QAction* action, bool saveToDefault=false);
    void setEncoding(QTextCodec* codec);

    // Traverse menu
    void on_actionMoveFirst_triggered();
    void on_actionFastRewind_triggered();
    void on_actionPreviousMove_triggered();
    void on_actionNextMove_triggered();
    void on_actionFastForward_triggered();
    void on_actionMoveLast_triggered();
    void on_actionBackToParent_triggered();
    void on_actionPreviousBranch_triggered();
    void on_actionNextBranch_triggered();
    void on_actionJumpToClicked_triggered();
    void on_actionJumpToMoveNumber_triggered();

    // View menu -> Move Number
    void on_actionNoMoveNumber_triggered();
    void on_actionResetMoveNubmerInBranch_triggered();
    void on_actionLast1Move_triggered();
    void on_actionLast2Moves_triggered();
    void on_actionLast5Moves_triggered();
    void on_actionLast10Moves_triggered();
    void on_actionLast20Moves_triggered();
    void on_actionLast50Moves_triggered();
    void on_actionAllMoves_triggered();

    // View menu
    void on_actionShowMoveNumber_triggered();
    void on_actionShowMoveNumber_parent_triggered();
    void on_actionShowCoordinate_triggered();
    void on_actionShowCoordinateI_triggered();
    void on_actionShowMarker_triggered();
    void on_actionShowBranchMoves_triggered();
    void on_actionBranchMode_triggered();
    void on_actionRotateBoardClockwise_triggered();
    void on_actionFlipBoardHorizontally_triggered();
    void on_actionFlipBoardVertically_triggered();
    void on_actionResetBoard_triggered();

    // Tools menu
    void on_actionCountTerritory_triggered();
    void on_actionPlayWithComputer_triggered();
    void on_actionAutomaticReplay_triggered();
    void on_actionTutorBothSides_triggered();
    void on_actionTutorOneSide_triggered();
    void on_actionPlaySound_triggered();
    void on_actionOptions_triggered();
    void on_actionClearSettings_triggered();
    // Tools -> Language menu
    void on_actionLanguageSystemDefault_triggered();
    void on_actionLanguageEnglish_triggered();
    void on_actionLanguageJapanese_triggered();

    // window
    void on_actionNextTab_triggered();
    void on_actionPreviousTab_triggered();

    // Help menu
    void on_actionAbout_triggered();
    void on_actionAboutQT_triggered();

    // Board tab widget
    void on_boardTabWidget_currentChanged(QWidget* );
    void on_boardTabWidget_tabCloseRequested(int index);

    // Collection Widget
//    void on_collectionWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
//    void on_collectionWidget_itemDoubleClicked(QTreeWidgetItem* item, int column);
    void on_collectionWidget_itemActivated(QTreeWidgetItem* item, int column);
    void on_actionCollectionMoveUp_triggered();
    void on_actionCollectionMoveDown_triggered();
    void on_actionDeleteSgfFromCollection_triggered();

    // Board widget
    void boardCleared();
    void nodeAdded(go::nodePtr parent, go::nodePtr node, bool select);
    void nodeDeleted(go::nodePtr node, bool deleteChildren);
    void nodeModified(go::nodePtr node);
    void currentNodeChanged(go::nodePtr node);
    void updateTerritory(int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory, double komi);

    // Branch widget
    void branchWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void branchWidget_customContextMenuRequested(const QPoint& pos);
    void on_actionBranchMoveUp_triggered();
    void on_actionBranchMoveDown_triggered();

    // Comment widget
    void on_commentWidget_textChanged();

    // Score Dialog
    void scoreDialogClosed(int);

    // Open URL
    void openUrlReadReady(const QHttpResponseHeader& resp);
    void openUrlReadProgress(int done, int total);
    void openUrlDone(bool error);
    void openUrlCancel();

    // play a game
    void playGameEnded();
    void on_actionGamePass_triggered();
    void on_actionGameResign_triggered();
    void on_actionGameUndo_triggered();

    // tab change
    void onTabChangeRequest();

    // auto replay
    void automaticReplay_ended();
};

#endif // MAINWINDOW_H
