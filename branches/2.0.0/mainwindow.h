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
    void setStatusBarWidget();

    // new, open, save, close
    void fileNew(QTextCodec* codec, int xsize=19, int ysize=19, double komi=6.5, int handicap=0);
    bool fileOpen(QTextCodec* codec, const QString& fname, bool guessCodec);
    bool urlOpen(const QUrl& url, bool newTab);
    bool fileSave(Document*);
    bool fileSaveAs(Document*);
    bool fileSaveAs(Document* doc, const QString& fname);
    bool closeTab(int index);
    bool maybeSave(Document* doc);

    // document
    QString newDocumentName();
    SgfDocument* openDocument(const QString& fname, QTextCodec* codec, bool guessCodec);
    void addDocument(SgfDocument* doc, BoardWidget* board=NULL);
    bool closeDocument(Document* doc, bool save=true, bool closeTab=true);

    // branch widget
    void createBranchWidget(Document* doc);
    void createBranchWidget(BoardWidget* board, Go::NodePtr node);
    void createBranchWidget(BoardWidget* board, QTreeWidgetItem* root, QTreeWidgetItem* parent1, QTreeWidgetItem* parent2, Go::NodePtr parentNode, Go::NodePtr node);
    QTreeWidgetItem* createBranchItem(BoardWidget* board, Go::NodePtr node);
    QString getBranchItemText(BoardWidget* board, Go::NodePtr node);
    void removeBranchItem(QTreeWidgetItem* parent, NodeTreeMap& map, Go::NodePtr node);

    // collection view
    void createCollectionModel(const Go::NodeList& gameList, QStandardItemModel* model);
    void createCollectionModelRow(const Go::NodePtr& game, QList<QStandardItem*>& items);

    void updateCaption(bool updateTab);

    bool getOpenFileName(QString& fname, QTextCodec*& codec);
    bool getSaveFileName(QString& fname, QTextCodec*& codec);

private:
    Ui::MainWindow *ui;
    QUndoGroup undoGroup;
    QActionGroup* tabChangeGroup;
    QMap<QAction*, QTextCodec*> encoding;
    QTextCodec* defaultCodec;
    DocumentManager docManager;
    uint docID;
    QProgressDialog* progressDialog;
    QUrl downloadURL;
    QByteArray downloadBuff;
    bool downloadNewTab;
    QLabel* moveNumberLabel;
    QLabel* capturedLabel;
    int sgfLineWidth;

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
    void on_actionExit_triggered();

    // Edit Menu
    void on_actionCopySgfToClipboard_triggered();
    void on_actionCopyCurrentBranchToClipboard_triggered();
    void on_actionPasteSgfToNewTab_triggered();
    void on_actionPasteSgfIntoCollection_triggered();
    void on_actionDeleteAfterCurrent_triggered();
    void on_actionDeleteCurrentOnly_triggered();
    void on_actionPass_triggered();
    void on_actionGameInformation_triggered();

    // Navigation Menu
    void on_actionNavigationMoveFirst_triggered();
    void on_actionNavigationFastRewind_triggered();
    void on_actionNavigationMovePrevious_triggered();
    void on_actionNavigationMoveLast_triggered();
    void on_actionNavigationFastForward_triggered();
    void on_actionNavigationMoveNext_triggered();

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
    void on_sgfdocument_nodeModified(Go::NodePtr node);
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
    void on_collectionView_doubleClicked(QModelIndex index);
    void on_actionCollectionMoveDown_triggered();
    void on_actionCollectionMoveUp_triggered();
    void on_actionCollectionDelete_triggered();

    // Open URL
    void on_openUrl_ReadReady(const QHttpResponseHeader&);
    void on_openUrl_UrlReadProgress(int, int);
    void on_openUrl_UrlDone(bool);
};


/**
  create new document name
*/
inline
QString MainWindow::newDocumentName(){
    return tr("Untitled-%1").arg(++docID);
}

#endif // MAINWINDOW_H
