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
#include <QCloseEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sgfdocument.h"

/**
  Constructor
*/
MainWindow::MainWindow(const QString& fname, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    docID(0)
{
    // initialize view
    ui->setupUi(this);
    initializeMenu();

    // open or create new tab
    if (fname.isEmpty())
        fileNew();
    else
        fileOpen(fname);
    ui->boardTabWidget->removeTab(0);
}

/**
  Destructor
*/
MainWindow::~MainWindow()
{
    delete ui;
}

/**
  change event
*/
void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

/**
  close event
*/
void MainWindow::closeEvent(QCloseEvent* e){
    for (int i=0; i<ui->boardTabWidget->count(); ++i){
//        BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(i));
//        if (maybeSave(board) == false){
//            e->ignore();
//            return;
//        }
    }
    e->accept();
}

/**
  create new document in new tab
*/
bool MainWindow::fileNew(QTextCodec* codec, int xsize, int ysize, double komi, int handicap){
    SgfDocument* doc = new SgfDocument(xsize, ysize, komi, handicap, this);
    doc->setName( tr("Untitled-%1").arg(++docID) );
    createNewTab(doc);

    return true;
}

/**
  open exist file in new tab.
  if file is already opened, document will be active.
*/
bool MainWindow::fileOpen(const QString& fname, QTextCodec* codec, bool newTab){
    return true;
}

/**
  initialize menu
  create action and actino group
*/
void MainWindow::initializeMenu(){
    // Edit -> undo/redo
    undoAction = undoGroup.createUndoAction(this);
    undoAction->setShortcut(QKeySequence::Undo);
//    undoAction->setIcon( QIcon(":/res/undo.png") );
    redoAction = undoGroup.createRedoAction(this);
    redoAction->setShortcut(QKeySequence::Redo);
//    redoAction->setIcon( QIcon(":/res/redo.png") );
    ui->menuEdit->addAction(undoAction);
    ui->menuEdit->addAction(redoAction);
//    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), redoAction);
//    ui->menuEdit->insertAction(redoAction, undoAction);
//    ui->editToolBar->insertAction(ui->editToolBar->actions().at(0), redoAction);
//    ui->editToolBar->insertAction(redoAction, undoAction);
}

/**
  show document in new tab
*/
bool MainWindow::createNewTab(Document* doc){
    SgfDocument* sgfDoc = qobject_cast<SgfDocument*>(doc);
    if (sgfDoc){
        BoardWidget* board = new BoardWidget(sgfDoc);
        ui->boardTabWidget->addTab(board, doc->name());
        ui->boardTabWidget->setCurrentWidget(board);
        return true;
    }

    return false;
}

/**
  file new
  create new document in new tab
*/
void MainWindow::on_actionNew_triggered()
{
    fileNew();
}

/**
  board tab changed
*/
void MainWindow::on_boardTabWidget_currentChanged(QWidget* widget)
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(widget);
    if (board == NULL)
        return;

    // change undo stack of current tab to active undo stack
    SgfDocument* doc = board->document();
    undoGroup.setActiveStack(doc->undoStack());
}

/**
  tab close
*/
void MainWindow::on_boardTabWidget_tabCloseRequested(int index)
{
    qDebug("tab close requested");
}
