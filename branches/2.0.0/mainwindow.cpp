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
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStandardItemModel>
#include "mugoapp.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "boardwidget.h"
#include "sgf.h"
#include "sgfdocument.h"


Q_DECLARE_METATYPE(Go::NodePtr);


/**
  Constructor
*/
MainWindow::MainWindow(const QString& fname, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    docID(0)
{
    ui->setupUi(this);

    // default codec
    defaultCodec = QTextCodec::codecForName("UTF-8");

    // keyboard shortcut
    setKeyboardShortcut();

    // Edit -> undo/redo
    QAction* undoAction = undoGroup.createUndoAction(this);
    QAction* redoAction = undoGroup.createRedoAction(this);
    undoAction->setIcon( QIcon(":/res/undo.png") );
    redoAction->setIcon( QIcon(":/res/redo.png") );
    ui->menuEdit->addAction(undoAction);
    ui->menuEdit->addAction(redoAction);
    ui->editToolBar->addAction(undoAction);
    ui->editToolBar->addAction(redoAction);
//    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), redoAction);
//    ui->menuEdit->insertAction(redoAction, undoAction);

    // window -> toolbars menu
    ui->menuToolbars->addAction( ui->fileToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->editToolBar->toggleViewAction() );

    // window (dock view)
    ui->menuWindow->addAction( ui->commentDockWidget->toggleViewAction() );
    ui->menuWindow->addAction( ui->branchDockWidget->toggleViewAction() );
    ui->menuWindow->addAction( ui->collectionDockWidget->toggleViewAction() );
    ui->menuWindow->addAction( ui->undoDockWidget->toggleViewAction() );

    // default view
    ui->collectionDockWidget->hide();
    ui->undoDockWidget->hide();

    // undo view
    ui->undoView->setGroup(&undoGroup);

    // open or new tab
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
  changeEvent
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
  get active board
*/
BoardWidget* MainWindow::currentBoard(){
    QWidget* widget = ui->boardTabWidget->currentWidget();
    BoardWidget* board = qobject_cast<BoardWidget*>(widget);
    return board;
}

/**
  Set Keyboard Shortcut
*/
void MainWindow::setKeyboardShortcut(){
    ui->actionNew->setShortcut(QKeySequence::New);
    ui->actionOpen->setShortcut(QKeySequence::Open);
    ui->actionSave->setShortcut(QKeySequence::Save);
    ui->actionExit->setShortcut(QKeySequence::Quit);
}

/**
  file New
*/
void MainWindow::fileNew(int xsize, int ysize, double komi, int handicap)
{
    SgfDocument* doc = new SgfDocument(defaultCodec, this);
    doc->gameList[0]->gameInformation->xsize = xsize;
    doc->gameList[0]->gameInformation->ysize = ysize;
    doc->gameList[0]->gameInformation->komi  = komi;
    doc->gameList[0]->gameInformation->handicap = handicap;
    doc->setDocName( tr("Untitled-%1").arg(++docID) );
    BoardWidget* board = new BoardWidget(doc, this);
    addDocument(board);
}

/**
  file Open
*/
bool MainWindow::fileOpen(const QString& fname)
{
    SgfDocument* doc = new SgfDocument(defaultCodec, this);
    if (doc->open(fname, true) == false){
        QMessageBox::critical(this, QString(), tr("File open error: %1").arg(fname));
        delete doc;
        return false;
    }
    BoardWidget* board = new BoardWidget(doc, this);
    addDocument(board);

    return true;
}

/**
    save
*/
bool MainWindow::fileSave(Document* doc){
    if (doc->getFileName().isEmpty())
        fileSaveAs(doc);

    if (doc->save(doc->getFileName()) == false){
        QMessageBox::critical(this, QString(), tr("File save error: %1").arg(doc->getFileName()));
        return false;
    }
    return true;
}

/**
    save
*/
bool MainWindow::fileSaveAs(Document* doc){
    QString filter = "Smart Game Format (*.sgf);;All Files (*.*)";
    QString fname = QFileDialog::getSaveFileName(this, QString(), QString(doc->getDocName()), filter, NULL);
    if (fname.isEmpty())
        return false;

    QFileInfo fi(fname);
    if (fi.suffix().isEmpty())
        fi.setFile( fname + ".sgf" );
    if (doc->save(fi.absoluteFilePath()) == false){
        QMessageBox::critical(this, QString(), tr("File save error: %1").arg(fname));
        return false;
    }
    return true;
}

/**
  close tab
*/
bool MainWindow::closeTab(int index){
    BoardWidget* boardWidget = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(index));
    SgfDocument* doc = boardWidget->document();
    TabData& tabData = tabDatas[doc];

    // delete branch widget
    ui->branchStackedWidget->removeWidget(tabDatas[doc].branchWidget);
    delete tabData.branchWidget;

    // delete collection model
    delete tabData.collectionModel;

    // delete board widget
    ui->boardTabWidget->removeTab(index);
    delete boardWidget;

    // delete document
    tabDatas.remove(doc);
    delete doc;

    return true;
}

/**
  addDocument
*/
void MainWindow::addDocument(BoardWidget* board)
{
    // create widget
    QTreeWidget* branchWidget = new QTreeWidget;
    QStandardItemModel* model = new QStandardItemModel;

    // save tab data
    TabData td;
    td.boardWidget  = board;
    td.branchWidget = branchWidget;
    td.branchType   = branchMode;
    td.collectionModel = model;
    tabDatas[board->document()] = td;

    // initialize branch widget
    QTreeWidgetItem* dummy = new QTreeWidgetItem(QStringList(""));
    createBranchWidget( board, dummy, dummy, Go::NodePtr(), board->getCurrentGame() );
    for (int i=0; i<dummy->childCount();){
        QTreeWidgetItem* item = dummy->child(i);
        dummy->removeChild(item);
        branchWidget->invisibleRootItem()->addChild(item);
    }

    branchWidget->setHeaderHidden(true);
    branchWidget->setIndentation(17);
    ui->branchStackedWidget->addWidget(branchWidget);
    connect(branchWidget,
            SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            SLOT(on_branchWidget_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

    // collection
    model->setHorizontalHeaderLabels(QStringList() << tr("White") << tr("Black") << tr("Game Name") << tr("Date") << tr("Result"));
    Go::NodeList gameList = board->document()->gameList;
    foreach(Go::NodePtr game, gameList){
        QList<QStandardItem*> items;
        items.push_back( new QStandardItem(game->gameInformation->whitePlayer) );
        items.push_back( new QStandardItem(game->gameInformation->blackPlayer) );
        items.push_back( new QStandardItem(game->gameInformation->date) );
        items.push_back( new QStandardItem(game->gameInformation->gameName.isEmpty() ? game->gameInformation->event : game->gameInformation->gameName) );
        items.push_back( new QStandardItem(game->gameInformation->result) );
        model->appendRow(items);
    }

    // document
    connect(board->document(), SIGNAL(nodeAdded(Go::NodePtr)), SLOT(on_document_nodeAdded(Go::NodePtr)));

    // board
    connect(board, SIGNAL(currentNodeChanged(Go::NodePtr)), SLOT(on_boardWidget_currentNodeChanged(Go::NodePtr)));

    // create new tab
    int n = ui->boardTabWidget->addTab(board, board->document()->getDocName());
    ui->boardTabWidget->setCurrentIndex(n);
}

/**
  create branch tree widget
*/
void MainWindow::createBranchWidget(BoardWidget* board, QTreeWidgetItem* parent1, QTreeWidgetItem* parent2, Go::NodePtr parentNode, Go::NodePtr node){
    // create tree item widget
    QTreeWidgetItem* item = createBranchItem(board, node);
    if (item->parent())
        item->parent()->removeChild(item);

    Go::NodePtr parent1Node = parent1->data(0, Qt::UserRole).value<Go::NodePtr>();
    QTreeWidgetItem* parentWidget = NULL;
    if ((!parentNode || parentNode->childNodes.size() == 1) && (parent1Node == NULL || parent1Node->childNodes.size() == 1)){
        parent1->addChild(item);
        parentWidget = parent1;
    }
    else if (parentNode->childNodes.empty() == false){
        parent2->addChild(item);
        parentWidget = parent2;
    }

    foreach(Go::NodePtr childNode, node->childNodes){
        createBranchWidget(board, parentWidget, item, node, childNode);
    }
}

/**
  create branch tree item from node
*/
QTreeWidgetItem* MainWindow::createBranchItem(BoardWidget* board, Go::NodePtr node){
    // icons
    static QIcon blackIcon(":/res/black_128.png");
    static QIcon whiteIcon(":/res/white_128.png");
    static QIcon greenIcon(":/res/green_64.png");

    // if item exist return exist item
    QTreeWidgetItem* item = tabDatas[board->document()].nodeToTreeItem[node];
    if (item != NULL)
        return item;

    // get node type
    QString str;
    if (node->isStone() == false && node->gameInformation)
        str = tr("Game Information");
    else if (node->isStone() == false && !node->gameInformation)
        str = tr("Other");
    else if (node->isStone() && node->isPass())
        str = tr("Pass");
    else
        str = board->getCoordinateString(node, false);

    // create tree item
    item = new QTreeWidgetItem(QStringList(str));
    tabDatas[board->document()].nodeToTreeItem[node] = item;

    // set icon
    if (node->isBlack())
        item->setIcon(0, blackIcon);
    else if (node->isWhite())
        item->setIcon(0, whiteIcon);
    else
        item->setIcon(0, greenIcon);

    // set nodeptr to tree item
    QVariant v;
    v.setValue(node);
    item->setData(0, Qt::UserRole, v);

    // return created item
    return item;
}

/**
  slot
  File -> New
*/
void MainWindow::on_actionNew_triggered()
{
    fileNew();
}

/**
  Slot
  File -> Open
*/
void MainWindow::on_actionOpen_triggered()
{
    QString filter = "All Go Format (*.sgf *.ugf *.ugi *.ngf *.gib);;Smart Game Format (*.sgf);;ugf (*.ugf *.ugi);;ngf (*.ngf);;gib (*.gib);;All Files (*.*)";
    QString fname = QFileDialog::getOpenFileName(this, QString(), QString(), filter, NULL);
    if (fname.isEmpty())
        return;
    fileOpen(fname);
}

/**
  Slot
  File -> Save
*/
void MainWindow::on_actionSave_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
            return;
    fileSave(board->document());
}

/**
  Slot
  File -> Save As
*/
void MainWindow::on_actionSaveAs_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
            return;
    fileSaveAs(board->document());
}

/**
  Slot
  File -> Exit
*/
void MainWindow::on_actionExit_triggered()
{
    close();
}

/**
  Slot
  Help -> About...
*/
void MainWindow::on_actionAbuot_triggered()
{
    QString html = "<html><p>%1 version %2</p><p>%3</p><p><a href=\"http://code.google.com/p/mugo/\">http://code.google.com/p/mugo/</a></p></html>";
    QMessageBox::about(this, QString(), html.arg(APP_NAME).arg(APP_VERSION).arg(COPYRIGHT));
}

/**
  Slot
  Help -> About Qt...
*/
void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}

/**
  Slot
  node added
*/
void MainWindow::on_document_nodeAdded(Go::NodePtr node){
//    rebuildBranchWidget(node->parent());
//nodeToTreeItem[node];
    // get document and board widget.
    SgfDocument* doc = qobject_cast<SgfDocument*>(sender());
    BoardWidget* board = tabDatas[doc].boardWidget;

    // create new tree item.
    createBranchItem(board, node);

    Go::NodePtr parentNode = node->parent();
    QTreeWidgetItem* parent2 = tabDatas[doc].nodeToTreeItem[parentNode];
    QTreeWidgetItem* parent1 = parent2->parent() ? parent2->parent() : tabDatas[doc].branchWidget->invisibleRootItem();
    createBranchWidget(board, parent1, parent2, parentNode, node);
}

/**
  Slot
  current node is changed
*/
void MainWindow::on_boardWidget_currentNodeChanged(Go::NodePtr node){
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    SgfDocument* doc = board->document();
    QTreeWidgetItem* item = tabDatas[doc].nodeToTreeItem[node];
    tabDatas[doc].branchWidget->setCurrentItem(item);
}

/**
  Slot
  current tab is changed.
*/
void MainWindow::on_boardTabWidget_currentChanged(QWidget* widget)
{
    if (widget == NULL)
        return;
    BoardWidget* boardWidget = qobject_cast<BoardWidget*>(widget);
    TabData& tabData = tabDatas[boardWidget->document()];

    ui->branchStackedWidget->setCurrentWidget( tabData.branchWidget );
    ui->collectionView->setModel(tabData.collectionModel);

    undoGroup.setActiveStack(boardWidget->document()->getUndoStack());
}

/**
  Slot
  tab close request
*/
void MainWindow::on_boardTabWidget_tabCloseRequested(int index)
{
    closeTab(index);
}

/**
  Slot
  current item changed in branch widget.
*/
void MainWindow::on_branchWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/){
    BoardWidget* board = currentBoard();
    if (board == NULL)
            return;

    QVariant v = current->data(0, Qt::UserRole);
    Go::NodePtr node = v.value<Go::NodePtr>();
    if (node == NULL)
        return;
    board->setCurrentNode(node);
}
