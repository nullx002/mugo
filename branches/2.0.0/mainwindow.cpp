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
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStandardItemModel>
#include <QInputDialog>
#include <QUrl>
#include <QHttp>
#include <QProgressDialog>
#include <QLabel>
#include <QComboBox>
#include <QClipboard>
#include "mugoapp.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "boardwidget.h"
#include "gameinformationdialog.h"
#include "exportasciidialog.h"
#include "sgf.h"
#include "sgfdocument.h"
#include "command.h"


Q_DECLARE_METATYPE(Go::NodePtr);


/**
  Constructor
*/
MainWindow::MainWindow(const QString& fname, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , tabChangeGroup(new QActionGroup(this))
    , docID(0)
    , sgfLineWidth(50)

{
    ui->setupUi(this);

    QSettings settings;

    // default codec
    defaultCodec = QTextCodec::codecForName( settings.value("defaultCodec", "UTF-8").toByteArray() );

    // keyboard shortcut
    setKeyboardShortcut();

    // File -> Reload
    ui->fileToolBar->insertAction( ui->actionExportBoardAsImage, ui->menuReload->menuAction() );
    ui->fileToolBar->insertSeparator( ui->actionExportBoardAsImage );

    // File -> Collection
    ui->collectionToolBar->insertAction(ui->collectionToolBar->actions().at(0), ui->collectionDockWidget->toggleViewAction());
    ui->collectionDockWidget->toggleViewAction()->setIcon( QIcon(":/res/collection.png") );

    // Edit -> undo/redo
    QAction* undoAction = undoGroup.createUndoAction(this);
    QAction* redoAction = undoGroup.createRedoAction(this);
    undoAction->setShortcut(QKeySequence::Undo);
    redoAction->setShortcut(QKeySequence::Redo);
    undoAction->setIcon( QIcon(":/res/undo.png") );
    redoAction->setIcon( QIcon(":/res/redo.png") );
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), redoAction);
    ui->menuEdit->insertAction(redoAction, undoAction);
    ui->editToolBar->insertAction(ui->editToolBar->actions().at(0), redoAction);
    ui->editToolBar->insertAction(redoAction, undoAction);

    // window -> toolbars menu
    ui->menuToolbars->addAction( ui->fileToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->editToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->navigationToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->collectionToolBar->toggleViewAction() );

    // window (dock view)
    ui->menuWindow->insertAction( ui->actionPreviousTab, ui->commentDockWidget->toggleViewAction() );
    ui->menuWindow->insertAction( ui->actionPreviousTab, ui->branchDockWidget->toggleViewAction() );
    ui->menuWindow->insertAction( ui->actionPreviousTab, ui->collectionDockWidget->toggleViewAction() );
    ui->menuWindow->insertAction( ui->actionPreviousTab, ui->undoDockWidget->toggleViewAction() );
    ui->menuWindow->insertSeparator( ui->actionPreviousTab );

    // default view
    ui->collectionDockWidget->hide();
    ui->undoDockWidget->hide();

    // undo view
    ui->undoView->setGroup(&undoGroup);

    // encoding
    connect(ui->menuReload->menuAction(), SIGNAL(triggered()), SLOT(on_actionReload_triggered()));
    QActionGroup* encodingGroup = new QActionGroup(this);
    foreach(QAction* act, ui->menuReload->actions()){
        encodingGroup->addAction(act);
        connect(act, SIGNAL(triggered()), SLOT(on_actionReload_triggered()));
    }
    encoding[ui->actionEncodingUTF8] = QTextCodec::codecForName("UTF-8");
    encoding[ui->actionEncodingISO8859_1] = QTextCodec::codecForName("ISO-8859-1");
    encoding[ui->actionEncodingISO8859_2] = QTextCodec::codecForName("ISO-8859-2");
    encoding[ui->actionEncodingISO8859_3] = QTextCodec::codecForName("ISO-8859-3");
    encoding[ui->actionEncodingISO8859_4] = QTextCodec::codecForName("ISO-8859-4");
    encoding[ui->actionEncodingISO8859_5] = QTextCodec::codecForName("ISO-8859-5");
    encoding[ui->actionEncodingISO8859_6] = QTextCodec::codecForName("ISO-8859-6");
    encoding[ui->actionEncodingISO8859_7] = QTextCodec::codecForName("ISO-8859-7");
    encoding[ui->actionEncodingISO8859_8] = QTextCodec::codecForName("ISO-8859-8");
    encoding[ui->actionEncodingISO8859_9] = QTextCodec::codecForName("ISO-8859-9");
    encoding[ui->actionEncodingISO8859_10] = QTextCodec::codecForName("ISO-8859-10");
    encoding[ui->actionEncodingISO8859_11] = QTextCodec::codecForName("TIS-620");
    encoding[ui->actionEncodingISO8859_13] = QTextCodec::codecForName("ISO-8859-13");
    encoding[ui->actionEncodingISO8859_14] = QTextCodec::codecForName("ISO-8859-14");
    encoding[ui->actionEncodingISO8859_15] = QTextCodec::codecForName("ISO-8859-15");
    encoding[ui->actionEncodingISO8859_16] = QTextCodec::codecForName("ISO-8859-16");
    encoding[ui->actionEncodingWindows_1250] = QTextCodec::codecForName("windows-1250");
    encoding[ui->actionEncodingWindows_1251] = QTextCodec::codecForName("windows-1251");
    encoding[ui->actionEncodingWindows_1252] = QTextCodec::codecForName("windows-1252");
    encoding[ui->actionEncodingWindows_1253] = QTextCodec::codecForName("windows-1253");
    encoding[ui->actionEncodingWindows_1254] = QTextCodec::codecForName("windows-1254");
    encoding[ui->actionEncodingWindows_1255] = QTextCodec::codecForName("windows-1255");
    encoding[ui->actionEncodingWindows_1256] = QTextCodec::codecForName("windows-1256");
    encoding[ui->actionEncodingWindows_1257] = QTextCodec::codecForName("windows-1257");
    encoding[ui->actionEncodingWindows_1258] = QTextCodec::codecForName("windows-1258");
    encoding[ui->actionEncodingKoi8_R] = QTextCodec::codecForName("KOI8-R");
    encoding[ui->actionEncodingKoi8_U] = QTextCodec::codecForName("KOI8-U");
    encoding[ui->actionEncodingGB2312] = QTextCodec::codecForName("GB2312");
    encoding[ui->actionEncodingBig5] = QTextCodec::codecForName("Big5");
    encoding[ui->actionEncodingShiftJIS] = QTextCodec::codecForName("Shift_JIS");
    encoding[ui->actionEncodingJIS] = QTextCodec::codecForName("ISO-2022-JP");
    encoding[ui->actionEncodingEucJP] = QTextCodec::codecForName("EUC-JP");
    encoding[ui->actionEncodingKorean] = QTextCodec::codecForName("EUC-KR");

    // status bar
    moveNumberLabel = new QLabel;
    moveNumberLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    moveNumberLabel->setToolTip(tr("Move Number"));
    ui->statusBar->addPermanentWidget(moveNumberLabel, 0);

    capturedLabel = new QLabel;
    capturedLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    capturedLabel->setToolTip(tr("Captured"));
    ui->statusBar->addPermanentWidget(capturedLabel, 0);

    // window settings
    restoreGeometry( settings.value("mainwindowGeometry").toByteArray() );
    restoreState( settings.value("docksState").toByteArray() );
    ui->collectionView->header()->restoreState( settings.value("collectionState").toByteArray() );

    // open or new tab
    if (fname.isEmpty())
        fileNew(defaultCodec);
    else
        fileOpen(defaultCodec, fname, true);
    ui->boardTabWidget->removeTab(0);
}

/**
  Destructor
*/
MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("mainwindowGeometry", saveGeometry());
    settings.setValue("docksState", saveState());
    settings.setValue("collectionState", ui->collectionView->header()->saveState());

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
  changeEvent
*/
void MainWindow::closeEvent(QCloseEvent *e){
    DocumentManager::iterator iter = docManager.begin();
    while (iter != docManager.end()){
        if (maybeSave(iter.key()) ==false){
            e->ignore();
            return;
        }
        ++iter;
    }
    e->accept();
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
  get active board
*/
Document* MainWindow::currentDocument(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return NULL;

    return board->document();
}

/**
  Set Keyboard Shortcut
*/
void MainWindow::setKeyboardShortcut(){
    ui->actionNew->setShortcut(QKeySequence::New);
    ui->actionOpen->setShortcut(QKeySequence::Open);
    ui->actionCloseTab->setShortcut(QKeySequence::Close);
    ui->actionSave->setShortcut(QKeySequence::Save);
    ui->actionExit->setShortcut(QKeySequence::Quit);
    ui->actionCopySgfToClipboard->setShortcut(QKeySequence::Copy);
    ui->actionPasteSgfToNewTab->setShortcut(QKeySequence::Paste);
    ui->actionPreviousTab->setShortcut(QKeySequence::PreviousChild);
    ui->actionNextTab->setShortcut(QKeySequence::NextChild);
}

/**
  set statusbar widget
*/
void MainWindow::setStatusBarWidget(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    moveNumberLabel->setText( tr("Last Move: %1").arg(board->getMoveNumber()) );
    capturedLabel->setText( tr("Dead: White %1 Black %2").arg(board->getCapturedWhite()).arg(board->getCapturedBlack()) );
}

/**
  file New
*/
void MainWindow::fileNew(QTextCodec* codec, int xsize, int ysize, double komi, int handicap)
{
    SgfDocument* doc = new SgfDocument(codec, this);
    doc->gameList[0]->gameInformation->xsize = xsize;
    doc->gameList[0]->gameInformation->ysize = ysize;
    doc->gameList[0]->gameInformation->komi  = komi;
    doc->gameList[0]->gameInformation->handicap = handicap;
    doc->setDocName( newDocumentName() );
    addDocument(doc);
}

/**
  file Open
*/
bool MainWindow::fileOpen(QTextCodec* codec, const QString& fname, bool guessCodec)
{
    DocumentManager::iterator iter = docManager.begin();
    while (iter != docManager.end()){
        if (iter.key()->getFileName() == fname){
            ui->boardTabWidget->setCurrentWidget( iter.value().boardWidget );
            return true;
        }
        ++iter;
    }

    SgfDocument* doc = openDocument(fname, codec, guessCodec);
    if (doc == NULL)
        return false;
    addDocument(doc);

    return true;
}

/**
  Url Open
*/
bool MainWindow::urlOpen(const QUrl& url, bool newTab){
    downloadURL = url;
    downloadBuff.clear();
    downloadNewTab = newTab;

    QHttp* http = new QHttp;
    connect( http, SIGNAL(readyRead(const QHttpResponseHeader&)), SLOT(on_openUrl_ReadReady(const QHttpResponseHeader&)) );
    connect( http, SIGNAL(dataReadProgress(int, int)), SLOT(on_openUrl_UrlReadProgress(int, int)) );
    connect( http, SIGNAL(done(bool)), SLOT(on_openUrl_UrlDone(bool)) );

    QHttp::ConnectionMode mode = url.scheme().toLower() == "https" ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;
    http->setHost(url.host(), mode, url.port() == -1 ? 0 : url.port());
    http->get( url.encodedPath() );

    progressDialog = new QProgressDialog(tr("Downloading SGF File"), "cancel", 0, 100, this);
    connect(progressDialog, SIGNAL(canceled()), this, SLOT(on_openUrl_canceled()));
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->show();
    progressDialog->setValue(0);

//    ui->actionReload->setEnabled(true);

    return true;
}

/**
    save
*/
bool MainWindow::fileSave(Document* doc){
    if (doc->getFileName().isEmpty())
        return fileSaveAs(doc);
    else
        return fileSaveAs(doc, doc->getFileName());
}

/**
    save
*/
bool MainWindow::fileSaveAs(Document* doc){
    QString fname;
    QTextCodec* codec = doc->getCodec();
    if (getSaveFileName(fname, codec) == false)
        return false;

    SgfDocument* sgfDoc = qobject_cast<SgfDocument*>(doc);

    QFileInfo fi(fname);
    if (fi.suffix().isEmpty()){
        if (sgfDoc)
            fi.setFile( fname + ".sgf" );
    }

    doc->setCodec(codec);

    return fileSaveAs(doc, fi.filePath());
}

/**
    save
*/
bool MainWindow::fileSaveAs(Document* doc, const QString& fname){
    if (fname.isEmpty())
        return false;

    SgfDocument* sgfDoc = qobject_cast<SgfDocument*>(doc);
    if (sgfDoc)
        sgfDoc->lineWidth = sgfLineWidth;

    if (doc->save(fname) == false){
        QMessageBox::critical(this, QString(), tr("File save error: %1").arg(fname));
        return false;
    }

    docManager[doc].tabChangeAction->setText( doc->getDocName() );

    return true;
}

/**
  close tab
*/
bool MainWindow::closeTab(int index){
    BoardWidget* boardWidget = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(index));
    if (boardWidget){
        SgfDocument* doc = boardWidget->document();
        return closeDocument(doc);
    }

    return false;
}

/**
 may be save
*/
bool MainWindow::maybeSave(Document* doc){
    if (doc->isDirty() == false)
        return true;

    QMessageBox::StandardButton ret =
                    QMessageBox::warning(this, APP_NAME,
                        tr("%1 has been modified.\n"
                           "Do you want to save your changes?").arg(doc->getDocName()),
                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave(doc);
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

/**
  create document
*/
SgfDocument* MainWindow::openDocument(const QString& fname, QTextCodec* codec, bool guessCodec){
    SgfDocument* doc = new SgfDocument(codec, this);
    if (doc->open(fname, guessCodec) == false){
        QMessageBox::critical(this, QString(), tr("File open error: %1").arg(fname));
        delete doc;
        return NULL;
    }
    return doc;
}

/**
  add document
*/
void MainWindow::addDocument(SgfDocument* doc, BoardWidget* board)
{
    // tab data
    ViewData& view = docManager[doc];

    // create branch widget
    QTreeWidget* branchWidget = new QTreeWidget;
    view.branchWidget = branchWidget;
    view.branchType   = branchMode;

    // create collection model
    QStandardItemModel* model = new QStandardItemModel;
    view.collectionModel = model;

    if (board == NULL){
        board = new BoardWidget(doc, this);
        view.boardWidget  = board;

        connect(board, SIGNAL(currentGameChanged(Go::NodePtr)), SLOT(on_boardWidget_currentGameChanged(Go::NodePtr)));
        connect(board, SIGNAL(currentNodeChanged(Go::NodePtr)), SLOT(on_boardWidget_currentNodeChanged(Go::NodePtr)));

        board->setCurrentGame(doc->gameList.front(), true);
    }
    else{
        view.boardWidget  = board;

        // set document after create view data.
        board->setDocument(doc);
    }

    // initialize branch widget
    ui->branchStackedWidget->addWidget(branchWidget);
    connect(branchWidget,
            SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            SLOT(on_branchWidget_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

    // collection
    createCollectionModel(doc->gameList, model);

    // document
    connect(doc, SIGNAL(modified(bool)), SLOT(on_sgfdocument_modified(bool)));
    connect(doc, SIGNAL(nodeAdded(Go::NodePtr)), SLOT(on_sgfdocument_nodeAdded(Go::NodePtr)));
    connect(doc, SIGNAL(nodeDeleted(Go::NodePtr, bool)), SLOT(on_sgfdocument_nodeDeleted(Go::NodePtr, bool)));
    connect(doc, SIGNAL(nodeModified(Go::NodePtr)), SLOT(on_sgfdocument_nodeModified(Go::NodePtr)));
    connect(doc, SIGNAL(gameAdded(Go::NodePtr, int)), SLOT(on_sgfdocument_gameAdded(Go::NodePtr, int)));
    connect(doc, SIGNAL(gameDeleted(Go::NodePtr, int)), SLOT(on_sgfdocument_gameDeleted(Go::NodePtr, int)));
    connect(doc, SIGNAL(gameMoved(Go::NodePtr, int, int)), SLOT(on_sgfdocument_gameMoved(Go::NodePtr, int, int)));

    // create tab active action
    view.tabChangeAction = new QAction(board);
    view.tabChangeAction->setCheckable(true);
    view.tabChangeAction->setText(doc->getDocName());
    tabChangeGroup->addAction(view.tabChangeAction);
    ui->menuWindow->addAction(view.tabChangeAction);
    connect(view.tabChangeAction, SIGNAL(triggered()), SLOT(on_tabChangeRequest()));

    // create new tab
    int n = ui->boardTabWidget->addTab(board, doc->getDocName());
    ui->boardTabWidget->setCurrentIndex(n);
}

/**
  closeDocument
*/
bool MainWindow::closeDocument(Document* doc, bool save, bool closeTab){
    if (save && maybeSave(doc) == false)
        return false;

    if (undoGroup.activeStack() == doc->getUndoStack())
        undoGroup.setActiveStack(NULL);

    ViewData& view = docManager[doc];

    // delete branch widget
    ui->branchStackedWidget->removeWidget(view.branchWidget);
    delete view.branchWidget;

    // delete collection model
    delete view.collectionModel;

    // delete board widget
    if (closeTab){
        int index = ui->boardTabWidget->indexOf(view.boardWidget);
        ui->boardTabWidget->removeTab(index);
        delete view.boardWidget;
    }

    // delete document
    docManager.remove(doc);
    delete doc;

    return true;
}

/**
  create branch tree widget
*/
void MainWindow::createBranchWidget(Document* doc){
    ViewData& view = docManager[doc];
    view.branchWidget->clear();
    view.nodeToTreeItem.clear();

    QTreeWidgetItem* dummy = new QTreeWidgetItem(QStringList(""));
    createBranchWidget( view.boardWidget, dummy, dummy, dummy, Go::NodePtr(), view.boardWidget->getCurrentGame() );

    int count = dummy->childCount();
    for (int i=0; i<count; ++i){
        QTreeWidgetItem* item = dummy->child(0);
        dummy->removeChild(item);
        view.branchWidget->invisibleRootItem()->addChild(item);
    }
    delete dummy;

    if (view.branchWidget->invisibleRootItem()->childCount() > 0)
        view.branchWidget->setCurrentItem( view.branchWidget->invisibleRootItem()->child(0) );

    view.branchWidget->setHeaderHidden(true);
    view.branchWidget->setIndentation(17);
}

/**
  create branch tree widget
*/
void MainWindow::createBranchWidget(BoardWidget* board, Go::NodePtr node){
    ViewData& view = docManager[board->document()];
    QTreeWidgetItem* parent2 = view.nodeToTreeItem[node];
    QTreeWidgetItem* parent1 = parent2->parent() ? parent2->parent() : view.branchWidget->invisibleRootItem();

    foreach(Go::NodePtr childNode, node->childNodes)
        createBranchWidget(board, view.branchWidget->invisibleRootItem(), parent1, parent2, node, childNode);
}

/**
  create branch tree widget
*/
void MainWindow::createBranchWidget(BoardWidget* board, QTreeWidgetItem* root, QTreeWidgetItem* parent1, QTreeWidgetItem* parent2, Go::NodePtr parentNode, Go::NodePtr node){
    // create tree item widget
    QTreeWidgetItem* item = createBranchItem(board, node);
    QTreeWidgetItem* currentParent = NULL;
    if (item->parent())
        currentParent = item->parent();
    else if (root->indexOfChild(item) >= 0)
        currentParent = root;

    Go::NodePtr parent1Node = parent1->data(0, Qt::UserRole).value<Go::NodePtr>();
    QTreeWidgetItem* parentWidget = NULL;
    int index = 0;
    if ((!parentNode || parentNode->childNodes.size() == 1) && (parent1Node == NULL || parent1Node->childNodes.size() == 1)){
        parentWidget = parent1;
        int parentIndex = parentWidget->indexOfChild(parent2);
        if (parentIndex >= 0)
            index = parentIndex + 1;
    }
    else if (parentNode->childNodes.empty() == false){
        parentWidget = parent2;
        index = parentNode->childNodes.indexOf(node);
    }

    if (currentParent){
        if (currentParent != parentWidget || index != parentWidget->indexOfChild(item)){
            currentParent->removeChild(item);
            parentWidget->insertChild(index, item);
        }
    }
    else
        parentWidget->insertChild(index, item);

    foreach(Go::NodePtr childNode, node->childNodes)
        createBranchWidget(board, root, parentWidget, item, node, childNode);
}

/**
  create branch tree item from node
*/
QTreeWidgetItem* MainWindow::createBranchItem(BoardWidget* board, Go::NodePtr node){
    // icons
    static QIcon blackIcon(":/res/black_128.png");
    static QIcon whiteIcon(":/res/white_128.png");
    static QIcon greenIcon(":/res/green_64.png");

    ViewData& data = docManager[board->document()];

    // if item exist return exist item
    QTreeWidgetItem* item = data.nodeToTreeItem[node];
    if (item != NULL)
        return item;

    // get node type
    QString str = getBranchItemText(board, node);

    // create tree item
    item = new QTreeWidgetItem(QStringList(str));
    data.nodeToTreeItem[node] = item;

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
  get branch tree item's text
*/
QString MainWindow::getBranchItemText(BoardWidget* board, Go::NodePtr node){
    QString str;
    if (node->isStone() == false && !node->gameInformation)
        str = tr("Other");
    else if (node->isStone() && node->isPass())
        str = tr("Pass");
    else if (node->isStone())
        str = board->getCoordinateString(node, false);

    if (node->name.isEmpty() == false)
        str += " " + node->name;

    if (node->comment.isEmpty() == false)
        str += " " + tr("Comment");

    if (str.isEmpty() == false && str[0].isSpace())
        str.remove(0, 1);

    if (node->gameInformation){
        if (str.isEmpty())
            str = tr("Game Information");
        else
            str.insert(0, tr("Info") + " ");
    }

    return str;
}

/**
  create collection mdoel
*/
void MainWindow::createCollectionModel(const Go::NodeList& gameList, QStandardItemModel* model){
    model->clear();
    model->setHorizontalHeaderLabels(QStringList() << tr("White") << tr("Black") << tr("Game Name") << tr("Date") << tr("Result"));

    foreach(Go::NodePtr game, gameList){
        QList<QStandardItem*> items;
        createCollectionModelRow(game, items);
        model->appendRow(items);
    }
}

/**
  create collection model row
*/
void MainWindow::createCollectionModelRow(const Go::NodePtr& game, QList<QStandardItem*>& items){
    items.push_back( new QStandardItem(game->gameInformation->whitePlayer) );
    items.push_back( new QStandardItem(game->gameInformation->blackPlayer) );
    items.push_back( new QStandardItem(game->gameInformation->gameName.isEmpty() ? game->gameInformation->event : game->gameInformation->gameName) );
    items.push_back( new QStandardItem(game->gameInformation->date) );
    items.push_back( new QStandardItem(game->gameInformation->result) );
}

/**
  update caption
*/
void MainWindow::updateCaption(bool updateTab){
    if (updateTab){
        for (int i=0; i<ui->boardTabWidget->count(); ++i){
            BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(i));
            if (board == NULL)
                continue;

            if (board->document()->isDirty())
                ui->boardTabWidget->setTabText(i, board->document()->getDocName() + " *");
            else
                ui->boardTabWidget->setTabText(i, board->document()->getDocName());
        }
    }

    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    Document* doc = board->document();

//    Go::NodePtr game = board->getCurrentGame();
//    Go::GameInformationPtr info = game->gameInformation;
    Go::GameInformationPtr info = board->getCurrentNode()->getInformation();

    QString title;
    if (info->blackPlayer.isEmpty() == false || info->whitePlayer.isEmpty() == false)
        title = tr("%1 %2(W) vs %3 %4(B) Result:%5").arg(info->whitePlayer).arg(info->whiteRank).arg(info->blackPlayer).arg(info->blackRank).arg(info->result);
    else
        title = doc->getDocName();

    if (info->gameName.isEmpty() == false)
        title += " - " + info->gameName;
    else if (info->event.isEmpty() == false)
        title += " - " + info->event;

    if (doc->isDirty())
        title += " *";
    title += " - " APP_NAME;

    setWindowTitle(title);
}

/**
  remove node from NodeToTreeMap
*/
void MainWindow::removeBranchItem(QTreeWidgetItem* parent, NodeTreeMap& map, Go::NodePtr node){
    QTreeWidgetItem* item = map[node];
    if (item->parent() == parent || item->parent() == NULL)
        parent->removeChild(item);
    map.remove(node);

    foreach(Go::NodePtr childNode, node->childNodes)
        removeBranchItem(parent, map, childNode);
}

/**
  get open file name
*/
bool MainWindow::getOpenFileName(QString& fname, QTextCodec*& codec){
    QString filter = "All Go Format (*.sgf *.ugf *.ugi *.ngf *.gib);;Smart Game Format (*.sgf);;ugf (*.ugf *.ugi);;ngf (*.ngf);;gib (*.gib);;All Files (*.*)";
//    QString fname = QFileDialog::getOpenFileName(this, QString(), QString(), filter, NULL);
//    if (fname.isEmpty())
//        return;

    QFileDialog dlg(this, QString(), QString(), filter);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFile);

    QLayout* layout = dlg.layout();
    QLabel* label = new QLabel(tr("Encoding:"), &dlg);
    QComboBox* combo = new QComboBox(&dlg);
    layout->addWidget(label);
    layout->addWidget(combo);

    QList<QAction*> actions;
    combo->addItem(tr("Auto Detect"));
    combo->setCurrentIndex(0);
    actions.push_back(NULL);
    foreach(QAction* act, ui->menuReload->actions()){
        if (act->isSeparator() == false){
            combo->addItem(act->text());
            if (codec && encoding[act]->name() == codec->name())
                combo->setCurrentIndex( combo->count() - 1 );
            actions.push_back(act);
        }
    }

    if (dlg.exec() != QDialog::Accepted)
        return false;

    if (dlg.selectedFiles().size() == 0)
        return false;

    fname = dlg.selectedFiles()[0];
    codec = combo->currentIndex() >= 0 ? encoding[actions[combo->currentIndex()]] : defaultCodec;

    return true;
}

/**
  get save file name
*/
bool MainWindow::getSaveFileName(QString& fname, QTextCodec*& codec){
    QString filter = "Smart Game Format (*.sgf);;All Files (*.*)";
/*
    QString fname = QFileDialog::getSaveFileName(this, QString(), QString(doc->getDocName()), filter, NULL);
    if (fname.isEmpty())
        return false;
*/

    QFileDialog dlg(this, QString(), QString(), filter);
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
            if (encoding[act]->name() == codec->name())
                combo->setCurrentIndex( combo->count() - 1 );
            actions.push_back(act);
        }
    }

    if (dlg.exec() != QDialog::Accepted)
        return false;

    if (dlg.selectedFiles().size() == 0)
        return false;

    fname = dlg.selectedFiles()[0];
    codec = encoding[actions[combo->currentIndex()]];

    return true;
}

/**
  slot
  File -> New
*/
void MainWindow::on_actionNew_triggered()
{
    fileNew(defaultCodec);
}

/**
  Slot
  File -> Open
*/
void MainWindow::on_actionOpen_triggered()
{
    QString fname;
    QTextCodec* codec = NULL;
    if (getOpenFileName(fname, codec) == false)
        return;

    if (codec == NULL)
        fileOpen(defaultCodec, fname, true);
    else
        fileOpen(codec, fname, false);
}

/**
  Slot
  File -> reload
*/
void MainWindow::on_actionReload_triggered()
{
    // get action
    QAction* act = qobject_cast<QAction*>(sender());
    if (act == NULL)
        return;

    // get document
    BoardWidget* board = currentBoard();
    if (board == NULL || (board->document()->getFileName().isEmpty() && board->document()->getUrl().isEmpty()))
        return;

    //  return if cancelled.
    if (maybeSave(board->document()) == false)
        return;

    QTextCodec* codec = encoding.find(act) != encoding.end() ? encoding[act] : board->document()->getCodec();
    if (board->document()->getFileName().isEmpty() == false){
        SgfDocument* doc = openDocument(board->document()->getFileName(), codec, false);
        if (doc == NULL){
            closeDocument(board->document(), false);
            return;
        }

        closeDocument(board->document(), false, false);
        addDocument(doc, board);
    }
    else if (board->document()->getUrl().isEmpty() == false){
        board->document()->setCodec(codec);
        urlOpen( QUrl(board->document()->getUrl()), false );
    }
}

/**
  Slot
  File -> open URL
*/
void MainWindow::on_actionOpenURL_triggered()
{
    QInputDialog dlg(this);
    dlg.resize( 400, dlg.size().height() );
    dlg.setLabelText( tr("Enter the URL of a SGF file.") );
    if (dlg.exec() != QDialog::Accepted)
        return;

    urlOpen( QUrl(dlg.textValue()), true );
}

/**
  Slot
  File -> Close Tab
*/
void MainWindow::on_actionCloseTab_triggered()
{
    closeTab( ui->boardTabWidget->currentIndex() );
}

/**
  Slot
  File -> Close All Tabs
*/
void MainWindow::on_actionCloseAllTabs_triggered()
{
    int count = ui->boardTabWidget->count();
    for (int i=0; i<count; ++i)
    closeTab(0);
}

/**
  Slot
  File -> Save
*/
void MainWindow::on_actionSave_triggered()
{
    Document* doc = currentDocument();
    if (doc == NULL)
        return;

    fileSave(doc);
}

/**
  Slot
  File -> Save As
*/
void MainWindow::on_actionSaveAs_triggered()
{
    Document* doc = currentDocument();
    if (doc == NULL)
        return;
    fileSaveAs(doc);
}

/**
  Slot
  File -> Collection -> Import
*/
void MainWindow::on_actionCollectionImport_triggered()
{
    SgfDocument* doc = qobject_cast<SgfDocument*>(currentDocument());
    if (doc == NULL)
        return;

    // select file
    QString fname;
    QTextCodec* codec = NULL;
    if (getOpenFileName(fname, codec) == false)
        return;

    // open document
    SgfDocument* tmpDoc;
    if (codec == NULL)
        tmpDoc = openDocument(fname, defaultCodec, true);
    else
        tmpDoc = openDocument(fname, codec, false);

    if (tmpDoc == NULL)
        return;

    // add documents
    doc->getUndoStack()->push( new AddGameListCommand(doc, tmpDoc->gameList) );
}

/**
  Slot
  File -> Collection -> Extract
*/
void MainWindow::on_actionCollectionExtract_triggered()
{
    // current document
    SgfDocument* doc = qobject_cast<SgfDocument*>(currentDocument());
    if (doc == NULL)
        return;

    // selecting game
    QModelIndex index = ui->collectionView->currentIndex();
    if (index.row() < 0)
        return;

    // convert selecting game to sgf
    Go::NodePtr game = doc->gameList[index.row()];
    Go::NodeList gameList;
    gameList.push_back(game);

    Go::Sgf sgf;
    sgf.set(gameList);
    sgf.codec = doc->getCodec();

    // load sgf as new document
    SgfDocument* newDoc = new SgfDocument(doc->getCodec());
    newDoc->set(sgf);
    newDoc->setDocName( newDocumentName() );
    addDocument(newDoc);
    newDoc->setDirty();
}

/**
  Slot
  File -> Export Image
*/
void MainWindow::on_actionExportBoardAsImage_triggered()
{
    QMessageBox::warning(this, QString(), "Not Implemented");
/*
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    QImage image(640, 400, QImage::Format_ARGB32);
    QPainter p(&image);
    board->render(&p);

    QString filter = "PNG image(*.png);;Bitmap image(*.bmp);;JPEG image(*.jpeg *.jpg);;TIFF image(*.tiff *.tif)";
    QString selectedFilter;
    QString fname = QFileDialog::getSaveFileName(this, QString(), QString(), filter, &selectedFilter);
    if (fname.isEmpty())
        return;

    QFileInfo fi(fname);
    if (fi.suffix().isEmpty()){
        if (selectedFilter.indexOf("PNG") == 0)
            fname += ".png";
        else if (selectedFilter.indexOf("Bitmap") == 0)
            fname += ".bmp";
        else if (selectedFilter.indexOf("JPEG") == 0)
            fname += ".jpg";
        else if (selectedFilter.indexOf("TIFF") == 0)
            fname += ".tif";
    }
    image.save(fname);
*/
}

/**
  Slot
  File -> Export Ascii
*/
void MainWindow::on_actionExportAsciiToClipboard_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    ExportAsciiDialog dlg(this, board->getBoardBuffer());
    dlg.exec();
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
  copy sgf to clipboard
*/
void MainWindow::on_actionCopySgfToClipboard_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    QString str;
    QTextStream stream(&str, QIODevice::WriteOnly);

    Go::NodeList gameList;
    gameList.push_back(board->getCurrentGame());

    Go::Sgf sgf;
    sgf.set(gameList);
    sgf.saveStream(stream);
    stream.flush();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(str);
}

/**
  Slot
  copy current branch to clipboard
*/
void MainWindow::on_actionCopyCurrentBranchToClipboard_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    QString str("(");
    QString line;

    const Go::NodeList& nodeList = board->getCurrentNodeList();
    foreach(const Go::NodePtr& node, nodeList){
        Go::Sgf::Node sgfNode;
        sgfNode.set(node);

        QStringList strList = sgfNode.toStringList();
        if (strList.empty() == false)
            strList[0].insert(0, ";");

        foreach(const QString& s, strList){
            if (line.isEmpty() == false && line.size() + s.size() > sgfLineWidth){
                str += line + "\n";
                line.clear();
            }
            line += s;
        }
    }

    str.append(line);
    str.push_back(')');

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(str);
}

/**
  Slot
  paste sgf from clipboard to new tab
*/
void MainWindow::on_actionPasteSgfToNewTab_triggered()
{
    // get clipboard text
    QClipboard *clipboard = QApplication::clipboard();
    QString str = clipboard->text();

    // read sgf
    Go::Sgf sgf;
    QString::iterator first = str.begin();
    if (sgf.readStream(first, str.end()) == false)
        return;

    // create new document
    SgfDocument* doc = new SgfDocument(QTextCodec::codecForName("UTF-8"));
    if (doc->read(newDocumentName(), str.toUtf8(), false) == false){
        delete doc;
        return;
    }
    addDocument(doc);
    doc->setDirty();
}

/**
  Slot
  paste sgf from clipboard into collection
*/
void MainWindow::on_actionPasteSgfIntoCollection_triggered()
{
    SgfDocument* doc = qobject_cast<SgfDocument*>(currentDocument());
    if (doc == NULL)
        return;

    // get clipboard text
    QClipboard *clipboard = QApplication::clipboard();
    QString str = clipboard->text();

    // read sgf
    Go::Sgf sgf;
    QString::iterator first = str.begin();
    if (sgf.readStream(first, str.end()) == false)
        return;

    Go::NodeList gameList;
    sgf.get(gameList);

    // add sgf into collection
    doc->getUndoStack()->push( new AddGameListCommand(doc, gameList) );
}

/**
  Slot
  Edit -> Delete After Current
*/
void MainWindow::on_actionDeleteAfterCurrent_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    SgfDocument* doc = board->document();

    doc->getUndoStack()->push( new DeleteNodeCommand(doc, board->getCurrentNode(), true) );
}

/**
  Slot
  Edit -> Delete Current Only
*/
void MainWindow::on_actionDeleteCurrentOnly_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    SgfDocument* doc = board->document();

    doc->getUndoStack()->push( new DeleteNodeCommand(doc, board->getCurrentNode(), false) );
}

/**
  Slot
  Edit -> Pass
*/
void MainWindow::on_actionPass_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr currentNode = board->getCurrentNode();
    Go::NodePtr childNode;
    if (currentNode->nextColor == Go::black)
        childNode = Go::createBlackNode(currentNode);
    else if (currentNode->nextColor == Go::white)
        childNode = Go::createWhiteNode(currentNode);
    else if (currentNode->color == Go::black)
        childNode = Go::createWhiteNode(currentNode);
    else if (currentNode->color == Go::white)
        childNode = Go::createBlackNode(currentNode);
    else
        return;

    board->addItem(currentNode, childNode, -1);
    board->setCurrentNode(childNode);
}

/**
  Slot
  Edit -> Game Information
*/
void MainWindow::on_actionGameInformation_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr node = board->getCurrentNode();
    GameInformationDialog dlg(this, board->document(), node->getInformation());
    if (dlg.exec() != QDialog::Accepted)
        return;
}

/**
  Slot
  Navigation -> Move First
*/
void MainWindow::on_actionNavigationMoveFirst_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    const Go::NodeList& nodeList = board->getCurrentNodeList();
    board->setCurrentNode(nodeList.front());
}

/**
  Slot
  Navigation -> Fast Rewind
*/
void MainWindow::on_actionNavigationFastRewind_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->back(5);
}

/**
  Slot
  Navigation -> Move Next
*/
void MainWindow::on_actionNavigationMovePrevious_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->back();
}

/**
  Slot
  Navigation -> Move Last
*/
void MainWindow::on_actionNavigationMoveLast_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    const Go::NodeList& nodeList = board->getCurrentNodeList();
    board->setCurrentNode(nodeList.last());
}

/**
  Slot
  Navigation -> Fast Forward
*/
void MainWindow::on_actionNavigationFastForward_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->forward(5);
}

/**
  Slot
  Navigation -> Move Next
*/
void MainWindow::on_actionNavigationMoveNext_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->forward();
}


/**
  Slot
  Window -> previous tab
*/
void MainWindow::on_actionPreviousTab_triggered()
{
    if (ui->boardTabWidget->count() == 0)
        return;

    int index = ui->boardTabWidget->currentIndex();
    if (--index < 0)
        index = ui->boardTabWidget->count() - 1;
    ui->boardTabWidget->setCurrentIndex(index);
}

/**
  Slot
  Window -> next tab
*/
void MainWindow::on_actionNextTab_triggered()
{
    if (ui->boardTabWidget->count() == 0)
        return;

    int index = ui->boardTabWidget->currentIndex();
    if (++index == ui->boardTabWidget->count())
        index = 0;
    ui->boardTabWidget->setCurrentIndex(index);
}

/**
  Slot
  Window -> tab name
*/
void MainWindow::on_tabChangeRequest(){
    QWidget* widget = qobject_cast<QWidget*>(sender()->parent());
    ui->boardTabWidget->setCurrentWidget(widget);
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
  document modified
*/
void MainWindow::on_sgfdocument_modified(bool){
    updateCaption(true);
 }

/**
  Slot
  node added
*/
void MainWindow::on_sgfdocument_nodeAdded(Go::NodePtr node){
    // get document and board widget.
    SgfDocument* doc = qobject_cast<SgfDocument*>(sender());
    BoardWidget* board = docManager[doc].boardWidget;

    // create new tree item.
    createBranchWidget(board, node->parent());

    // set current node
    board->setCurrentNode(node);
}

/**
  Slot
  node deleted
*/
void MainWindow::on_sgfdocument_nodeDeleted(Go::NodePtr node, bool removeChildren){
    SgfDocument* doc = qobject_cast<SgfDocument*>(sender());
    ViewData& view = docManager[doc];
    view.boardWidget->setCurrentNode(node->parent());

    QTreeWidgetItem* item   = view.nodeToTreeItem[node];
    QTreeWidgetItem* parent = item->parent() ? item->parent() : view.branchWidget->invisibleRootItem();

    if (removeChildren == false){
        int index = parent->indexOfChild(item);
        int cnt = item->childCount();
        for (int i=cnt-1; i>=0; --i){
            QTreeWidgetItem* child = item->child(i);
            item->removeChild(child);
            parent->insertChild(index+1, child);
        }
    }
    else
        removeBranchItem(parent, view.nodeToTreeItem, node);

    view.nodeToTreeItem.remove(node);
    delete item;

    if (!node->parent())
        return;

    // remake tree widget
    createBranchWidget(docManager[doc].boardWidget, node->parent());
}

/**
  Slot
  node modified
*/
void MainWindow::on_sgfdocument_nodeModified(Go::NodePtr node){
    BoardWidget* board = currentBoard();
    if (board->getCurrentNode() == node){
        if (node->comment != ui->commentWidget->toPlainText())
            ui->commentWidget->setPlainText( node->comment );
    }

    SgfDocument* doc = qobject_cast<SgfDocument*>(sender());
    QTreeWidgetItem* item = docManager[doc].nodeToTreeItem[node];
    if (item)
        item->setText( 0, getBranchItemText(board, node) );
}

/**
  Slot
  game added to collection
*/
void MainWindow::on_sgfdocument_gameAdded(Go::NodePtr game, int index){
    SgfDocument* doc = qobject_cast<SgfDocument*>(sender());
    if (doc == NULL)
        return;

    // add game into collection model
    QList<QStandardItem*> items;
    createCollectionModelRow(game, items);
    docManager[doc].collectionModel->insertRow(index, items);
}

/**
  Slot
  game deleted from collection
*/
void MainWindow::on_sgfdocument_gameDeleted(Go::NodePtr /*game*/, int index){
    SgfDocument* doc = qobject_cast<SgfDocument*>(sender());
    if (doc == NULL)
        return;

    // delete game from collection model
    docManager[doc].collectionModel->removeRow(index);
}

/**
  Slot
  game moved in collection
*/
void MainWindow::on_sgfdocument_gameMoved(Go::NodePtr /*game*/, int from, int to){
    SgfDocument* doc = qobject_cast<SgfDocument*>(sender());
    if (doc == NULL)
        return;

    // move items
    QStandardItemModel* model = docManager[doc].collectionModel;
    QList<QStandardItem*> items = model->takeRow(from);
    model->insertRow(to, items);

    ui->collectionView->setCurrentIndex( model->index(to, 0) );
}

/**
  Slot
  current node changed
*/
void MainWindow::on_boardWidget_currentNodeChanged(Go::NodePtr node){
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    Document* doc = board->document();
    QTreeWidgetItem* item = docManager[doc].nodeToTreeItem[node];
    if (item)
        docManager[doc].branchWidget->setCurrentItem(item);

    if (board == currentBoard()){
        ui->commentWidget->setPlainText(node->comment);
        setStatusBarWidget();
    }

    updateCaption(false);
}

/**
  Slot
  current game changed
*/
void MainWindow::on_boardWidget_currentGameChanged(Go::NodePtr /*game*/){
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    if (board == NULL)
        return;
    SgfDocument* doc = board->document();

    createBranchWidget(doc);

    updateCaption(false);
}

/**
  Slot
  current tab is changed.
*/
void MainWindow::on_boardTabWidget_currentChanged(QWidget* widget)
{
    BoardWidget* boardWidget = qobject_cast<BoardWidget*>(widget);
    if (boardWidget  == NULL)
        return;
    ViewData& view = docManager[boardWidget->document()];

    // window menu
    view.tabChangeAction->setChecked(true);

    // branch tree
    ui->branchStackedWidget->setCurrentWidget( view.branchWidget );

    // collection
    ui->collectionView->setModel(view.collectionModel);
    int n = boardWidget->document()->gameList.indexOf(boardWidget->getCurrentGame());
    ui->collectionView->setCurrentIndex( ui->collectionView->model()->index(n, 0) );

    // comment
    ui->commentWidget->setPlainText(boardWidget->getCurrentNode()->comment);

    // status bar
    setStatusBarWidget();


    undoGroup.setActiveStack(boardWidget->document()->getUndoStack());

    updateCaption(false);
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
    if (current == NULL)
        return;

    BoardWidget* board = currentBoard();
    if (board == NULL)
            return;

    QVariant v = current->data(0, Qt::UserRole);
    Go::NodePtr node = v.value<Go::NodePtr>();
    if (node == NULL)
        return;
    board->setCurrentNode(node);
}

/**
  Slot
  comment edited.
*/
void MainWindow::on_commentWidget_textChanged(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
            return;

    SgfDocument* doc  = board->document();
    Go::NodePtr  node = board->getCurrentNode();

    if (node->comment == ui->commentWidget->toPlainText())
        return;

    static SetCommentCommand* lastCommand = NULL;

    if (lastCommand && doc->getUndoStack()->canUndo()){
        const QUndoCommand* last = doc->getUndoStack()->command( doc->getUndoStack()->count() - 1 );
        if (last == lastCommand && node == lastCommand->getNode()){
            lastCommand->setComment( ui->commentWidget->toPlainText() );
            return;
        }
    }

    lastCommand = new SetCommentCommand(doc, node, ui->commentWidget->toPlainText());
    doc->getUndoStack()->push(lastCommand);
}

/**
  Slot
  collection view -> double clicked
*/
void MainWindow::on_collectionView_doubleClicked(QModelIndex index)
{
    if (index.row() < 0)
        return;

    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr game = board->document()->gameList[index.row()];
    if (board->getCurrentGame() == game)
        return;

    board->document()->getUndoStack()->push( new SetCurrentGameCommand(board, game) );
}

/**
  Slot
  Collection -> Move Up
*/
void MainWindow::on_actionCollectionMoveUp_triggered()
{
    SgfDocument* doc = qobject_cast<SgfDocument*>(currentDocument());
    if (doc == NULL)
        return;

    QModelIndex index = ui->collectionView->currentIndex();
    if (index.row() <= 0)
        return;

    doc->getUndoStack()->push( new MoveUpInCollectionCommand(doc, doc->gameList[index.row()]) );
}

/**
  Slot
  Collection -> Move Down
*/
void MainWindow::on_actionCollectionMoveDown_triggered()
{
    SgfDocument* doc = qobject_cast<SgfDocument*>(currentDocument());
    if (doc == NULL)
        return;

    QModelIndex index = ui->collectionView->currentIndex();
    if (index.row() < 0 || index.row() >= doc->gameList.size() - 1)
        return;

    doc->getUndoStack()->push( new MoveDownInCollectionCommand(doc, doc->gameList[index.row()]) );
}

/**
  Slot
  Collection -> Delete
*/
void MainWindow::on_actionCollectionDelete_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    SgfDocument* doc = board->document();
    if (doc == NULL)
        return;

    QModelIndex index = ui->collectionView->currentIndex();
    if (index.row() < 0 || index.row() >= doc->gameList.size())
        return;

    if (doc->gameList.indexOf(board->getCurrentGame()) == index.row()){
        QMessageBox::warning(this, QString(), tr("Remove sgf from collection failed because this sgf is editing."));
        return;
    }

    Go::NodeList gameList;
    gameList.push_back(doc->gameList[index.row()]);
    doc->getUndoStack()->push( new DeleteGameListCommand(doc, gameList) );
}

/**
  Slot
  Open URL
*/
void MainWindow::on_openUrl_ReadReady(const QHttpResponseHeader& resp){
    QHttp* http = qobject_cast<QHttp*>(sender());

    switch (resp.statusCode()) {
        case 200:                   // Ok
        case 301:                   // Moved Permanently
        case 302:                   // Found
        case 303:                   // See Other
        case 307:                   // Temporary Redirect
            // these are not error conditions
            downloadBuff.append( http->readAll() );
            break;

        default:
            QMessageBox::information(this, APP_NAME,
                                     tr("Download failed: %1.")
                                     .arg(resp.reasonPhrase()));
            http->abort();
            break;
    }
}

/**
  Slot
  Open URL
*/
void MainWindow::on_openUrl_UrlReadProgress(int done, int total){
    progressDialog->setMaximum(total);
    progressDialog->setValue(done);
}

/**
  Slot
  Open URL
*/
void MainWindow::on_openUrl_UrlDone(bool error){
    QByteArray buf = downloadBuff;
    downloadBuff.clear();
    sender()->deleteLater();
    delete progressDialog;

    if (error == true)
        return;

    QString docName = downloadURL.toString();
    int p = docName.lastIndexOf('/');
    if (p >= 0)
        docName = docName.mid(p+1);

    if (downloadNewTab){
        SgfDocument* doc = new SgfDocument(defaultCodec);
        if (doc->read(docName, buf, true) == false){
            delete doc;
            return;
        }
        doc->setUrl(downloadURL.toString());
        addDocument(doc);
    }
    else{
        BoardWidget* board = currentBoard();
        if (board == NULL)
            return;

        SgfDocument* doc = new SgfDocument(board->document()->getCodec());
        if (doc->read(docName, buf, false) == false){
            delete doc;
            return;
        }
        doc->setUrl(downloadURL.toString());
        addDocument(doc, board);
    }
}
