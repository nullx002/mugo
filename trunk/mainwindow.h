#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QTreeWidgetItem>
#include <QTextCodec>
#include <QUndoGroup>
#include <QProcess>
#include <QHttp>
#include <QProgressDialog>
#include <QActionGroup>
#include "boardwidget.h"
#include "countterritorydialog.h"
#include "gtp.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef QMap<go::nodePtr, QTreeWidgetItem*> NodeToTreeWidgetType;

    struct TabData{
        TabData() : branchMode(false), countTerritoryDialog(NULL), playGame(NULL), gtpProcess(NULL){}

        QAction* menuAction;

        NodeToTreeWidgetType nodeToTree;
        QTreeWidget* branchWidget;

        QString fileName;
        QString documentName;

        QTextCodec* codec;
        QAction* encode;

        bool branchMode;
        QVector<bool> countTerritoryMenuStatus;
        QVector<bool> playWithComputerMenuStatus;

        CountTerritoryDialog* countTerritoryDialog;

        PlayGame* playGame;
        QProcess* gtpProcess;
    };

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool fileNew(int xsize=19, int ysize=19, int handicap=0, double komi=6.5);
    bool fileOpen();
    bool fileOpen(const QString& fname, bool guessCodec=true, bool newTab=true, bool forceOpen=false);
    bool fileSave();
    bool fileSaveAs();
    bool fileSaveAs(const QString& fname);
    bool fileClose();
    bool tabClose(int index);
    bool allTabClose();
    bool maybeSave();

protected:
    virtual void closeEvent(QCloseEvent* e);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dropEvent(QDropEvent* event);

private:
    void addDocument(BoardWidget* board);
    void setDocument(BoardWidget* board);

    void setCurrentFile(const QString& fname);
    void updateRecentFileActions();

    void setCaption();
    void updateMenu();
    void updateGameList();

    void setEditMode(QAction* action, BoardWidget::eEditMode editMode);
    void setAnnotation(int annotation, int moveAnnotation, int nodeAnnotation);
    void setAnnotation(QAction* action, int annotation);
    void setMoveAnnotation(QAction* action, int annotation);
    void setNodeAnnotation(QAction* action, int annotation);

    void setTreeData();
    QTreeWidgetItem* addTreeWidget(go::nodePtr node, bool needRemake = false);
    QTreeWidgetItem* createTreeWidget(go::nodePtr node);
    QTreeWidgetItem* remakeTreeWidget(QTreeWidgetItem* currentWidget);
    void deleteNode(bool deleteNode);
    void deleteTreeWidget(go::nodePtr node, bool deleteChildren);
    void deleteTreeWidgetForMap(go::nodePtr node);
    void setTreeWidget(go::nodePtr n);
    QString createTreeText(const go::nodePtr node);

    go::nodePtr getNode(QTreeWidgetItem* treeWidget);

    void setCountTerritoryMode(bool on=true);
    void setPlayWithComputerMode(bool on=true);
    void endGame();

    void alertLanguageChanged();

    Ui::MainWindow *ui;
    QMap<BoardWidget*, TabData> tabDatas;
    TabData* tabData;
    QActionGroup tabMenuGroups;
    BoardWidget* boardWidget;
    QTreeWidget* branchWidget;
    NodeToTreeWidgetType* nodeToTreeWidget;
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

private slots:
    // File menu
    void on_gameListWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void on_actionCloseAllTabs_triggered();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionOpenURL_triggered();
    void on_actionReload_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSaveBoardAsPicture_triggered();
    void on_actionExportAsciiToClipboard_triggered();
    void on_actionPrint_triggered();
    void on_actionCloseTab_triggered();
    void on_actionExit_triggered();
    void openRecentFile();

    // Edit menu
    void on_actionCopySGFtoClipboard_triggered();
    void on_actionCopyCurrentSGFtoClipboard_triggered();
    void on_actionPasteSGFfromClipboard_triggered();
    void on_actionPasteSGFasBranchfromClipboard_triggered();
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
    void setEncoding(QAction* action);

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
    void on_actionPlayWithGnugo_triggered();
    void on_actionTutorBossSides_triggered();
    void on_actionTutorOneSide_triggered();
    void on_actionPlaySound_triggered();
    void on_action19x19Board_triggered();
    void on_action13x13Board_triggered();
    void on_action9x9Board_triggered();
    void on_actionCustomBoardSize_triggered();
    void on_actionOptions_triggered();
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

    // Board widget
    void nodeAdded(go::nodePtr parent, go::nodePtr node, bool select);
    void nodeDeleted(go::nodePtr node, bool deleteChildren);
    void nodeModified(go::nodePtr node);
    void currentNodeChanged(go::nodePtr node);
    void updateTerritory(int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory, double komi);

    // Branch widget
    void branchWidgetCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

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

    // tab change
    void onTabChangeRequest();
};

#endif // MAINWINDOW_H
