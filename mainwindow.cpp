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
#include <QDebug>
#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>
#include "mugoapp.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sgfdocument.h"
#include "sgf.h"
#include "sgfcommand.h"

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

    ui->boardTabWidget->removeTab(0);

    // open or create new tab
    if (fname.isEmpty())
        fileNew();
    else
        fileOpen(fname);
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
        BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(i));
        if (maybeSave(board->document()) == false){
            e->ignore();
            return;
        }
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
bool MainWindow::fileOpen(const QString& fname, QTextCodec* codec, bool guessCodec, bool newTab){
    // find document from opened document list,
    // and show document if document found.
    DocViewData::iterator iter = docView.begin();
    while (iter != docView.end()){
        if (iter.key()->fileInfo() == fname){
            ui->boardTabWidget->setCurrentWidget( iter.value().boardWidget );
            return true;
        }
        ++iter;
    }

    if (codec == NULL)
        codec = mugoApp()->defaultCodec();

    // open document if document isn't opened.
    SgfDocument* doc = readSgfDocument(fname, codec, guessCodec);
    if (doc == NULL)
        return false;

    // show document in new tab
    createNewTab(doc);

/*
    // add recent file list
    addRecentFile(fname);
*/

    return true;
}

/**
  overwrite save.
  if document isn't saved, call save as and show save dialog.
*/
bool MainWindow::fileSave(SgfDocument* doc){
    if (doc->fileInfo().suffix() != "sgf")
        return fileSaveAs(doc);
    else
        return fileSaveAs(doc, doc->fileInfo());
}

/**
  show file save dialog, and save document
*/
bool MainWindow::fileSaveAs(SgfDocument* doc){
    // initial path for save dialog
    QString initialPath;
    QFileInfo fi = doc->fileInfo();
    if (fi.absoluteFilePath().isEmpty() == false){
        fi.setFile(fi.absoluteDir().absolutePath(), fi.completeBaseName() + ".sgf");
        initialPath = fi.absoluteFilePath();
    }
    else
        initialPath = doc->name() + ".sgf";

    // show file chooser dialog
    QString fname;
    QTextCodec* codec = doc->codec();
    if (getSaveFileName(initialPath, fname, codec) == false)
        return false;

    // save
    fi.setFile(fname);
    if (fi.suffix().isEmpty())
        fi.setFile( fname + ".sgf" );
    return fileSaveAs(doc, fi);
}

/**
  save document with file name
*/
bool MainWindow::fileSaveAs(SgfDocument* doc, const QFileInfo& fileInfo){
    Go::Sgf sgf(doc->gameList);
    if (sgf.save(fileInfo, doc->codec()) == false)
        return false;

    doc->setDirty(false);
    doc->setFileInfo(fileInfo);

    return true;
}

/**
  close tab.
  save document before close if need.
*/
bool MainWindow::closeTab(BoardWidget* board){
    if (maybeSave(board->document()) == true){
        // remove document and view datas.
        Document* doc = board->document();
//        ViewData& view = docView[doc];
        docView.remove(doc);

        // remove view
//        int index = ui->boardTabWidget->indexOf(board);
//        ui->boardTabWidget->removeTab(index);
        delete board;

        // remove doc
        undoGroup.setActiveStack(0);
        delete doc;

        return true;
    }
    return false;
}

/**
  initialize menu
  create action and actino group
*/
void MainWindow::initializeMenu(){
    // File
    ui->actionNew->setShortcut(QKeySequence::New);
    ui->actionOpen->setShortcut(QKeySequence::Open);
    ui->actionCloseTab->setShortcut(QKeySequence::Close);
    ui->actionSave->setShortcut(QKeySequence::Save);
    ui->actionSaveAs->setShortcut(QKeySequence::SaveAs);
#ifdef Q_WS_WIN
    ui->actionExit->setShortcut(Qt::CTRL + Qt::Key_Q);
#else
    ui->actionExit->setShortcut(QKeySequence::Quit);
#endif

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
        ViewData& data = docView[doc];

        BoardWidget* board = new BoardWidget(sgfDoc);
        ui->boardTabWidget->addTab(board, doc->name());
        ui->boardTabWidget->setCurrentWidget(board);

        data.boardWidget = board;

        connect(sgfDoc, SIGNAL(nodeModified(const Go::NodePtr&)), SLOT(on_sgfDocument_nodeModified(const Go::NodePtr&)));
        connect(board, SIGNAL(nodeChanged(const Go::NodePtr&)), SLOT(on_board_nodeChanged(const Go::NodePtr&)));

        return true;
    }

    return false;
}

/**
  show open file dialog
*/
bool MainWindow::getOpenFileName(QString& fname, QTextCodec*& codec){
    QString filter = "All Go Format (*.sgf *.ugf *.ugi *.ngf *.gib);;Smart Game Format (*.sgf);;ugf (*.ugf *.ugi);;ngf (*.ngf);;gib (*.gib);;All Files (*.*)";

    // use os file dialog
    fname = QFileDialog::getOpenFileName(this, QString(), QString(), filter, NULL);
    if (fname.isEmpty())
        return false;

    // os dialog can't choose codec.
    codec = mugoApp()->defaultCodec();

/*
    // crreate open file dialog
    QFileDialog dlg(this, QString(), QString(), filter);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFile);

    // add encoding combo box
    QLayout* layout = dlg.layout();
    QLabel* label = new QLabel(tr("Encoding:"), &dlg);
    QComboBox* combo = new QComboBox(&dlg);
    layout->addWidget(label);
    layout->addWidget(combo);

    QList<QAction*> actions;
    combo->addItem(tr("Auto Detect"));
    combo->setCurrentIndex(0);
    actions.push_back(NULL);
    combo->insertSeparator(combo->count());
    actions.push_back(NULL);
    foreach(QAction* act, ui->menuReload->actions()){
        if (act->isSeparator() == false){
            combo->addItem(act->text());
            if (codec && encodingActionToCodec[act]->name() == codec->name())
                combo->setCurrentIndex( combo->count() - 1 );
            actions.push_back(act);
        }
        else{
            combo->insertSeparator(combo->count());
            actions.push_back(NULL);
        }
    }

    // show dialog
    if (dlg.exec() != QDialog::Accepted)
        return false;

    if (dlg.selectedFiles().size() == 0)
        return false;

    fname = dlg.selectedFiles()[0];
//    codec = combo->currentIndex() >= 0 ? encodingActionToCodec[actions[combo->currentIndex()]] : mugoApp()->defaultCodec();
*/

    return true;
}

/**
  get save file name
*/
bool MainWindow::getSaveFileName(const QString& initialPath, QString& fname, QTextCodec*& codec){
    QString filter = "Smart Game Format (*.sgf);;All Files (*.*)";
    fname = QFileDialog::getSaveFileName(this, QString(), initialPath, filter, NULL);
    if (fname.isEmpty())
        return false;

    // os dialog can't choose codec.
    codec = mugoApp()->defaultCodec();

/*
    QFileDialog dlg(this, QString(), initialPath, filter);
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    QLayout* layout = dlg.layout();
    QLabel* label = new QLabel(tr("Encoding:"), &dlg);
    QComboBox* combo = new QComboBox(&dlg);
    layout->addWidget(label);
    layout->addWidget(combo);

    QList<QAction*> actions;
    foreach(QAction* act, ui->menuReload->actions()){
        if (act->isSeparator() == false){
            combo->addItem(act->text());
            if (encodingActionToCodec[act]->name() == codec->name())
                combo->setCurrentIndex( combo->count() - 1 );
            actions.push_back(act);
        }
        else{
            combo->insertSeparator(combo->count());
            actions.push_back(NULL);
        }
    }

    if (dlg.exec() != QDialog::Accepted)
        return false;

    if (dlg.selectedFiles().size() == 0)
        return false;

    fname = dlg.selectedFiles()[0];
    codec = encodingActionToCodec[actions[combo->currentIndex()]];
*/

    return true;
}

/**
  read sgf file
*/
SgfDocument* MainWindow::readSgfDocument(const QString& fname, QTextCodec* codec, bool guessCodec){
    QFileInfo info(fname);
    QString ext = info.suffix().toLower();

    Go::NodeList gameList;
    if (ext.compare("sgf") == 0){
        Go::Sgf sgf(gameList);
        sgf.load(fname, codec, guessCodec);
        SgfDocument* doc = new SgfDocument(gameList, this);
        doc->setFileInfo(QFileInfo(fname));
        return doc;
    }

    return NULL;
}

/**
 maybe save
*/
bool MainWindow::maybeSave(SgfDocument* doc){
    if (doc->dirty() == false)
        return true;

    QMessageBox::StandardButton ret =
                    QMessageBox::warning(this, APP_NAME,
                        tr("%1 has been modified.\n"
                           "Do you want to save your changes?").arg(doc->name()),
                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave(doc);
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

/**
  udpate view with node information
*/
void MainWindow::updateView(const Go::NodePtr& node){
    if (ui->commentEdit->toPlainText() != node->comment())
        ui->commentEdit->setPlainText(node->comment());
}

/**
  create new document in new tab
*/
void MainWindow::on_actionNew_triggered()
{
    fileNew();
}

/**
  open document in new tab
*/
void MainWindow::on_actionOpen_triggered()
{
    QString fname;
    QTextCodec* codec = NULL;
    if (getOpenFileName(fname, codec) == false)
        return;

    fileOpen(fname, codec, codec == NULL);
}

/**
  save document
*/
void MainWindow::on_actionSave_triggered()
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // save active document
    fileSave(board->document());
}

/**
  save as document
*/
void MainWindow::on_actionSaveAs_triggered()
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // save active document
    fileSaveAs(board->document());
}

/**
  exit application
*/
void MainWindow::on_actionExit_triggered()
{
    // window close
    close();
}

/**
  node modified
*/
void MainWindow::on_sgfDocument_nodeModified(const Go::NodePtr& node)
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // if modified node isn't current node, return
    if (board->currentNode() != node)
        return;

    // update view
    updateView(node);
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
    undoGroup.setActiveStack( board->document()->undoStack() );

    // update view
    updateView(board->currentNode());
}

/**
  tab close
*/
void MainWindow::on_boardTabWidget_tabCloseRequested(int index)
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(index));
    if (board == NULL)
        return;

    // close requested tab
    closeTab(board);
}

/**
  current node changed
*/
void MainWindow::on_board_nodeChanged(const Go::NodePtr& node){
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // if sender isn't active, can't update view
    if (sender() != board)
        return;
    updateView(node);
}

/**
  comment text changed
*/
void MainWindow::on_commentEdit_textChanged()
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // create new node, and add to document
    if (board->currentNode()->comment() != ui->commentEdit->toPlainText())
        board->document()->undoStack()->push( new SetCommentCommand(board->document(), board->currentNode(), ui->commentEdit->toPlainText()) );
}
