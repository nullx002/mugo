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
#include <QTreeWidget>
#include <QPlainTextEdit>
#include <QMessageBox>
#include "mugoapp.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
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

    ui->undoDockWidget->setVisible(false);
    ui->undoView->setGroup(&undoGroup);

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
    if (closeAllDocuments())
        e->accept();
    else
        e->ignore();
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

    // if codec isn't designated, use default codec of application
    if (codec == NULL)
        codec = mugoApp()->defaultCodec();

    // open document if document isn't opened.
    QFileInfo info(fname);
    QString ext = info.suffix().toLower();

    GoDocument* doc = NULL;
    if (ext.compare("sgf") == 0){
        doc = new SgfDocument(this);
        if (doc->open(fname, codec, guessCodec) == false){
            delete doc;
            return false;
        }
    }

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
bool MainWindow::fileSave(GoDocument* doc){
    if (doc->fileInfo().suffix() != "sgf")
        return fileSaveAs(doc);
    else
        return fileSaveAs(doc, doc->fileInfo());
}

/**
  show file save dialog, and save document
*/
bool MainWindow::fileSaveAs(GoDocument* doc){
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
bool MainWindow::fileSaveAs(GoDocument* doc, const QFileInfo& fileInfo){
    return doc->save(fileInfo.filePath());
}

/**
  close document.
  save document before close if need.
*/
bool MainWindow::closeDocument(GoDocument* doc){
    if (maybeSave(doc) == false)
        return false;

    // delete views
    ViewData& view = docView[doc];
    delete view.commentEdit;
    delete view.branchWidget;
    delete view.boardWidget;

    // delete document
    undoGroup.setActiveStack(0);
    docView.remove(doc);
    delete doc;

    return true;
}

/**
  close all documents
*/
bool MainWindow::closeAllDocuments(){
    while (ui->boardTabWidget->count()){
        BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(0));
        if (closeDocument(board->document()) == false)
            return false;
    }

    return true;
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
        connect(sgfDoc, SIGNAL(nodeAdded(const Go::NodePtr&)), SLOT(on_sgfDocument_nodeAdded(const Go::NodePtr&)));
        connect(sgfDoc, SIGNAL(nodeModified(const Go::NodePtr&)), SLOT(on_sgfDocument_nodeModified(const Go::NodePtr&)));
        ViewData& data = docView[doc];

        // new comment widget
        QPlainTextEdit* comment = new QPlainTextEdit;
        data.commentEdit = comment;
        connect(comment, SIGNAL(textChanged()), SLOT(on_commentEdit_textChanged()));
        ui->commentStackedWidget->addWidget(comment);

        // new tree widget
        QTreeWidget* branch = new QTreeWidget;
        connect(branch, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(on_branchWidget_currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
        data.branchWidget = branch;
        branch->setHeaderHidden(true);
        branch->setIndentation(17);
        ui->branchStackedWidget->addWidget(branch);

        // new board widget
        BoardWidget* board = new BoardWidget;
        data.boardWidget = board;
        connect(board, SIGNAL(gameChanged(const Go::NodePtr&)), SLOT(on_board_gameChanged(const Go::NodePtr&)));
        connect(board, SIGNAL(nodeChanged(const Go::NodePtr&)), SLOT(on_board_nodeChanged(const Go::NodePtr&)));
        board->setDocument(sgfDoc);
        ui->boardTabWidget->addTab(board, sgfDoc->name());
        ui->boardTabWidget->setCurrentWidget(board);

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
 maybe save
*/
bool MainWindow::maybeSave(GoDocument* doc){
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
  udpate all views by node information
*/
void MainWindow::updateView(GoDocument* doc){
    ViewData& view = docView[doc];
    if (view.commentEdit->toPlainText() != view.boardWidget->currentNode()->comment())
        view.commentEdit->setPlainText(view.boardWidget->currentNode()->comment());
}

/**
  create tree items in branch view
*/
void MainWindow::createBranchItems(BoardWidget* board, QTreeWidget* branch, const Go::NodePtr& game){
    createBranchItems(board, branch->invisibleRootItem(), game, false);
}

/**
  create tree items in branch view
*/
void MainWindow::createBranchItems(BoardWidget* board, QTreeWidgetItem* parent, const Go::NodePtr& node, bool shouldCreateChild){
    QTreeWidgetItem* item = createBranchItem(board, node);
    parent->addChild(item);
    foreach (const Go::NodePtr& child, node->children())
        if (shouldCreateChild || node->children().size() > 1)
            createBranchItems(board, item, child, node->children().size() > 1);
        else
            createBranchItems(board, parent, child, false);
}

/**
  add tree item in branch view
*/
void MainWindow::addBranchItem(QTreeWidget* branch, const Go::NodePtr& node){
}

/**
  create tree item for branch view
*/
QTreeWidgetItem* MainWindow::createBranchItem(BoardWidget* board, const Go::NodePtr& node){
    static QIcon green(":/res/green_64.png");
    static QIcon black(":/res/black_128.png");
    static QIcon white(":/res/white_128.png");

    QTreeWidgetItem* item = new QTreeWidgetItem();

    // set node icon
    if (node->color() == Go::eBlack)
        item->setIcon(0, black);
    else if (node->color() == Go::eWhite)
        item->setIcon(0, white);
    else
        item->setIcon(0, green);

    // set node name
    QStringList nodeName;
    if (node->isStone())
        nodeName.push_back( Go::coordinateString(board->xsize(), board->ysize(), node->x(), node->y(), false) );
    else if (node->information())
        nodeName.push_back("Game Info");

    if (node->name().isEmpty() == false)
        nodeName.push_back(node->name());

    if (node->comment().isEmpty() == false)
        nodeName.push_back("Comment");

    item->setText(0, nodeName.join(" "));

    // set node to tree item data
    item->setData(0, Qt::UserRole, QVariant::fromValue<Go::NodePtr>(node));

    return item;
}

/**
  File -> New
  create new document in new tab
*/
void MainWindow::on_actionNew_triggered()
{
    fileNew();
}

/**
  File -> Open
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
  File -> Close Tab
  close active tab
*/
void MainWindow::on_actionCloseTab_triggered()
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // close active tab
    closeDocument(board->document());
}

/**
  File -> Close All Tab
  close all tab
*/
void MainWindow::on_actionCloseAllTabs_triggered()
{
    // close all tab
    closeAllDocuments();
}

/**
  File -> Save
  save the document of active tab to file
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
  File -> Save As
  save the document of active tab to the specified file
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
  File -> Exit
  exit application
*/
void MainWindow::on_actionExit_triggered()
{
    // window close
    close();
}

/**
  node added
*/
void MainWindow::on_sgfDocument_nodeAdded(const Go::NodePtr& node)
{
    // get document
    GoDocument* doc = qobject_cast<GoDocument*>(sender());
    if (doc == NULL)
        return;

    // get tree widget
    QTreeWidget* branch = docView[doc].branchWidget;

    addBranchItem(branch, node);
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
    updateView(board->document());
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

    // set undo stack of new current tab to active stack.
    undoGroup.setActiveStack( board->document()->undoStack() );

    // activate comment widget of new current tab
    ui->commentStackedWidget->setCurrentWidget( docView[board->document()].commentEdit );

    // activate branch widget of new current tab
    ui->branchStackedWidget->setCurrentWidget( docView[board->document()].branchWidget );

    // update view
    updateView(board->document());
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
    closeDocument(board->document());
}

/**
  current game changed
*/
void MainWindow::on_board_gameChanged(const Go::NodePtr& game){
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    if (board == NULL)
        return;

    // get branch widget
    QTreeWidget* branch = docView[board->document()].branchWidget;
    if (branch == NULL)
        return;

    branch->clear();
    createBranchItems(board, branch, game);
}

/**
  current node changed
*/
void MainWindow::on_board_nodeChanged(const Go::NodePtr& node){
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    if (board == NULL)
        return;

    updateView(board->document());
}

/**
  comment text changed
*/
void MainWindow::on_commentEdit_textChanged()
{
    // find sender's document
    Document* doc = NULL;
    DocViewData::const_iterator iter = docView.begin();
    while (iter != docView.end()){
        if (iter->commentEdit == sender()){
            doc = iter.key();
            break;
        }
        ++iter;
    }

    SgfDocument* sgfDoc = qobject_cast<SgfDocument*>(doc);
    if (sgfDoc == NULL)
        return;

    static Go::Node* lastCommentNode = NULL;
    static SetCommentCommand* lastCommentCommand = NULL;

    ViewData& view = docView[doc];
    if (view.boardWidget->currentNode()->comment() == view.commentEdit->toPlainText())
        return;

    if (lastCommentNode != view.boardWidget->currentNode().data()){
        lastCommentCommand = new SetCommentCommand(sgfDoc, view.boardWidget->currentNode(), view.commentEdit->toPlainText());
        doc->undoStack()->push(lastCommentCommand);
    }
    else
        lastCommentCommand->setComment(view.commentEdit->toPlainText());

    lastCommentNode = view.boardWidget->currentNode().data();
}

/**
  current item was changed in branch view
*/
void MainWindow::on_branchWidget_currentItemChanged(QTreeWidgetItem* current,QTreeWidgetItem* previous){
    // find sender's document
    Document* doc = NULL;
    DocViewData::const_iterator iter = docView.begin();
    while (iter != docView.end()){
        if (iter->branchWidget == sender()){
            doc = iter.key();
            break;
        }
        ++iter;
    }

    // document isn't found
    if (doc == NULL)
        return;

    // select node no board widget
    Go::NodePtr node = current->data(0, Qt::UserRole).value<Go::NodePtr>();
    docView[doc].boardWidget->setNode(node);
}
