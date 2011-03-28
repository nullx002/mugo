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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUndoGroup>
#include "sgfdocument.h"
#include "boardwidget.h"
#
class QFileInfo;
class QTreeWidget;
class QTreeWidgetItem;
class QPlainTextEdit;

namespace Ui {
    class MainWindow;
}


inline
bool operator <(const Go::NodePtr& node1, const Go::NodePtr& node2){
    return node1.data() < node2.data();
}


class ViewData{
public:
    ViewData() : boardWidget(NULL), branchWidget(NULL), commentEdit(NULL){}

    BoardWidget* boardWidget;
    QTreeWidget* branchWidget;
    QPlainTextEdit* commentEdit;
    QMap<Go::NodePtr, QTreeWidgetItem*> nodeToTreeItem;
};


/**
  Main Window
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef QMap<Document*, ViewData> DocViewData;

    // constructor, destructor
    explicit MainWindow(const QString& fname, QWidget *parent = 0);
    ~MainWindow();

public slots:
    /// @name new, open, save, close
    bool fileNew(QTextCodec* codec=NULL, int xsize=19, int ysize=19, double komi=6.5, int handicap=0);
    bool fileOpen(const QString& fname, QTextCodec* codec=NULL, bool guessCodec=true);
    bool fileSave(GoDocument* doc);
    bool fileSaveAs(GoDocument* doc);
    bool fileSaveAs(GoDocument* doc, const QFileInfo& fileInfo);
    bool closeDocument(GoDocument* doc);
    bool closeAllDocuments();

protected:
    /// @name event
    void changeEvent(QEvent* e);
    void closeEvent(QCloseEvent* e);

    /// @name initialize
    void initializeMenu();
    void createEncodingMenu();

    /// @name create new tab
    bool createNewTab(Document* doc);

    /// @name file dialog
    bool getOpenFileName(QString& fname, QTextCodec*& codec);
    bool getSaveFileName(const QString& initialPath, QString& fname, QTextCodec*& codec);

    /// @name load/save
    bool reload(QTextCodec* codec);
    bool maybeSave(GoDocument* doc);

    /// @name view
    void updateView(GoDocument* doc);

    /// @name branch view
    void createBranchItems(Document* doc, const Go::NodePtr& game);
    void createBranchItems(Document* doc, QTreeWidgetItem* parent, const Go::NodePtr& node);
    void addBranchItem(Document* doc, QTreeWidget* branch, const Go::NodePtr& node);
    bool shouldNest(const Go::NodePtr& node);
    void rebuildBranchItems(ViewData& view, const Go::NodePtr& node);
    QTreeWidgetItem* createBranchItem(Document* doc, const Go::NodePtr& node);
    QTreeWidgetItem* getParentItem(QTreeWidgetItem* item);

private slots:
    //@{
    /// @name slot for file menu
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionOpenURL_triggered();
    void on_actionCloseTab_triggered();
    void on_actionCloseAllTabs_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExportBoardAsImage_triggered();
    void on_actionExportAsciiToClipboard_triggered();
    void on_actionPrint_triggered();
    void on_actionExit_triggered();

    /// @name slot for file reload menu
    void on_actionSystem_triggered();
    void on_actionUTF8_triggered();
    void on_actionISO_8859_1_triggered();
    void on_actionISO_8859_15_triggered();
    void on_actionWindows_1252_triggered();
    void on_actionISO_8859_14_triggered();
    void on_actionISO_8859_7_triggered();
    void on_actionWindows_1253_triggered();
    void on_actionISO_8859_10_triggered();
    void on_actionISO_8859_3_triggered();
    void on_actionISO_8859_4_triggered();
    void on_actionISO_8859_13_triggered();
    void on_actionWindows_1257_triggered();
    void on_actionISO_8859_2_triggered();
    void on_actionWindows_1250_triggered();
    void on_actionISO_8859_5_triggered();
    void on_actionWindows_1251_triggered();
    void on_actionKOI8_R_triggered();
    void on_actionKOI8_U_triggered();
    void on_actionISO_8859_16_triggered();
    void on_actionISO_8859_11_triggered();
    void on_actionISO_8859_9_triggered();
    void on_actionWindows_1254_triggered();
    void on_actionWindows_1258_triggered();
    void on_actionISO_8859_6_triggered();
    void on_actionWindows_1256_triggered();
    void on_actionWindows_1255_triggered();
    void on_actionISO8859_8_triggered();
    void on_actionGB2312_triggered();
    void on_actionBig5_triggered();
    void on_actionEucKR_triggered();
    void on_actionEucJP_triggered();
    void on_actionISO_2022_JP_triggered();
    void on_actionShiftJIS_triggered();

    /// @name slot for document
    void on_sgfDocument_nodeAdded(const Go::NodePtr& node);
    void on_sgfDocument_nodeDeleted(const Go::NodePtr& node);
    void on_sgfDocument_nodeModified(const Go::NodePtr& node);

    /// @name slot for board tab widget
    void on_boardTabWidget_tabCloseRequested(int index);
    void on_boardTabWidget_currentChanged(QWidget*);

    /// @name slot for board widget
    void on_board_gameChanged(const Go::NodePtr& game);
    void on_board_nodeChanged(const Go::NodePtr& node);

    /// @name slot for comment widget
    void on_commentEdit_textChanged();

    /// @name slot for branch widget
    void on_branchWidget_currentItemChanged(QTreeWidgetItem* current,QTreeWidgetItem* previous);
    //@}

private:
    Ui::MainWindow *ui;

    int docID;
    DocViewData docView;

    QUndoGroup undoGroup;
    QAction* undoAction;
    QAction* redoAction;
};

#endif // MAINWINDOW_H
