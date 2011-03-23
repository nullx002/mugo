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

class ViewData{
public:
    ViewData() : boardWidget(NULL), branchWidget(NULL), commentEdit(NULL){}

    BoardWidget* boardWidget;
    QTreeWidget* branchWidget;
    QPlainTextEdit* commentEdit;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef QMap<Document*, ViewData> DocViewData;

    // constructor, destructor
    explicit MainWindow(const QString& fname, QWidget *parent = 0);
    ~MainWindow();

public slots:
    // new, open, save, close
    bool fileNew(QTextCodec* codec=NULL, int xsize=19, int ysize=19, double komi=6.5, int handicap=0);
    bool fileOpen(const QString& fname, QTextCodec* codec=NULL, bool guessCodec=true, bool newTab=true);
    bool fileSave(SgfDocument* doc);
    bool fileSaveAs(SgfDocument* doc);
    bool fileSaveAs(SgfDocument* doc, const QFileInfo& fileInfo);

protected:
    void changeEvent(QEvent* e);
    void closeEvent(QCloseEvent* e);

    // initialize
    void initializeMenu();

    // create new tab
    bool createNewTab(Document* doc);

    // file dialog
    bool getOpenFileName(QString& fname, QTextCodec*& codec);
    bool getSaveFileName(const QString& initialPath, QString& fname, QTextCodec*& codec);

    // read
    SgfDocument* readSgfDocument(const QString& fname, QTextCodec* codec, bool guessCodec);

    // save
    bool closeTab(BoardWidget* board);
    bool maybeSave(SgfDocument* doc);

    // view
    void updateView(SgfDocument* doc);
    void createBranchItems(BoardWidget* board, QTreeWidget* branch, const Go::NodePtr& node);
    void createBranchItems(BoardWidget* board, QTreeWidgetItem* parent, const Go::NodePtr& node, bool shouldCreateChild);
    QTreeWidgetItem* createBranchItem(BoardWidget* board, const Go::NodePtr& node);

private slots:
    // file menu
    void on_actionSaveAs_triggered();
    void on_actionSave_triggered();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionExit_triggered();

    // document
    void on_sgfDocument_nodeModified(const Go::NodePtr& node);

    // board tab widget
    void on_boardTabWidget_tabCloseRequested(int index);
    void on_boardTabWidget_currentChanged(QWidget*);

    // board widget
    void on_board_gameChanged(const Go::NodePtr& game);
    void on_board_nodeChanged(const Go::NodePtr& node);

    // comment widget
    void on_commentEdit_textChanged();

private:
    Ui::MainWindow *ui;

    int docID;
    DocViewData docView;

    QUndoGroup undoGroup;
    QAction* undoAction;
    QAction* redoAction;
};

#endif // MAINWINDOW_H
