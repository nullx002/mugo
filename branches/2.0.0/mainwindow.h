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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUndoGroup>
#include <QActionGroup>
#include <QUrl>
#include <QModelIndex>
#include "godata.h"


class QTreeWidget;
class QTreeWidgetItem;
class QStandardItemModel;
class QStandardItem;
class QHttpResponseHeader;
class QProgressDialog;
class QLabel;
class BoardWidget;
class Document;
class SgfDocument;


namespace Ui {
    class MainWindow;
}

/**
  MainWindow
*/
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    enum BranchType{ gameMode, branchMode };
    typedef QMap<Go::NodePtr, QTreeWidgetItem*> NodeTreeMap;

    class ViewData{
        public:
            BoardWidget* boardWidget;
            QTreeWidget* branchWidget;
            BranchType   branchType;
            QStandardItemModel* collectionModel;
            NodeTreeMap nodeToTreeItem;
            QAction* tabChangeAction;
    };
    typedef QMap<Document*, ViewData> DocumentManager;

    MainWindow(const QString& fname=QString(), QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);
    BoardWidget* currentBoard();
    Document* currentDocument();
    void setKeyboardShortcut();
    void createMenu();
    void createEncodingAction();
    void setPreferences(BoardWidget*);

    // new, open, save, close
    void fileNew(QTextCodec* codec, int xsize=19, int ysize=19, double komi=6.5, int handicap=0);
    bool fileOpen(const QString& fname, QTextCodec* codec, bool guessCodec);
    bool urlOpen(const QUrl& url, bool newTab);
    bool fileSave(Document*);
    bool fileSaveAs(Document*);
    bool fileSaveAs(Document* doc, const QString& fname);
    bool closeTab(int index);
    bool maybeSave(Document* doc);
    void addRecentFile(const QString& fname);
    void updateRecentFileActions();

    // document
    QString newDocumentName();
    SgfDocument* openDocument(const QString& fname, QTextCodec* codec, bool guessCodec);
    void addDocument(SgfDocument* doc, BoardWidget* board=NULL);
    bool closeDocument(Document* doc, bool save=true, bool closeTab=true);

    // branch widget
    void createBranchWidget(Document* doc);
    void createBranchWidget(BoardWidget* board, Go::NodePtr node);
    void createBranchWidget(ViewData& data, BoardWidget* board, QTreeWidgetItem* root, QTreeWidgetItem* parent1, QTreeWidgetItem* parent2, Go::NodePtr parentNode, Go::NodePtr node);
    QTreeWidgetItem* createBranchItem(ViewData& data, BoardWidget* board, Go::NodePtr node);
    QString getBranchItemText(BoardWidget* board, Go::NodePtr node);
    void removeBranchItem(QTreeWidgetItem* parent, NodeTreeMap& map, Go::NodePtr node);

    // collection view
    void createCollectionModel(const Go::NodeList& gameList, QStandardItemModel* model);
    void createCollectionModelRow(const Go::NodePtr& game, QList<QStandardItem*>& items);

    void updateStatusBar(BoardWidget* board=NULL);
    void updateCaption(bool updateTab);
    void updateMenu();

    bool getOpenFileName(QString& fname, QTextCodec*& codec);
    bool getSaveFileName(const QString& initialPath, QString& fname, QTextCodec*& codec);

private:
    Ui::MainWindow *ui;
    QUndoGroup undoGroup;
    QActionGroup* editGroup;
    QActionGroup* encodingGroup;
    QActionGroup* tabChangeGroup;
    QMap<QAction*, QTextCodec*> encoding;
    DocumentManager docManager;
    uint docID;
    QProgressDialog* progressDialog;
    int downloadID;
    QUrl downloadURL;
    bool downloadNewTab;
    QLabel* encodingLabel;
    QLabel* moveNumberLabel;
    QLabel* capturedLabel;
    int sgfLineWidth;
    int maxRecentFiles;

private slots:
    // File Menu
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionReload_triggered();
    void on_actionOpenURL_triggered();
    void on_actionCloseTab_triggered();
    void on_actionCloseAllTabs_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionCollectionImport_triggered();
    void on_actionCollectionExtract_triggered();
    void on_actionExportBoardAsImage_triggered();
    void on_actionExportAsciiToClipboard_triggered();
    void on_actionOpenRecentFile_triggered();
    void on_actionExit_triggered();

    // Edit Menu
    void on_actionCopySgfToClipboard_triggered();
    void on_actionCopyCurrentBranchToClipboard_triggered();
    void on_actionPasteSgfToNewTab_triggered();
    void on_actionPasteSgfIntoCollection_triggered();
    void on_actionGameInformation_triggered();
    void on_actionDeleteAfterCurrent_triggered();
    void on_actionDeleteCurrentOnly_triggered();
    void on_actionPass_triggered();
    void on_actionAlternateMove_triggered();
    void on_actionStonesAndMarkers_triggered();
    void on_actionAddBlackStone_triggered();
    void on_actionAddWhiteStone_triggered();
    void on_actionAddEmpty_triggered();
    void on_actionAddLabel_triggered();
    void on_actionAddLabelManually_triggered();
    void on_actionAddCircle_triggered();
    void on_actionAddCross_triggered();
    void on_actionAddTriangle_triggered();
    void on_actionAddSquare_triggered();
    void on_actionDeleteMarker_triggered();
    void on_actionEditNodeName_triggered();
    void on_actionSetMoveNumber_triggered();
    void on_actionUnsetMoveNumber_triggered();
    void on_actionWhiteFirst_triggered(bool checked);
    void on_actionRotateSGFClockwise_triggered();
    void on_actionFlipSgfHorizontally_triggered();
    void on_actionFlipSgfVertically_triggered();

    // Edit -> Node Annotation
    void on_actionNoNodeAnnotation_triggered();
    void on_actionEven_triggered();
    void on_actionGoodForBlack_triggered();
    void on_actionVeryGoodForBlack_triggered();
    void on_actionGoodForWhite_triggered();
    void on_actionVeryGoodForWhite_triggered();
    void on_actionUnclear_triggered();

    // Edit -> Move Annotation
    void on_actionNoMoveAnnotation_triggered();
    void on_actionGoodMove_triggered();
    void on_actionVeryGoodMove_triggered();
    void on_actionBadMove_triggered();
    void on_actionVeryBadMove_triggered();
    void on_actionDoubtfulMove_triggered();
    void on_actionInterestingMove_triggered();

    // Edit -> Annotation
    void on_actionHotspot_triggered(bool checked);

    // Navigation Menu
    void on_actionMoveFirst_triggered();
    void on_actionFastRewind_triggered();
    void on_actionMovePrevious_triggered();
    void on_actionMoveLast_triggered();
    void on_actionFastForward_triggered();
    void on_actionMoveNext_triggered();
    void on_actionBackToParent_triggered();
    void on_actionPreviousSibling_triggered();
    void on_actionNextSibling_triggered();
    void on_actionJumpToMoveNumber_triggered();
    void on_actionJumpToClicked_triggered(bool checked);

    // View Menu
    void on_actionMoveNumber_triggered();
    void on_actionShowMoveNumber_triggered(bool checked);
    void on_actionResetMoveNumberInBranch_triggered(bool checked);
    void on_actionNoMoveNumber_triggered();
    void on_actionLast1Move_triggered();
    void on_actionLast2Moves_triggered();
    void on_actionLast5Moves_triggered();
    void on_actionLast10Moves_triggered();
    void on_actionLast20Moves_triggered();
    void on_actionLast50Moves_triggered();
    void on_actionAllMoves_triggered();
    void on_actionBranchMode_triggered(bool checked);
    void on_actionShowCoordinate_triggered(bool checked);
    void on_actionShowCoordinateWithI_triggered(bool checked);
    void on_actionShowMarker_triggered(bool checked);
    void on_actionNoMarkup_triggered();
    void on_actionShowChildren_triggered();
    void on_actionShowSiblings_triggered();
    void on_actionRotateClockwise_triggered();
    void on_actionFlipHorizontally_triggered(bool checked);
    void on_actionFlipVertically_triggered(bool checked);
    void on_actionResetBoard_triggered();

    // Tools Menu
    void on_actionOptions_triggered();

    // Window Menu
    void on_actionPreviousTab_triggered();
    void on_actionNextTab_triggered();
    void on_tabChangeRequest();

    // Help Menu
    void on_actionAbuot_triggered();
    void on_actionAboutQt_triggered();

    // Document
    void on_sgfdocument_modified(bool dirty);
    void on_sgfdocument_nodeAdded(Go::NodePtr node);
    void on_sgfdocument_nodeDeleted(Go::NodePtr node, bool removeChildren);
    void on_sgfdocument_nodeModified(Go::NodePtr node, bool needRecreateBoard);
    void on_sgfdocument_gameAdded(Go::NodePtr game, int index);
    void on_sgfdocument_gameDeleted(Go::NodePtr game, int index);
    void on_sgfdocument_gameMoved(Go::NodePtr game, int from, int to);

    // BoardWidget
    void on_boardWidget_currentNodeChanged(Go::NodePtr node);
    void on_boardWidget_currentGameChanged(Go::NodePtr game);

    // BoardTabWidget
    void on_boardTabWidget_currentChanged(QWidget* );
    void on_boardTabWidget_tabCloseRequested(int index);

    // BranchWidget
    void on_branchWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

    // CommentWidget
    void on_commentWidget_textChanged();

    // Collection View
    void on_collectionView_activated(const QModelIndex& index);
    void on_actionCollectionMoveDown_triggered();
    void on_actionCollectionMoveUp_triggered();
    void on_actionCollectionDelete_triggered();

    // Open URL
    void on_openUrl_dataReadProgress(int, int);
    void on_openUrl_requestFinished(int id, bool error);
};


/**
  create new document name
*/
inline
QString MainWindow::newDocumentName(){
    return tr("Untitled-%1").arg(++docID);
}

#endif // MAINWINDOW_H
