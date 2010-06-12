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
#include <QInputDialog>
#include <QUrl>
#include <QHttp>
#include <QLabel>
#include <QComboBox>
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
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), redoAction);
    ui->menuEdit->insertAction(redoAction, undoAction);
    ui->editToolBar->insertAction(ui->editToolBar->actions().at(0), redoAction);
    ui->editToolBar->insertAction(redoAction, undoAction);

    // File -> Reload
    ui->fileToolBar->addAction( ui->menuReload->menuAction() );

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

    // open or new tab
    if (fname.isEmpty())
        fileNew(defaultCodec);
    else
        fileOpen(defaultCodec, fname);
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
    ui->actionSave->setShortcut(QKeySequence::Save);
    ui->actionExit->setShortcut(QKeySequence::Quit);
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
    doc->setDocName( tr("Untitled-%1").arg(++docID) );
    addDocument(doc);
}

/**
  file Open
*/
bool MainWindow::fileOpen(QTextCodec* codec, const QString& fname)
{
    DocumentManager::iterator iter = docManager.begin();
    while (iter != docManager.end()){
        if (iter.key()->getFileName() == fname){
            ui->boardTabWidget->setCurrentWidget( iter.value().boardWidget );
            return true;
        }
        ++iter;
    }

    SgfDocument* doc = new SgfDocument(codec, this);
    if (doc->open(fname, true) == false){
        QMessageBox::critical(this, QString(), tr("File open error: %1").arg(fname));
        delete doc;
        return false;
    }
    addDocument(doc);

    return true;
}

/**
  Url Open
*/
bool MainWindow::urlOpen(const QUrl& url){
    downloadURL = url;
    downloadBuff.clear();

    QHttp* http = new QHttp;
    connect( http, SIGNAL(readyRead(const QHttpResponseHeader&)), SLOT(on_openUrl_ReadReady(const QHttpResponseHeader&)) );
    connect( http, SIGNAL(dataReadProgress(int, int)), SLOT(on_openUrl_UrlReadProgress(int, int)) );
    connect( http, SIGNAL(done(bool)), SLOT(on_openUrl_UrlDone(bool)) );

    QHttp::ConnectionMode mode = url.scheme().toLower() == "https" ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;
    http->setHost(url.host(), mode, url.port() == -1 ? 0 : url.port());

    http->get( url.encodedPath() );

//    progressDialog = new QProgressDialog(tr("Downloading SGF File"), "cancel", 0, 100, this);
//    connect(progressDialog, SIGNAL(canceled()), this, SLOT(openUrlCancel()));
//    progressDialog->setWindowModality(Qt::WindowModal);
//    progressDialog->show();
//    progressDialog->setValue(0);

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
            actions.push_back(act);
        }
    }

    if (dlg.exec() != QDialog::Accepted)
        return false;

    if (dlg.selectedFiles().size() == 0)
        return false;

    if (combo->currentIndex() >= 0)
        doc->setCodec(encoding[actions[combo->currentIndex()]]);

    return fileSaveAs(doc, dlg.selectedFiles()[0]);
}

/**
    save
*/
bool MainWindow::fileSaveAs(Document* doc, const QString& fname){
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
    if (boardWidget){
        SgfDocument* doc = boardWidget->document();
        return closeDocument(doc);
    }

    return false;
}

/**
  addDocument
*/
void MainWindow::addDocument(SgfDocument* doc, BoardWidget* board)
{
    // create widget
    if (board == NULL)
        board = new BoardWidget(doc, this);
    else
        board->setDocument(doc);
    QTreeWidget* branchWidget = new QTreeWidget;
    QStandardItemModel* model = new QStandardItemModel;

    // save tab data
    ViewData view;
    view.boardWidget  = board;
    view.branchWidget = branchWidget;
    view.branchType   = branchMode;
    view.collectionModel = model;
    docManager[doc] = view;

    // initialize branch widget
    QTreeWidgetItem* dummy = new QTreeWidgetItem(QStringList(""));
    createBranchWidget( board, dummy, dummy, dummy, Go::NodePtr(), board->getCurrentGame() );
    for (int i=0; i<dummy->childCount();){
        QTreeWidgetItem* item = dummy->child(i);
        dummy->removeChild(item);
        branchWidget->invisibleRootItem()->addChild(item);
    }
    if (branchWidget->invisibleRootItem()->childCount() > 0)
        branchWidget->setCurrentItem( branchWidget->invisibleRootItem()->child(0) );

    branchWidget->setHeaderHidden(true);
    branchWidget->setIndentation(17);
    ui->branchStackedWidget->addWidget(branchWidget);
    connect(branchWidget,
            SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            SLOT(on_branchWidget_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

    // collection
    model->setHorizontalHeaderLabels(QStringList() << tr("White") << tr("Black") << tr("Game Name") << tr("Date") << tr("Result"));
    Go::NodeList gameList = doc->gameList;
    foreach(Go::NodePtr game, gameList){
        QList<QStandardItem*> items;
        items.push_back( new QStandardItem(game->gameInformation->whitePlayer) );
        items.push_back( new QStandardItem(game->gameInformation->blackPlayer) );
        items.push_back( new QStandardItem(game->gameInformation->gameName.isEmpty() ? game->gameInformation->event : game->gameInformation->gameName) );
        items.push_back( new QStandardItem(game->gameInformation->date) );
        items.push_back( new QStandardItem(game->gameInformation->result) );
        model->appendRow(items);
    }

    // document
    connect(doc, SIGNAL(nodeAdded(Go::NodePtr)), SLOT(on_sgfdocument_nodeAdded(Go::NodePtr)));
    connect(doc, SIGNAL(nodeDeleted(Go::NodePtr, bool)), SLOT(on_sgfdocument_nodeDeleted(Go::NodePtr, bool)));

    // board
    connect(board, SIGNAL(currentNodeChanged(Go::NodePtr)), SLOT(on_boardWidget_currentNodeChanged(Go::NodePtr)));

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
  update caption
*/
void MainWindow::updateCaption(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    Document* doc = board->document();

    Go::NodePtr game = board->getCurrentGame();
    Go::GameInformationPtr info = game->gameInformation;

    QString title;
    if (info->blackPlayer.isEmpty() == false || info->whitePlayer.isEmpty() == false)
        title = tr("%1 %2(W) vs %3 %4(B) Result:%5").arg(info->whitePlayer).arg(info->whiteRank).arg(info->blackPlayer).arg(info->blackRank).arg(info->result);
    else
        title = doc->getDocName();

    if (info->gameName.isEmpty() == false)
        title += " " + info->gameName;
    else if (info->event.isEmpty() == false)
        title += " " + info->event;

    if (doc->isDirty())
        title += " *";
    title += " - " APP_NAME;

    setWindowTitle(title);

    int index = ui->boardTabWidget->indexOf(board);
    if (doc->isDirty())
        ui->boardTabWidget->setTabText(index, doc->getDocName() + " *");
    else
        ui->boardTabWidget->setTabText(index, doc->getDocName());
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
    foreach(QAction* act, ui->menuReload->actions()){
        if (act->isSeparator() == false){
            combo->addItem(act->text());
            actions.push_back(act);
        }
    }

    if (dlg.exec() != QDialog::Accepted)
        return;

    if (dlg.selectedFiles().size() == 0)
        return;

    QTextCodec* codec = combo->currentIndex() >= 0 ? encoding[actions[combo->currentIndex()]] : defaultCodec;
    fileOpen(codec, dlg.selectedFiles()[0]);
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
    if (board == NULL || board->document()->getFileName().isEmpty())
        return;

    //  return if cancelled.
    if (maybeSave(board->document()) == false)
        return;

    QTextCodec* codec = encoding.find(act) != encoding.end() ? encoding[act] : board->document()->getCodec();
    SgfDocument* doc = new SgfDocument(codec, this);
    if (doc->open(board->document()->getFileName(), false) == false){
        QMessageBox::critical( this, QString(), tr("File open error: %1").arg(board->document()->getFileName()) );
        closeDocument(board->document(), false);
        delete doc;
        return;
    }
    closeDocument(board->document(), false, false);
    addDocument(doc, board);
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

    urlOpen( QUrl(dlg.textValue()) );
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
    updateCaption();
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
    updateCaption();
}

/**
  Slot
  File -> Export Image
*/
void MainWindow::on_actionExportBoardAsImage_triggered()
{
    BoardWidget* board = currentBoard();

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
}

/**
  Slot
  File -> Export Ascii
*/
void MainWindow::on_actionExportAsciiToClipboard_triggered()
{
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
  Edit -> Delete After Current
*/
void MainWindow::on_actionDeleteAfterCurrent_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    SgfDocument* doc = board->document();

    doc->deleteNodeCommand( board->getCurrentNode(), true );
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

    doc->deleteNodeCommand( board->getCurrentNode(), false );
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
  current node is changed
*/
void MainWindow::on_boardWidget_currentNodeChanged(Go::NodePtr node){
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    Document* doc = board->document();
    QTreeWidgetItem* item = docManager[doc].nodeToTreeItem[node];
    if (item)
        docManager[doc].branchWidget->setCurrentItem(item);

    updateCaption();
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

    ui->branchStackedWidget->setCurrentWidget( view.branchWidget );
    ui->collectionView->setModel(view.collectionModel);

    undoGroup.setActiveStack(boardWidget->document()->getUndoStack());

    updateCaption();
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
void MainWindow::on_openUrl_UrlReadProgress(int, int){
}

/**
  Slot
  Open URL
*/
void MainWindow::on_openUrl_UrlDone(bool error){
    QByteArray buf = downloadBuff;
    downloadBuff.clear();
    sender()->deleteLater();

    if (error == true)
        return;

    QString fname = downloadURL.toString();
    int p = fname.lastIndexOf('/');
    if (p >= 0)
        fname = fname.mid(p+1);

    SgfDocument* doc = new SgfDocument(defaultCodec);
    if (doc->read(fname, buf, true) == false)
        return;

    addDocument(doc);
}
