#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QTreeWidgetItem>
#include <QTextCodec>
#include "boardwidget.h"
#include "countterritorydialog.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	typedef QMap<go::node*, QTreeWidgetItem*> NodeToTreeWidgetType;


    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setCaption();

protected:
    virtual void closeEvent(QCloseEvent* e);
    virtual void keyPressEvent(QKeyEvent* event);

private:
    bool fileNew();
    bool fileOpen();
    bool fileOpen(const QString& fname);
    bool fileOpen(const QString& fname, const QString& filter);
    bool fileSave();
    bool fileSaveAs();
    bool fileSaveAs(const QString& fname);
    bool fileClose();
    bool maybeSave();

    void setEncoding(QAction* action, const char* codecName);
    void setShowMoveNumber(QAction* action, int moveNumber);
    void setEditMode(QAction* action, BoardWidget::eEditMode editMode);
    void setAnnotation(int annotation);
    void setAnnotation1(QAction* action, int annotation);
    void setAnnotation2(QAction* action, int annotation);
    void setAnnotation3(QAction* action, int annotation);

    void setBoardSize(int xsize, int ysize);
    void setCurrentFile(const QString& fname);
    void updateRecentFileActions();

    void setTreeData();
    QTreeWidgetItem* addTreeWidget(go::node* node, bool needRemake = false);
    QTreeWidgetItem* createTreeWidget(go::node* node);
    QTreeWidgetItem* remakeTreeWidget(QTreeWidgetItem* currentWidget);
    void deleteNode();
    void deleteTreeWidget(go::node* node);
    void deleteTreeWidgetForMap(go::node* node);
    void setTreeWidget(go::node* n);
    QString createTreeText(const go::node* node);

    go::node* getNode(QTreeWidgetItem* treeWidget);

    void setLanguage(const QString& locale, QAction* act);

    Ui::MainWindow *ui;
    QTextCodec* codec;
    QString fileName;
    QString filter;
    NodeToTreeWidgetType nodeToTreeWidget;
    int annotation1;
    int annotation2;
    int annotation3;
    bool branchMode;

    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];
    QAction *recentSeparator;
    QLabel* moveNumberLabel;
    QLabel* capturedLabel;
    CountTerritoryDialog* countTerritoryDialog;

private slots:
    // File menu
    void on_actionSetup_triggered();
    void on_actionLanguageJapanese_triggered();
    void on_actionLanguageEnglish_triggered();
    void on_actionLanguageSystemDefault_triggered();
    void on_actionCountTerritory_triggered();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionReload_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSaveBoardAsPicture_triggered();
    void on_actionExit_triggered();
    void openRecentFile();

    // Edit menu
    void on_actionGameInformation_triggered();
    void on_actionDelete_triggered();
    void on_actionPass_triggered();
    void on_actionEditNodeName_triggered();

    // Edit menu -> Stone & Marker
    void on_actionAlternateMove_triggered();
    void on_actionAddBlackStone_triggered();
    void on_actionAddWhiteStone_triggered();
    void on_actionAddEmpty_triggered();
    void on_actionAddLabel_triggered();
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
    void on_actionEncodingUTF8_triggered();
    void on_actionWindows_1252_triggered();
    void on_actionISO8859_1_triggered();
    void on_actionEncodingGB2312_triggered();
    void on_actionEncodingBig5_triggered();
    void on_actionEncodingKorean_triggered();
    void on_actionEncodingEucJP_triggered();
    void on_actionEncodingJIS_triggered();
    void on_actionEncodingShiftJIS_triggered();

    // Traverse menu
    void on_actionFirstMove_triggered();
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

    // View menu -> Toolbars
    void on_actionMainToolbar_triggered();
    void on_actionEditToolbar_triggered();
    void on_actionNavigationToolbar_triggered();
    void on_actionOptionToolbar_triggered();

    // Option menu
    void on_action19x19Board_triggered();
    void on_action13x13Board_triggered();
    void on_action9x9Board_triggered();
    void on_actionCustomBoardSize_triggered();
    void on_actionPlaySound_triggered();

    // Help menu
    void on_actionAbout_triggered();
    void on_actionAboutQT_triggered();

    // Board widget
    void on_boardWidget_nodeAdded(go::node* parent, go::node* node, bool select);
    void on_boardWidget_nodeDeleted(go::node* node);
    void on_boardWidget_nodeModified(go::node* node);
    void on_boardWidget_currentNodeChanged(go::node* node);
    void on_boardWidget_updateTerritory(int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory, double komi);

    // Branch widget
    void on_branchDockWidget_visibilityChanged(bool visible);
    void on_branchWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

    // Comment widget
    void on_commentDockWidget_visibilityChanged(bool visible);
    void on_commentWidget_textChanged();

    // Score Dialog
    void scoreDialogClosed();
};

#endif // MAINWINDOW_H
