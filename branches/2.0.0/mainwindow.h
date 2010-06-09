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
#include "godata.h"


class QTreeWidget;
class QTreeWidgetItem;
class QStandardItemModel;
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

    class TabData{
        public:
            BoardWidget* boardWidget;
            QTreeWidget* branchWidget;
            BranchType   branchType;
            QStandardItemModel* collectionModel;
            QMap<Go::NodePtr, QTreeWidgetItem*> nodeToTreeItem;
    };

    MainWindow(const QString& fname=QString(), QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    BoardWidget* currentBoard();
    void setKeyboardShortcut();
    void fileNew(int xsize=19, int ysize=19, double komi=6.5, int handicap=0);
    bool fileOpen(const QString& fname);
    bool fileSave(Document*);
    bool fileSaveAs(Document*);
    bool fileSaveAs(Document* doc, const QString& fname);
    bool closeTab(int index);
    void addDocument(BoardWidget* board);
    void createBranchWidget(BoardWidget* board, Go::NodePtr node);
    void createBranchWidget(BoardWidget* board, QTreeWidgetItem* root, QTreeWidgetItem* parent1, QTreeWidgetItem* parent2, Go::NodePtr parentNode, Go::NodePtr node);
//    void createBranchWidget(BoardWidget* board, QTreeWidgetItem* root, QTreeWidgetItem* parent, Go::NodePtr node, bool branch);
    QTreeWidgetItem* createBranchItem(BoardWidget* board, Go::NodePtr node);

private:
    Ui::MainWindow *ui;
    QUndoGroup undoGroup;
    QTextCodec* defaultCodec;
    QMap<SgfDocument*, TabData> tabDatas;
    uint docID;

private slots:
    // File Menu
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();

    // Help Menu
    void on_actionAbuot_triggered();
    void on_actionAboutQt_triggered();

    // Document
    void on_document_nodeAdded(Go::NodePtr node);

    // BoardWidget
    void on_boardWidget_currentNodeChanged(Go::NodePtr node);

    // BoardTabWidget
    void on_boardTabWidget_currentChanged(QWidget* );
    void on_boardTabWidget_tabCloseRequested(int index);

    // BranchWidget
    void on_branchWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
};

#endif // MAINWINDOW_H
