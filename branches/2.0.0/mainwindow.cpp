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
#include <QBuffer>
#include <QProgressDialog>
#include <QLabel>
#include <QComboBox>
#include <QClipboard>
#include "mugoapp.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "boardwidget.h"
#include "setupdialog.h"
#include "gameinformationdialog.h"
#include "newdocumentdialog.h"
#include "exportasciidialog.h"
#include "countterritorydialog.h"
#include "sgf.h"
#include "sgfdocument.h"
#include "command.h"


Q_DECLARE_METATYPE(Go::NodePtr);
Q_DECLARE_METATYPE(QAction*);


/**
  Constructor
*/
MainWindow::MainWindow(const QString& fname, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , tabChangeGroup(new QActionGroup(this))
    , docID(0)
    , sgfLineWidth(50)
    , stepsOfFastMove(FAST_MOVE_STEPS)
{
    ui->setupUi(this);

    QSettings settings;

    // keyboard shortcut
    setKeyboardShortcut();

    // menu
    createMenu();

    // encoding
    createEncodingAction();

    // status bar
    moveNumberLabel = new QLabel;
    moveNumberLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    moveNumberLabel->setToolTip(tr("Move Number"));
    ui->statusBar->addPermanentWidget(moveNumberLabel, 0);

    capturedLabel = new QLabel;
    capturedLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    capturedLabel->setToolTip(tr("Captured"));
    ui->statusBar->addPermanentWidget(capturedLabel, 0);

    encodingLabel = new QLabel;
    encodingLabel ->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    encodingLabel ->setToolTip(tr("Encoding"));
    ui->statusBar->addPermanentWidget(encodingLabel, 0);

    // window settings
    restoreGeometry( settings.value("mainwindowGeometry").toByteArray() );
    restoreState( settings.value("docksState").toByteArray() );
    ui->collectionView->header()->restoreState( settings.value("collectionState").toByteArray() );

    // recent files
    maxRecentFiles = settings.value("maxRecentFiles", 6).toInt();
    updateRecentFileActions();

    // open or create new tab
    if (fname.isEmpty())
        fileNew(mugoApp()->defaultCodec());
    else
        fileOpen(fname, mugoApp()->defaultCodec(), true);
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
//    ui->actionExit->setShortcut(QKeySequence::Quit);
    ui->actionExit->setShortcut( QKeySequence("Ctrl+Q") );
    ui->actionCopySgfToClipboard->setShortcut(QKeySequence::Copy);
    ui->actionPasteSgfToNewTab->setShortcut(QKeySequence::Paste);
    ui->actionPreviousTab->setShortcut(QKeySequence::PreviousChild);
    ui->actionNextTab->setShortcut(QKeySequence::NextChild);
}

/**
  create menu
*/
void MainWindow::createMenu(){
    // File -> Reload
    ui->fileToolBar->insertAction( ui->actionExportBoardAsImage, ui->menuReload->menuAction() );
    ui->fileToolBar->insertSeparator( ui->actionExportBoardAsImage );
    connect(ui->menuReload->menuAction(), SIGNAL(triggered()), SLOT(on_actionReload_triggered()));

    // File -> Collection
    ui->collectionToolBar->insertAction(ui->collectionToolBar->actions().at(0), ui->collectionDockWidget->toggleViewAction());
    ui->collectionDockWidget->toggleViewAction()->setIcon( QIcon(":/res/collection.png") );

    // Edit -> undo/redo
    undoAction = undoGroup.createUndoAction(this);
    redoAction = undoGroup.createRedoAction(this);
    undoAction->setShortcut(QKeySequence::Undo);
    redoAction->setShortcut(QKeySequence::Redo);
    undoAction->setIcon( QIcon(":/res/undo.png") );
    redoAction->setIcon( QIcon(":/res/redo.png") );
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), redoAction);
    ui->menuEdit->insertAction(redoAction, undoAction);
    ui->editToolBar->insertAction(ui->editToolBar->actions().at(0), redoAction);
    ui->editToolBar->insertAction(redoAction, undoAction);

    // Edit -> Alternate Move, Stones & Markers
    connect(ui->menuStonesAndMarkers->menuAction(), SIGNAL(triggered()), SLOT(on_actionStonesAndMarkers_triggered()));
    ui->menuStonesAndMarkers->menuAction()->setCheckable(true);
    ui->menuStonesAndMarkers->setIcon(ui->actionAddLabel->icon());
    ui->menuStonesAndMarkers->menuAction()->setData( QVariant::fromValue(ui->actionAddLabel) );
    ui->editToolBar->insertAction(ui->editToolBar->actions().at(4), ui->menuStonesAndMarkers->menuAction());

    editGroup = new QActionGroup(this);
    editGroup->addAction(ui->actionAlternateMove);
    editGroup->addAction(ui->actionAddBlackStone);
    editGroup->addAction(ui->actionAddWhiteStone);
    editGroup->addAction(ui->actionAddEmpty);
    editGroup->addAction(ui->actionAddLabel);
    editGroup->addAction(ui->actionAddLabelManually);
    editGroup->addAction(ui->actionAddCircle);
    editGroup->addAction(ui->actionAddCross);
    editGroup->addAction(ui->actionAddTriangle);
    editGroup->addAction(ui->actionAddSquare);
    editGroup->addAction(ui->actionDeleteMarker);

    // Edit -> Annotation
    QActionGroup* nodeAnnotationGroup = new QActionGroup(this);
    nodeAnnotationGroup->addAction(ui->actionNoNodeAnnotation);
    nodeAnnotationGroup->addAction(ui->actionEven);
    nodeAnnotationGroup->addAction(ui->actionGoodForBlack);
    nodeAnnotationGroup->addAction(ui->actionVeryGoodForBlack);
    nodeAnnotationGroup->addAction(ui->actionGoodForWhite);
    nodeAnnotationGroup->addAction(ui->actionVeryGoodForWhite);
    nodeAnnotationGroup->addAction(ui->actionUnclear);

    QActionGroup* moveAnnotationGroup = new QActionGroup(this);
    moveAnnotationGroup->addAction(ui->actionNoMoveAnnotation);
    moveAnnotationGroup->addAction(ui->actionGoodMove);
    moveAnnotationGroup->addAction(ui->actionVeryGoodMove);
    moveAnnotationGroup->addAction(ui->actionBadMove);
    moveAnnotationGroup->addAction(ui->actionVeryBadMove);
    moveAnnotationGroup->addAction(ui->actionDoubtfulMove);
    moveAnnotationGroup->addAction(ui->actionInterestingMove);

    // View -> Move Number
    ui->viewToolBar->insertAction(ui->actionBranchMode, ui->menuMoveNumber->menuAction());
    ui->menuMoveNumber->setIcon(QIcon(":/res/showmovenumber.png"));
    ui->menuMoveNumber->menuAction()->setCheckable(true);
    connect(ui->menuMoveNumber->menuAction(), SIGNAL(triggered()), SLOT(on_actionMoveNumber_triggered()));
    QActionGroup* moveNumberGroup = new QActionGroup(this);
    moveNumberGroup->addAction(ui->actionNoMoveNumber);
    moveNumberGroup->addAction(ui->actionLast1Move);
    moveNumberGroup->addAction(ui->actionLast2Moves);
    moveNumberGroup->addAction(ui->actionLast5Moves);
    moveNumberGroup->addAction(ui->actionLast10Moves);
    moveNumberGroup->addAction(ui->actionLast20Moves);
    moveNumberGroup->addAction(ui->actionLast50Moves);
    moveNumberGroup->addAction(ui->actionAllMoves);

    // View -> Show Variations
    QActionGroup* showVariationsGroup = new QActionGroup(this);
    showVariationsGroup->addAction(ui->actionNoMarkup);
    showVariationsGroup->addAction(ui->actionShowChildren);
    showVariationsGroup->addAction(ui->actionShowSiblings);

    // Window -> toolbars menu
    ui->menuToolbars->addAction( ui->fileToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->editToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->navigationToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->viewToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->collectionToolBar->toggleViewAction() );

    // Window (dock view)
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
}

/**
  set preferences

  initialize boardwidget
*/
void MainWindow::setPreferences(BoardWidget* board){
    QSettings settings;

    // board
    board->setBoardColor( settings.value("board/boardColor", BOARD_COLOR).value<QColor>() );
    board->setBoardImage( settings.value("board/boardPath").toString() );
    board->setBoardType( (BoardWidget::Preference::ResourceType)settings.value("board/boardType", 0).toInt() );
    board->setCoordinateColor( settings.value("board/coordinateColor", COORDINATE_COLOR).value<QColor>() );
    board->setCoordinateFont( settings.value("board/coordinateFont", "Sans").toString() );
    board->setBackgroundColor( settings.value("board/bgColor", BG_COLOR).value<QColor>() );
    board->setTutorBackgroundColor( settings.value("board/tutorBgColor", TUTOR_BG_COLOR).value<QColor>() );

    // stone
    board->setWhiteStoneColor( settings.value("stone/whiteColor", BLACK_STONE_COLOR).value<QColor>() );
    board->setWhiteStoneImage( settings.value("stone/whitePath").toString() );
    board->setWhiteStoneType( (BoardWidget::Preference::ResourceType)settings.value("stone/whiteType").toInt() );
    board->setBlackStoneColor( settings.value("stone/blackColor", BLACK_STONE_COLOR).value<QColor>() );
    board->setBlackStoneImage( settings.value("stone/blackPath").toString() );
    board->setBlackStoneType( (BoardWidget::Preference::ResourceType)settings.value("stone/blackType").toInt() );

    // marker
    board->setBranchColor( settings.value("marker/branchColor", BRANCH_COLOR).value<QColor>() );
    board->setFocusColor( settings.value("marker/focusColor", FOCUS_COLOR).value<QColor>() );
    board->setFocusType( settings.value("marker/focusType").toInt() );
    board->setLabelType( (BoardWidget::Preference::LabelType)settings.value("marker/labelType").toInt() );
    board->setLabelFont( settings.value("marker/labelFont", "Sans").toString() );

    // navigation
    stepsOfFastMove = settings.value("navigation/stepsOfFastMove", FAST_MOVE_STEPS).toInt();
    board->setAutomaticReplayInterval( settings.value("navigation/autoReplayInterval", AUTO_REPLAY_INTERVAL).toInt() );

    // sound
    board->setPlaySound( settings.value("sound/play", true).toBool() );
    if (settings.value("sound/type").toInt() == 1)
        board->setMoveSoundFile( settings.value("sound/path").toString() );
    else
        board->setMoveSoundFile( MOVE_SOUND_FILE );
}

/**
  create encoding action
*/
void MainWindow::createEncodingAction(){
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


    QList<QAction*> actions;
    QList<QTextCodec*> codecs;
    encodingGroup = new QActionGroup(this);
    foreach(QAction* act, ui->menuReload->actions()){
        encodingGroup->addAction(act);
        connect(act, SIGNAL(triggered()), SLOT(on_actionReload_triggered()));

        actions.push_back(act);
        codecs.push_back( encoding.value(act) );
    }

    mugoApp()->setEncodingActions(actions);
    mugoApp()->setCodecs(codecs);
}

/**
  set statusbar widget
*/
void MainWindow::updateStatusBar(BoardWidget* board){
    if (board == NULL){
        board = currentBoard();
        if (board == NULL)
            return;
    }

    moveNumberLabel->setText( tr("Last Move: %1 (%2)").arg(board->getMoveNumber()).arg(board->document()->positionString(board->getCurrentNode())) );
    capturedLabel->setText( tr("Prisoners: White %1 Black %2").arg(board->getCapturedWhite()).arg(board->getCapturedBlack()) );
    encodingLabel->setText( encodingGroup->checkedAction()->text() );
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
bool MainWindow::fileOpen(const QString& fname, QTextCodec* codec, bool guessCodec)
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
    addRecentFile(fname);

    return true;
}

/**
  Url Open
*/
bool MainWindow::urlOpen(const QUrl& url, bool newTab){
    downloadURL = url;
    downloadNewTab = newTab;

    QHttp* http = new QHttp;
    connect( http, SIGNAL(dataReadProgress(int, int)), SLOT(on_openUrl_dataReadProgress(int, int)) );
    connect( http, SIGNAL(requestFinished(int, bool)), SLOT(on_openUrl_requestFinished(int, bool)) );

    QHttp::ConnectionMode mode = url.scheme().toLower() == "https" ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;
    http->setHost(url.host(), mode, url.port() == -1 ? 0 : url.port());
    downloadID = http->get( url.encodedPath(), new QBuffer(http) );

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
    SgfDocument* sgfDoc = qobject_cast<SgfDocument*>(doc);

    // initial path for save dialog
    QString initialPath;
    if (doc->getFileName().isEmpty() == false)
        initialPath = doc->getFileName();
    else{
        if (sgfDoc){
            QSettings settings;
            QString in = settings.value("saveFileName", SAVE_FILE_NAME).toString();
            if (replaceSgfProperty(sgfDoc->gameList[0], in, initialPath) == 0)
                initialPath = doc->getDocName();
        }
        else
            initialPath = doc->getDocName();

        QFileInfo fi(initialPath);
        if (fi.suffix().isEmpty())
            initialPath += ".sgf";
    }

    // get save filename
    QString fname;
    QTextCodec* codec = doc->getCodec();
    if (getSaveFileName(initialPath, fname, codec) == false)
        return false;

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
    addRecentFile(fname);

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
  add file to recent file list
*/
void MainWindow::addRecentFile(const QString& fname){
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fname);
    files.push_front(fname);
    while (files.size() > maxRecentFiles)
        files.removeLast();
    settings.setValue("recentFileList", files);

    updateRecentFileActions();
}

/**
  update recent files menu
*/
void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), maxRecentFiles);

    qDeleteAll(ui->menuRecentFiles->actions());
    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = QFileInfo(files[i]).fileName();
        QAction* act = new QAction(text, this);
        ui->menuRecentFiles->addAction(act);
        connect(act, SIGNAL(triggered()), SLOT(on_actionOpenRecentFile_triggered()));
    }
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
    view.branchType   = gameMode;

    // create collection model
    QStandardItemModel* model = new QStandardItemModel;
    view.collectionModel = model;

    if (board == NULL){
        board = new BoardWidget(doc, this);
        setPreferences(board);
        view.boardWidget  = board;

        connect(board, SIGNAL(currentGameChanged(Go::NodePtr)), SLOT(on_boardWidget_currentGameChanged(Go::NodePtr)));
        connect(board, SIGNAL(currentNodeChanged(Go::NodePtr)), SLOT(on_boardWidget_currentNodeChanged(Go::NodePtr)));
        connect(board, SIGNAL(scoreUpdated(int, int, int, int, int, int, int, int, int)), SLOT(on_boardWidget_scoreUpdated(int, int, int, int, int, int, int, int, int)));

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
    connect(doc, SIGNAL(nodeModified(Go::NodePtr, bool)), SLOT(on_sgfdocument_nodeModified(Go::NodePtr, bool)));
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
    ViewData& data = docManager[doc];
    data.branchWidget->clear();
    data.nodeToTreeItem.clear();

    QTreeWidgetItem* dummy = new QTreeWidgetItem(QStringList(""));
    createBranchWidget( data, data.boardWidget, dummy, dummy, dummy, Go::NodePtr(), data.boardWidget->getCurrentGame() );

    int count = dummy->childCount();
    for (int i=0; i<count; ++i){
        QTreeWidgetItem* item = dummy->child(0);
        dummy->removeChild(item);
        data.branchWidget->invisibleRootItem()->addChild(item);
    }
    delete dummy;

    if (data.branchWidget->invisibleRootItem()->childCount() > 0)
        data.branchWidget->setCurrentItem( data.branchWidget->invisibleRootItem()->child(0) );

    data.branchWidget->setHeaderHidden(true);
    data.branchWidget->setIndentation(17);
}

/**
  create branch tree widget
*/
void MainWindow::createBranchWidget(BoardWidget* board, Go::NodePtr node){
    ViewData& data = docManager[board->document()];
    QTreeWidgetItem* parent2 = data.nodeToTreeItem[node];
    QTreeWidgetItem* parent1 = parent2->parent() ? parent2->parent() : data.branchWidget->invisibleRootItem();

    foreach(Go::NodePtr childNode, node->childNodes)
        createBranchWidget(data, board, data.branchWidget->invisibleRootItem(), parent1, parent2, node, childNode);
}

/**
  create branch tree widget
*/
void MainWindow::createBranchWidget(ViewData& data, BoardWidget* board, QTreeWidgetItem* root, QTreeWidgetItem* parent1, QTreeWidgetItem* parent2, Go::NodePtr parentNode, Go::NodePtr node){
    // create tree item widget
    QTreeWidgetItem* item = createBranchItem(data, board, node);
    QTreeWidgetItem* currentParent = NULL;
    if (item->parent())
        currentParent = item->parent();
    else if (root->indexOfChild(item) >= 0)
        currentParent = root;

    Go::NodePtr parent1Node = parent1->data(0, Qt::UserRole).value<Go::NodePtr>();
    QTreeWidgetItem* parentWidget = NULL;
    int index = parentNode ? parentNode->childNodes.indexOf(node) : 0;
    if (index < 0)
        index = 0;

    //
    bool isBranch =
        (parentNode && data.branchType == branchMode && parentNode->childNodes.size() > 1) ||
        (parentNode && data.branchType == gameMode && parentNode->childNodes.indexOf(node) > 0) ||
        (parent1Node && parent1Node->childNodes.size() > 1);

    if (isBranch == false){
        parentWidget = parent1;
        int parentIndex = parentWidget->indexOfChild(parent2);
        if (parentIndex >= 0)
            index = parentIndex + 1;
    }
    else if (parentNode->childNodes.empty() == false){
        parentWidget = parent2;
        index = parentNode->childNodes.indexOf(node);
        if (data.branchType == gameMode && parent1->childCount() > 0 && (parent1Node == NULL || parent1Node->childNodes.size() == 1))
            --index;
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
        createBranchWidget(data, board, root, parentWidget, item, node, childNode);
}

/**
  create branch tree item from node
*/
QTreeWidgetItem* MainWindow::createBranchItem(ViewData& data, BoardWidget* board, Go::NodePtr node){
    // icons
    static QIcon blackIcon(":/res/black_128.png");
    static QIcon whiteIcon(":/res/white_128.png");
    static QIcon greenIcon(":/res/green_64.png");

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

    // node type
    if (node->isStone() && node->isPass())
        str = tr("Pass");
    else if (node->isStone())
        str = board->document()->positionString(node);

    // node name
    if (node->name.isEmpty() == false)
        str += " " + node->name;

    // move number
    if (node->moveNumber > 0)
        str += " (" + QString::number(node->moveNumber) + ")";

    // comment
    if (node->comment.isEmpty() == false)
        str += " " + tr("Comment");

    // node annotation
    if (node->nodeAnnotation == Go::Node::even)
        str += " " + tr("[Even]");
    else if(node->nodeAnnotation == Go::Node::goodForBlack)
        str += " " + tr("[Good for Black]");
    else if(node->nodeAnnotation == Go::Node::veryGoodForBlack)
        str += " " + tr("[Very Good for Black]");
    else if(node->nodeAnnotation == Go::Node::goodForWhite)
        str += " " + tr("[Good for White]");
    else if(node->nodeAnnotation == Go::Node::veryGoodForWhite)
        str += " " + tr("[Very Good for White]");
    else if(node->nodeAnnotation == Go::Node::unclear)
        str += " " + tr("[Unclear]");

    // move annotation
    if (node->moveAnnotation == Go::Node::goodMove)
        str += " " + tr("[Good Move]");
    else if(node->moveAnnotation == Go::Node::veryGoodMove)
        str += " " + tr("[Very Good Move]");
    else if(node->moveAnnotation == Go::Node::badMove)
        str += " " + tr("[Bad Move]");
    else if(node->moveAnnotation == Go::Node::veryBadMove)
        str += " " + tr("[Very Bad Move]");
    else if(node->moveAnnotation == Go::Node::doubtfulMove)
        str += " " + tr("[Doubtful Move]");
    else if(node->moveAnnotation == Go::Node::interestingMove)
        str += " " + tr("[Interesting Move]");

    // annotation
    if (node->annotation == Go::Node::hotspot)
        str += " " + tr("[Hotspot]");

    // add stone
    if (node->whiteStones.empty() == false)
        str += " " + tr("Add White");
    if (node->blackStones.empty() == false)
        str += " " + tr("Add Black");
    if (node->emptyStones.empty() == false)
        str += " " + tr("Add Empty");

    // mark
    if (node->marks.empty() == false)
        str += " " + tr("Mark");
    if (node->whiteTerritories.empty() == false)
        str += " " + tr("White Territories");
    if (node->blackTerritories.empty() == false)
        str += " " + tr("Black Territories");
    if (node->dims.empty() == false)
        str += " " + tr("Dim");
    if (node->lines.empty() == false)
        str += " " + tr("Line");

    // game information
    if (str.isEmpty() == false && str[0].isSpace())
        str.remove(0, 1);
    if (node->gameInformation){
        if (str.isEmpty())
            str = tr("Game Information");
        else
            str.insert(0, tr("Info") + " ");
    }

    if (str.isEmpty())
        str = tr("Other");

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
  update menu
*/
void MainWindow::updateMenu(bool updateAll){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr game = board->getCurrentGame();
    Go::NodePtr node = board->getCurrentNode();
    ViewData& data = docManager[board->document()];

    // Edit -> Annotation
    ui->actionHotspot->setChecked(node->annotation == Go::Node::hotspot);

    // Edit -> Move Annotation
    if (node->moveAnnotation == Go::Node::noMoveAnnotation)
        ui->actionNoMoveAnnotation->setChecked(true);
    else if (node->moveAnnotation == Go::Node::goodMove)
        ui->actionGoodMove->setChecked(true);
    else if (node->moveAnnotation == Go::Node::veryGoodMove)
        ui->actionVeryGoodMove->setChecked(true);
    else if (node->moveAnnotation == Go::Node::badMove)
        ui->actionBadMove->setChecked(true);
    else if (node->moveAnnotation == Go::Node::veryBadMove)
        ui->actionVeryBadMove->setChecked(true);
    else if (node->moveAnnotation == Go::Node::doubtfulMove)
        ui->actionDoubtfulMove->setChecked(true);
    else if (node->moveAnnotation == Go::Node::interestingMove)
        ui->actionInterestingMove->setChecked(true);

    // Edit -> Node Annotation
    if (node->nodeAnnotation == Go::Node::noNodeAnnotation)
        ui->actionNoNodeAnnotation->setChecked(true);
    else if (node->nodeAnnotation == Go::Node::even)
        ui->actionEven->setChecked(true);
    else if (node->nodeAnnotation == Go::Node::goodForBlack)
        ui->actionGoodForBlack->setChecked(true);
    else if (node->nodeAnnotation == Go::Node::veryGoodForBlack)
        ui->actionVeryGoodForBlack->setChecked(true);
    else if (node->nodeAnnotation == Go::Node::goodForWhite)
        ui->actionGoodForWhite->setChecked(true);
    else if (node->nodeAnnotation == Go::Node::veryGoodForWhite)
        ui->actionVeryGoodForWhite->setChecked(true);
    else if (node->nodeAnnotation == Go::Node::unclear)
        ui->actionUnclear->setChecked(true);

    // Navigation -> Jump To Clicked
    ui->actionJumpToClicked->setChecked(board->isJumpToClicked());

    // View -> Show Move Number
    ui->menuMoveNumber->menuAction()->setChecked(board->getShowMoveNumber());
    ui->actionShowMoveNumber->setChecked(board->getShowMoveNumber());

    // View -> Reset Move Number In Branch
    ui->actionResetMoveNumberInBranch->setChecked(board->getResetMoveNumberMode() != BoardWidget::ResetMoveNumber::noReset);

    // View -> Move Number
    switch(board->getShowMoveNumberCount()){
        case -1:
            ui->actionAllMoves->setChecked(true);
            break;
        case 0:
            ui->actionNoMoveNumber->setChecked(true);
            break;
        case 1:
            ui->actionLast1Move->setChecked(true);
            break;
        case 2:
            ui->actionLast2Moves->setChecked(true);
            break;
        case 5:
            ui->actionLast5Moves->setChecked(true);
            break;
        case 10:
            ui->actionLast10Moves->setChecked(true);
            break;
        case 20:
            ui->actionLast20Moves->setChecked(true);
            break;
        case 50:
            ui->actionLast50Moves->setChecked(true);
            break;
    }

    if (updateAll == false)
        return;

    // File -> Reload
    QAction* encodingAction = encoding.key( board->document()->getCodec() );
    if (encodingAction)
        encodingAction->setChecked(true);
    ui->menuReload->menuAction()->setEnabled( board->document()->getFileName().isEmpty() == false || board->document()->getUrl().isEmpty() == false );

    // Edit -> Stones & Markers
    switch( board->getEditMode() ){
        case BoardWidget::EditMode::alternateMove:
            ui->actionAlternateMove->trigger();
            break;
        case BoardWidget::EditMode::addBlack:
            ui->actionAddBlackStone->trigger();
            break;
        case BoardWidget::EditMode::addWhite:
            ui->actionAddWhiteStone->trigger();
            break;
        case BoardWidget::EditMode::addEmpty:
            ui->actionAddEmpty->trigger();
            break;
        case BoardWidget::EditMode::addLabel:
            ui->actionAddLabel->trigger();
            break;
        case BoardWidget::EditMode::addLabelManually:
            ui->actionAddLabelManually->trigger();
            break;
        case BoardWidget::EditMode::addCircle:
            ui->actionAddCircle->trigger();
            break;
        case BoardWidget::EditMode::addCross:
            ui->actionAddCross->trigger();
            break;
        case BoardWidget::EditMode::addTriangle:
            ui->actionAddTriangle->trigger();
            break;
        case BoardWidget::EditMode::addSquare:
            ui->actionAddSquare->trigger();
            break;
        case BoardWidget::EditMode::removeMarker:
            ui->actionDeleteMarker->trigger();
            break;
    }

    // Edit -> White First
    ui->actionWhiteFirst->setChecked(game->nextColor == Go::white);

    // View -> Branch Mode
    ui->actionBranchMode->setChecked(data.branchType == branchMode);

    // View -> Show Coordinate
    ui->actionShowCoordinate->setChecked( board->getShowCoordinate() );

    // View -> Show Coordinate WIth I
    ui->actionShowCoordinateWithI->setChecked( board->getShowCoordinateWithI() );

    // View -> Show Variations
    if (board->getShowVariations() == 0)
        ui->actionShowChildren->setChecked(true);
    else if (board->getShowVariations() == 1)
        ui->actionShowSiblings->setChecked(true);
    else
        ui->actionNoMarkup->setChecked(true);

    // View -> Show Marker
    ui->actionShowMarker->setChecked( board->getShowMarker() );

    // View -> Rotate Clockwise
    ui->actionRotateClockwise->setChecked( board->getRotate() != 0 );

    // View -> Flip Horizontally
    ui->actionFlipHorizontally->setChecked( board->getFlipHorizntally() );

    // View -> Flip Vertically
    ui->actionFlipVertically->setChecked( board->getFlipVertically() );

    // Tools -> Score Mode
    ui->actionCountTerritory->setChecked( board->getScoreMode() == BoardWidget::ScoreMode::final );

    // Tools -> Tutor Mode
    ui->actionAutomaticReplay->setChecked( board->getTutorMode() == BoardWidget::TutorMode::replay );
    ui->actionTutorBothSides->setChecked( board->getTutorMode() == BoardWidget::TutorMode::tutorBothSides );
    ui->actionTutorOneSide->setChecked( board->getTutorMode() == BoardWidget::TutorMode::tutorOneSide );

    // Tools -> Play Sound
    ui->actionPlaySound->setChecked( board->isPlaySound() );

    // Tutor Mode
    setTutorMode(board, board->getTutorMode());

    // Score Mode
    setScoreMode(board, board->getScoreMode());
}

/**
  set score mdoe
*/
void MainWindow::setScoreMode(BoardWidget* board, int mode){
    QList<QAction*> allActions;
    allActions << ui->menuFile->actions()
               << ui->menuEdit->actions()
               << ui->menuNavigation->actions()
               << ui->menuView->actions()
               << ui->menuTools->actions();

    static QAction* actions[] = {
        ui->actionNew,
        ui->actionOpen,
        ui->actionOpenURL,
        ui->actionCloseTab,
        ui->actionCloseAllTabs,
        ui->actionSave,
        ui->actionSaveAs,
        ui->actionExportAsciiToClipboard,
        ui->menuRecentFiles->menuAction(),
        ui->actionExit,
        ui->actionCopySgfToClipboard,
        ui->actionCopyCurrentBranchToClipboard,
        ui->actionPasteSgfToNewTab,
        ui->menuMoveNumber->menuAction(),
        ui->actionShowCoordinate,
        ui->actionShowCoordinateWithI,
        ui->actionRotateClockwise,
        ui->actionFlipHorizontally,
        ui->actionFlipVertically,
        ui->actionResetBoard,
        ui->actionPlaySound,
        ui->actionOptions,
        ui->actionCountTerritory,
    };
    static int N = sizeof(actions) / sizeof(actions[0]);

    ViewData& data = docManager[board->document()];
    if (mode != BoardWidget::ScoreMode::noScore){
        ui->collectionDockWidget->setEnabled(false);
        data.branchWidget->setEnabled(false);
    }
    else{
        ui->collectionDockWidget->setEnabled(true);
        data.branchWidget->setEnabled(true);
    }

    foreach(QAction* act, allActions){
        if (mode != BoardWidget::ScoreMode::noScore){
            QAction** a = qFind(actions, actions+N, act);
            bool enable = a != actions + N;
            if (act->isEnabled() != enable)
                act->setEnabled(enable);
        }
        else{
            if (act->isEnabled() != true)
                act->setEnabled(true);
        }
    }

    if (mode == BoardWidget::ScoreMode::noScore)
        ui->menuReload->menuAction()->setEnabled( board->document()->getFileName().isEmpty() == false || board->document()->getUrl().isEmpty() == false );
    else
        ui->menuReload->menuAction()->setEnabled(false);

    undoAction->setEnabled( undoGroup.canUndo() );
    redoAction->setEnabled( undoGroup.canRedo() );
}

/**
  set tutor mdoe
*/
void MainWindow::setTutorMode(BoardWidget* board, int mode){
    QList<QAction*> allActions;
    allActions << ui->menuFile->actions()
               << ui->menuEdit->actions()
               << ui->menuNavigation->actions()
               << ui->menuView->actions()
               << ui->menuTools->actions();

    static QAction* actions[] = {
        ui->actionNew,
        ui->actionOpen,
        ui->actionOpenURL,
        ui->actionCloseTab,
        ui->actionCloseAllTabs,
        ui->actionSave,
        ui->actionSaveAs,
        ui->actionExportAsciiToClipboard,
        ui->menuRecentFiles->menuAction(),
        ui->actionExit,
        ui->actionCopySgfToClipboard,
        ui->actionCopyCurrentBranchToClipboard,
        ui->actionPasteSgfToNewTab,
        ui->menuMoveNumber->menuAction(),
        ui->actionShowCoordinate,
        ui->actionShowCoordinateWithI,
        ui->actionRotateClockwise,
        ui->actionFlipHorizontally,
        ui->actionFlipVertically,
        ui->actionResetBoard,
        ui->actionPlaySound,
        ui->actionOptions,
        ui->actionAutomaticReplay,
        ui->actionTutorBothSides,
        ui->actionTutorOneSide,
    };
    static int N = sizeof(actions) / sizeof(actions[0]);

    ViewData& data = docManager[board->document()];
    if (mode != BoardWidget::TutorMode::noTutor){
        undoGroup.setActiveStack(NULL);
        ui->collectionDockWidget->setEnabled(false);
        if (mode != BoardWidget::TutorMode::replay)
            data.branchWidget->hide();
    }
    else{
        undoGroup.setActiveStack(board->document()->getUndoStack());
        ui->collectionDockWidget->setEnabled(true);
        data.branchWidget->show();
    }

    foreach(QAction* act, allActions){
        if (mode != BoardWidget::TutorMode::noTutor){
            QAction** a = qFind(actions, actions+N, act);
            bool enable = a != actions + N;
            if (act->isEnabled() != enable)
                act->setEnabled(enable);
        }
        else{
            if (act->isEnabled() != true)
                act->setEnabled(true);
        }
    }

    if (mode == BoardWidget::TutorMode::noTutor)
        ui->menuReload->menuAction()->setEnabled( board->document()->getFileName().isEmpty() == false || board->document()->getUrl().isEmpty() == false );
    else
        ui->menuReload->menuAction()->setEnabled(false);

    undoAction->setEnabled( undoGroup.canUndo() );
    redoAction->setEnabled( undoGroup.canRedo() );
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
    combo->insertSeparator(combo->count());
    actions.push_back(NULL);
    foreach(QAction* act, ui->menuReload->actions()){
        if (act->isSeparator() == false){
            combo->addItem(act->text());
            if (codec && encoding[act]->name() == codec->name())
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
    codec = combo->currentIndex() >= 0 ? encoding[actions[combo->currentIndex()]] : mugoApp()->defaultCodec();

    return true;
}

/**
  get save file name
*/
bool MainWindow::getSaveFileName(const QString& initialPath, QString& fname, QTextCodec*& codec){
    QString filter = "Smart Game Format (*.sgf);;All Files (*.*)";
/*
    QString fname = QFileDialog::getSaveFileName(this, QString(), QString(doc->getDocName()), filter, NULL);
    if (fname.isEmpty())
        return false;
*/

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
            if (encoding[act]->name() == codec->name())
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
    codec = encoding[actions[combo->currentIndex()]];

    return true;
}

/**
  slot
  File -> New
*/
void MainWindow::on_actionNew_triggered()
{
    NewDocumentDialog dlg;
    if (dlg.exec() != QDialog::Accepted)
        return;

    fileNew(dlg.codec, dlg.xsize, dlg.ysize, dlg.komi);
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
        fileOpen(fname, mugoApp()->defaultCodec(), true);
    else
        fileOpen(fname, codec, false);
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
        tmpDoc = openDocument(fname, mugoApp()->defaultCodec(), true);
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
  File -> Recent Files
*/
void MainWindow::on_actionOpenRecentFile_triggered(){
    QAction* act = qobject_cast<QAction*>(sender());
    int n = ui->menuRecentFiles->actions().indexOf(act);
    if (n < 0)
        return;

    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    if (files.size() <= n)
        return;

    fileOpen(files[n], mugoApp()->defaultCodec(), true);
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
  Edit -> Delete After Current
*/
void MainWindow::on_actionDeleteAfterCurrent_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr currentNode = board->getCurrentNode();
    if (currentNode == board->getCurrentNodeList().front())
        return;

    SgfDocument* doc = board->document();
    doc->getUndoStack()->push( new DeleteNodeCommand(doc, currentNode, true) );
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

    Go::NodePtr currentNode = board->getCurrentNode();
    if (currentNode == board->getCurrentNodeList().front())
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

    board->addItem(currentNode, childNode);
    board->setCurrentNode(childNode);
}

/**
  Slot
  Edit -> Alternate Move
*/
void MainWindow::on_actionAlternateMove_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->setEditMode(BoardWidget::EditMode::alternateMove);

    ui->menuStonesAndMarkers->menuAction()->setChecked(false);
}

/**
  Slot
  Edit -> Stones And Markers
*/
void MainWindow::on_actionStonesAndMarkers_triggered(){
    QAction* act = ui->menuStonesAndMarkers->menuAction()->data().value<QAction*>();
    if (act)
        act->trigger();
}

/**
  Slot
  Edit -> Add Black Stone
*/
void MainWindow::on_actionAddBlackStone_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->setEditMode(BoardWidget::EditMode::addBlack);

    ui->menuStonesAndMarkers->menuAction()->setChecked(true);
    ui->menuStonesAndMarkers->setIcon( ui->actionAddBlackStone->icon() );
    ui->menuStonesAndMarkers->menuAction()->setData( QVariant::fromValue(ui->actionAddBlackStone) );
}

/**
  Slot
  Edit -> Add White Stone
*/
void MainWindow::on_actionAddWhiteStone_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->setEditMode(BoardWidget::EditMode::addWhite);

    ui->menuStonesAndMarkers->menuAction()->setChecked(true);
    ui->menuStonesAndMarkers->setIcon( ui->actionAddWhiteStone->icon() );
    ui->menuStonesAndMarkers->menuAction()->setData( QVariant::fromValue(ui->actionAddWhiteStone) );
}

/**
  Slot
  Edit -> Add Empty
*/
void MainWindow::on_actionAddEmpty_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->setEditMode(BoardWidget::EditMode::addEmpty);

    ui->menuStonesAndMarkers->menuAction()->setChecked(true);
    ui->menuStonesAndMarkers->setIcon( ui->actionAddEmpty->icon() );
    ui->menuStonesAndMarkers->menuAction()->setData( QVariant::fromValue(ui->actionAddEmpty) );
}

/**
  Slot
  Edit -> Add Label
*/
void MainWindow::on_actionAddLabel_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->setEditMode(BoardWidget::EditMode::addLabel);

    ui->menuStonesAndMarkers->menuAction()->setChecked(true);
    ui->menuStonesAndMarkers->setIcon( ui->actionAddLabel->icon() );
    ui->menuStonesAndMarkers->menuAction()->setData( QVariant::fromValue(ui->actionAddLabel) );
}

/**
  Slot
  Edit -> Add Label Manually
*/
void MainWindow::on_actionAddLabelManually_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->setEditMode(BoardWidget::EditMode::addLabelManually);

    ui->menuStonesAndMarkers->menuAction()->setChecked(true);
    ui->menuStonesAndMarkers->setIcon( ui->actionAddLabelManually->icon() );
    ui->menuStonesAndMarkers->menuAction()->setData( QVariant::fromValue(ui->actionAddLabelManually) );
}

/**
  Slot
  Edit -> Add Circle
*/
void MainWindow::on_actionAddCircle_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->setEditMode(BoardWidget::EditMode::addCircle);

    ui->menuStonesAndMarkers->menuAction()->setChecked(true);
    ui->menuStonesAndMarkers->setIcon( ui->actionAddCircle->icon() );
    ui->menuStonesAndMarkers->menuAction()->setData( QVariant::fromValue(ui->actionAddCircle) );
}

/**
  Slot
  Edit -> Add Cross
*/
void MainWindow::on_actionAddCross_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->setEditMode(BoardWidget::EditMode::addCross);

    ui->menuStonesAndMarkers->menuAction()->setChecked(true);
    ui->menuStonesAndMarkers->setIcon( ui->actionAddCross->icon() );
    ui->menuStonesAndMarkers->menuAction()->setData( QVariant::fromValue(ui->actionAddCross) );
}

/**
  Slot
  Edit -> Add Triangle
*/
void MainWindow::on_actionAddTriangle_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->setEditMode(BoardWidget::EditMode::addTriangle);

    ui->menuStonesAndMarkers->menuAction()->setChecked(true);
    ui->menuStonesAndMarkers->setIcon( ui->actionAddTriangle->icon() );
    ui->menuStonesAndMarkers->menuAction()->setData( QVariant::fromValue(ui->actionAddTriangle) );
}

/**
  Slot
  Edit -> Add Square
*/
void MainWindow::on_actionAddSquare_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->setEditMode(BoardWidget::EditMode::addSquare);

    ui->menuStonesAndMarkers->menuAction()->setChecked(true);
    ui->menuStonesAndMarkers->setIcon( ui->actionAddSquare->icon() );
    ui->menuStonesAndMarkers->menuAction()->setData( QVariant::fromValue(ui->actionAddSquare) );
}

/**
  Slot
  Edit -> Delete Marker
*/
void MainWindow::on_actionDeleteMarker_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->setEditMode(BoardWidget::EditMode::removeMarker);

    ui->menuStonesAndMarkers->menuAction()->setChecked(true);
    ui->menuStonesAndMarkers->setIcon( ui->actionDeleteMarker->icon() );
    ui->menuStonesAndMarkers->menuAction()->setData( QVariant::fromValue(ui->actionDeleteMarker) );
}

/**
  Slot
  Edit -> Edit Node Name
*/
void MainWindow::on_actionEditNodeName_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr node = board->getCurrentNode();

    QInputDialog dlg(this);
    dlg.setLabelText( tr("Input node name") );
    dlg.setTextValue(node->name);
    if (dlg.exec() != QDialog::Accepted)
        return;

    board->document()->getUndoStack()->push( new SetNodeNameCommand(board->document(), node, dlg.textValue()) );
}

/**
  Slot
  Edit -> Move Number -> Set Move Number
*/
void MainWindow::on_actionSetMoveNumber_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr node = board->getCurrentNode();
    if (node->isStone() == false)
        return;

    QInputDialog dlg(this);
    dlg.setInputMode(QInputDialog::IntInput);
    dlg.setLabelText( tr("Input move number") );
    dlg.setIntMinimum(1);
    dlg.setIntMaximum(1000);

    if (node->moveNumber > 0)
        dlg.setIntValue(node->moveNumber);

    if (dlg.exec() != QDialog::Accepted)
        return;

    board->document()->getUndoStack()->push( new SetMoveNumberCommand(board->document(), node, dlg.intValue()) );
}

/**
  Slot
  Edit -> Move Number -> Unset Move Number
*/
void MainWindow::on_actionUnsetMoveNumber_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr node = board->getCurrentNode();
    board->document()->getUndoStack()->push( new UnsetMoveNumberCommand(board->document(), node) );
}

/**
  Slot
  Edit -> White First
*/
void MainWindow::on_actionWhiteFirst_triggered(bool checked){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr node = board->getCurrentGame();
    node->nextColor = checked ? Go::white : Go::black;
}

/**
  Slot
  Edit -> Node Annotation -> No Annotation
*/
void MainWindow::on_actionNoNodeAnnotation_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetNodeAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::noNodeAnnotation) );
}

/**
  Slot
  Edit -> Node Annotation -> Even
*/
void MainWindow::on_actionEven_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetNodeAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::even) );
}

/**
  Slot
  Edit -> Node Annotation -> Good for Black
*/
void MainWindow::on_actionGoodForBlack_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetNodeAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::goodForBlack) );
}

/**
  Slot
  Edit -> Node Annotation -> Very Good for Black
*/
void MainWindow::on_actionVeryGoodForBlack_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetNodeAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::veryGoodForBlack) );
}

/**
  Slot
  Edit -> Node Annotation -> Good for White
*/
void MainWindow::on_actionGoodForWhite_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetNodeAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::goodForWhite) );
}

/**
  Slot
  Edit -> Node Annotation -> Very Good for White
*/
void MainWindow::on_actionVeryGoodForWhite_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetNodeAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::veryGoodForWhite) );
}

/**
  Slot
  Edit -> Node Annotation -> Unclear
*/
void MainWindow::on_actionUnclear_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetNodeAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::unclear) );
}

/**
  Slot
  Edit -> Move Annotation -> No Annotation
*/
void MainWindow::on_actionNoMoveAnnotation_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetMoveAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::noMoveAnnotation) );
}

/**
  Slot
  Edit -> Move Annotation -> Good Move
*/
void MainWindow::on_actionGoodMove_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetMoveAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::goodMove) );
}

/**
  Slot
  Edit -> Move Annotation -> Very Good Move
*/
void MainWindow::on_actionVeryGoodMove_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetMoveAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::veryGoodMove) );
}

/**
  Slot
  Edit -> Move Annotation -> Bad Move
*/
void MainWindow::on_actionBadMove_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetMoveAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::badMove) );
}

/**
  Slot
  Edit -> Move Annotation -> Very Bad Move
*/
void MainWindow::on_actionVeryBadMove_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetMoveAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::veryBadMove) );
}

/**
  Slot
  Edit -> Move Annotation -> Doubtful Move
*/
void MainWindow::on_actionDoubtfulMove_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetMoveAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::doubtfulMove) );
}

/**
  Slot
  Edit -> Move Annotation -> Interesting Move
*/
void MainWindow::on_actionInterestingMove_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetMoveAnnotationCommand(board->document(), board->getCurrentNode(), Go::Node::interestingMove) );
}

/**
  Slot
  Edit -> Annotation -> Hotspot
*/
void MainWindow::on_actionHotspot_triggered(bool checked){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new SetAnnotationCommand(board->document(), board->getCurrentNode(), checked ? Go::Node::hotspot : Go::Node::noAnnotation) );
}

/**
  Slot
  Edit -> Move First
*/
void MainWindow::on_actionMoveFirst_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    const Go::NodeList& nodeList = board->getCurrentNodeList();
    board->setCurrentNode(nodeList.front());
}

/**
  Slot
  Edit -> Rotate SGF Clockwise
*/
void MainWindow::on_actionRotateSGFClockwise_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new RotateSgfClockwiseCommand(board->document(), board->getCurrentGame()) );
}

/**
  Slot
  Edit -> Flip SGF Horizontally
*/
void MainWindow::on_actionFlipSgfHorizontally_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new FlipSgfHorizontallyCommand(board->document(), board->getCurrentGame()) );
}

/**
  Slot
  Edit -> Flip SGF Vertically
*/
void MainWindow::on_actionFlipSgfVertically_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->document()->getUndoStack()->push( new FlipSgfVerticallyCommand(board->document(), board->getCurrentGame()) );
}

/**
  Slot
  Navigation -> Fast Rewind
*/
void MainWindow::on_actionFastRewind_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->back(stepsOfFastMove);
}

/**
  Slot
  Navigation -> Move Next
*/
void MainWindow::on_actionMovePrevious_triggered()
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
void MainWindow::on_actionMoveLast_triggered()
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
void MainWindow::on_actionFastForward_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->forward(stepsOfFastMove);
}

/**
  Slot
  Navigation -> Move Next
*/
void MainWindow::on_actionMoveNext_triggered()
{
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    board->forward();
}

/**
  Slot
  Navigation -> Back to Parent
*/
void MainWindow::on_actionBackToParent_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr node = board->getCurrentNode();
    while (node->parent()){
        node = node->parent();
        if (node->childNodes.size() > 1)
            break;
    }
    board->setCurrentNode(node);
}

/**
  Slot
  Navigation -> Previous Sibling
*/
void MainWindow::on_actionPreviousSibling_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr node = board->getCurrentNode();
    Go::NodePtr prev = node->previousSibling();
    if (prev)
        board->setCurrentNode(prev);
}

/**
  Slot
  Navigation -> Next Sibling
*/
void MainWindow::on_actionNextSibling_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    Go::NodePtr node = board->getCurrentNode();
    Go::NodePtr next = node->nextSibling();
    if (next)
        board->setCurrentNode(next);
}

/**
  Slot
  Navigation -> Jump to Move Number
*/
void MainWindow::on_actionJumpToMoveNumber_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    const Go::NodeList& nodeList = board->getCurrentNodeList();
    int currentIndex = nodeList.indexOf(board->getCurrentNode());

    bool ok;
    int number = QInputDialog::getInt(this, QString(), tr("Input Move Number"), currentIndex, 0, nodeList.size()-1, 1, &ok);

    if(ok == false)
        return;

    board->setCurrentNode(nodeList[number]);
}

/**
  Slot
  Navigation -> Jump to Clicked
*/
void MainWindow::on_actionJumpToClicked_triggered(bool checked){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setJumpToClicked(checked);
}

/**
  Slot
  View -> Move Number -> Move Number
*/
void MainWindow::on_actionMoveNumber_triggered(){
    ui->actionShowMoveNumber->trigger();
}

/**
  Slot
  View -> Move Number -> Show Move Number
*/
void MainWindow::on_actionShowMoveNumber_triggered(bool checked){
    ui->menuMoveNumber->menuAction()->setChecked(checked);

    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowMoveNumber(checked);

    QSettings settings;
    settings.setValue("marker/showMoveNumber", checked);
}

/**
  Slot
  View -> Move Number -> Reset Move Number in Branch
*/
void MainWindow::on_actionResetMoveNumberInBranch_triggered(bool checked){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    SgfDocument* doc = board->document();
    ViewData& data = docManager[doc];

    if (checked)
        board->setResetMoveNumberMode( data.branchType == gameMode ? BoardWidget::ResetMoveNumber::branch : BoardWidget::ResetMoveNumber::allBranch );
    else
        board->setResetMoveNumberMode( BoardWidget::ResetMoveNumber::noReset );

    updateStatusBar();

    QSettings settings;
    settings.setValue("marker/resetMoveNumberInBranch", checked);
}

/**
  Slot
  View -> Move Number -> No Move Number
*/
void MainWindow::on_actionNoMoveNumber_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowMoveNumberCount(0);
}

/**
  Slot
  View -> Move Number -> Last 1 Move
*/
void MainWindow::on_actionLast1Move_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowMoveNumberCount(1);
}

/**
  Slot
  View -> Move Number -> Last 2 Moves
*/
void MainWindow::on_actionLast2Moves_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowMoveNumberCount(2);
}

/**
  Slot
  View -> Move Number -> Last 5 Moves
*/
void MainWindow::on_actionLast5Moves_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowMoveNumberCount(5);
}

/**
  Slot
  View -> Move Number -> Last 10 Moves
*/
void MainWindow::on_actionLast10Moves_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowMoveNumberCount(10);
}

/**
  Slot
  View -> Move Number -> Last 20 Moves
*/
void MainWindow::on_actionLast20Moves_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowMoveNumberCount(20);
}

/**
  Slot
  View -> Move Number -> Last 50 Moves
*/
void MainWindow::on_actionLast50Moves_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowMoveNumberCount(50);
}

/**
  Slot
  View -> Move Number -> All Moves
*/
void MainWindow::on_actionAllMoves_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowMoveNumberCount(-1);
}

/**
  Slot
  View -> branch mode
*/
void MainWindow::on_actionBranchMode_triggered(bool checked){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;
    SgfDocument* doc = board->document();

    ViewData& data = docManager[doc];
    data.branchType = checked ? branchMode : gameMode;

    Go::NodePtr currentNode = board->getCurrentNode();
    createBranchWidget(doc);
    board->setCurrentNode(currentNode);

    if (board->getResetMoveNumberMode() != BoardWidget::ResetMoveNumber::noReset)
        board->setResetMoveNumberMode( data.branchType == gameMode ? BoardWidget::ResetMoveNumber::branch : BoardWidget::ResetMoveNumber::allBranch );

    updateStatusBar();
}

/**
  Slot
  View -> Show Coordinate
*/
void MainWindow::on_actionShowCoordinate_triggered(bool checked){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowCoordinate(checked);
}

/**
  Slot
  View -> Show Coordinate With I
*/
void MainWindow::on_actionShowCoordinateWithI_triggered(bool checked){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowCoordinateWithI(checked);
}

/**
  Slot
  View -> Show Marker
*/
void MainWindow::on_actionShowMarker_triggered(bool checked){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowMarker(checked);
}

/**
  Slot
  View -> Show Variations -> No Markup
*/
void MainWindow::on_actionNoMarkup_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowVariations(2);
}

/**
  Slot
  View -> Show Variations -> Show Children
*/
void MainWindow::on_actionShowChildren_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowVariations(0);
}

/**
  Slot
  View -> Show Variations -> Show Siblings
*/
void MainWindow::on_actionShowSiblings_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setShowVariations(1);
}

/**
  Slot
  View -> Rotate Clockwise
*/
void MainWindow::on_actionRotateClockwise_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setRotate( (board->getRotate() + 1) % 4 ) ;
    ui->actionRotateClockwise->setChecked( board->getRotate() != 0 );
}

/**
  Slot
  View -> Flip Horizontally
*/
void MainWindow::on_actionFlipHorizontally_triggered(bool checked){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setFlipHorizontally(checked);
}

/**
  Slot
  View -> Flip Vertically
*/
void MainWindow::on_actionFlipVertically_triggered(bool checked){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setFlipVertically(checked);
}

/**
  Slot
  View -> Reset Board
*/
void MainWindow::on_actionResetBoard_triggered(){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    ui->actionFlipHorizontally->setChecked(false);
    ui->actionFlipVertically->setChecked(false);
    ui->actionRotateClockwise->setChecked(false);

    board->setFlipVertically(false);
    board->setFlipHorizontally(false);
    board->setRotate(0);
}

/**
  Slot
  Tools -> Count Territory
*/
void MainWindow::on_actionCountTerritory_triggered(bool checked){
    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    ViewData& data = docManager[board->document()];

    if (checked){
        data.countTerritoryDialog = new CountTerritoryDialog(this);
        connect(data.countTerritoryDialog, SIGNAL(finished(int)), this, SLOT(on_scoreDialog_finished(int)));
        data.countTerritoryDialog->setInformationNode( board->getGameInformation() );
        data.countTerritoryDialog->show();
        board->setScoreMode(BoardWidget::ScoreMode::final);
    }
    else{
        board->setScoreMode(BoardWidget::ScoreMode::noScore);
        delete data.countTerritoryDialog;
        data.countTerritoryDialog = NULL;
    }
    setScoreMode(board, board->getScoreMode());
}

/**
  Slot
  Tools -> Automatic Replay
*/
void MainWindow::on_actionAutomaticReplay_triggered(bool checked){
    if (checked){
        ui->actionTutorBothSides->setChecked(false);
        ui->actionTutorOneSide->setChecked(false);
    }

    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setTutorMode(checked ? BoardWidget::TutorMode::replay : BoardWidget::TutorMode::noTutor);
    setTutorMode(board, board->getTutorMode());
}

/**
  Slot
  Tools -> Tutor Both Sides
*/
void MainWindow::on_actionTutorBothSides_triggered(bool checked){
    if (checked){
        ui->actionAutomaticReplay->setChecked(false);
        ui->actionTutorOneSide->setChecked(false);
    }

    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setTutorMode(checked ? BoardWidget::TutorMode::tutorBothSides : BoardWidget::TutorMode::noTutor);
    setTutorMode(board, board->getTutorMode());
}

/**
  Slot
  Tools -> Tutor One Side
*/
void MainWindow::on_actionTutorOneSide_triggered(bool checked){
    if (checked){
        ui->actionAutomaticReplay->setChecked(false);
        ui->actionTutorBothSides->setChecked(false);
    }

    BoardWidget* board = currentBoard();
    if (board == NULL)
        return;

    board->setTutorMode(checked ? BoardWidget::TutorMode::tutorOneSide : BoardWidget::TutorMode::noTutor);
    setTutorMode(board, board->getTutorMode());
}

/**
  Slot
  Tools -> Play Sound
*/
void MainWindow::on_actionPlaySound_triggered(bool checked){
    QSettings settings;
    settings.setValue("sound/play", checked);

    for (int i=0; i<ui->boardTabWidget->count(); ++i){
        QWidget* widget = ui->boardTabWidget->widget(i);
        BoardWidget* board = qobject_cast<BoardWidget*>(widget);
        if (board)
            board->setPlaySound(checked);
    }
}

/**
  Slot
  Tools -> Options
*/
void MainWindow::on_actionOptions_triggered(){
    SetupDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    for (int i=0; i<ui->boardTabWidget->count(); ++i){
        QWidget* widget = ui->boardTabWidget->widget(i);
        BoardWidget* board = qobject_cast<BoardWidget*>(widget);
        if (board)
            setPreferences(board);
    }

    updateMenu(true);
}

/**
  Slot
  Tools -> Clear Settings
*/
void MainWindow::on_actionClearSettings_triggered(){
    if ( QMessageBox::question(this, QString(), tr("Are you sure you want to clear the configuration?"), QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
        return;

    QSettings settings;
    settings.clear();

    for (int i=0; i<ui->boardTabWidget->count(); ++i){
        QWidget* widget = ui->boardTabWidget->widget(i);
        BoardWidget* board = qobject_cast<BoardWidget*>(widget);
        if (board)
            setPreferences(board);
    }
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
void MainWindow::on_sgfdocument_nodeModified(Go::NodePtr node, bool /*needRecreateBoard*/){
    BoardWidget* board = currentBoard();

    // set comment
    if (board->getCurrentNode() == node){
        if (node->comment != ui->commentWidget->toPlainText())
            ui->commentWidget->setPlainText( node->comment );
    }

    // set tree text
    SgfDocument* doc = qobject_cast<SgfDocument*>(sender());
    QTreeWidgetItem* item = docManager[doc].nodeToTreeItem[node];
    if (item)
        item->setText( 0, getBranchItemText(board, node) );

    // update menu
    updateMenu();
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
        updateCaption(false);
        updateMenu();
        updateStatusBar();
    }

    // if tutor mode and new node is last node, exit tutor mode
    if (board->getTutorMode() != BoardWidget::TutorMode::noTutor){
        const Go::NodeList& nodeList = board->getCurrentNodeList();
        if (nodeList.indexOf(node) == nodeList.size() - 1){
            board->setTutorMode(BoardWidget::TutorMode::noTutor);
            if (board == currentBoard())
                updateMenu(true);
        }
    }
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
}

/**
  Slot
  score updated
*/
void MainWindow::on_boardWidget_scoreUpdated(int total, int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory){
    BoardWidget* board = NULL;
    CountTerritoryDialog* dialog = NULL;
    DocumentManager::const_iterator iter = docManager.begin();
    while (iter != docManager.end()){
        if (iter->boardWidget == sender()){
            board  = iter->boardWidget;
            dialog = iter->countTerritoryDialog;
            break;
        }
        ++iter;
    }
    if (board == NULL || dialog == NULL)
        return;

    double komi = board->getGameInformation()->komi;
    dialog->setScore(total, alive_b, alive_w, dead_b, dead_w, capturedBlack, capturedWhite, blackTerritory, whiteTerritory, komi);
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

    undoGroup.setActiveStack(boardWidget->document()->getUndoStack());

    updateCaption(false);
    updateMenu(true);
    updateStatusBar(boardWidget);
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

    BoardWidget* board = NULL;
    QTreeWidget* branchWidget = current->treeWidget();
    DocumentManager::const_iterator iter = docManager.begin();
    while (iter != docManager.end()){
        if (iter->branchWidget == branchWidget){
            board = iter->boardWidget;
            break;
        }
        ++iter;
    }
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
  collection view -> activated
*/
void MainWindow::on_collectionView_activated(const QModelIndex& index)
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
void MainWindow::on_openUrl_dataReadProgress(int done, int total){
    progressDialog->setMaximum(total);
    progressDialog->setValue(done);
}

/**
  Slot
  Open URL
*/
void MainWindow::on_openUrl_requestFinished(int id, bool error){
    if (id != downloadID)
        return;

    delete progressDialog;
    sender()->deleteLater();

    QHttp* http = qobject_cast<QHttp*>(sender());
    if (http == NULL)
        return;

    QBuffer* buffer = qobject_cast<QBuffer*>(http->currentDestinationDevice());
    if (buffer == NULL)
        return;

    QHttpResponseHeader res = http->lastResponse();
    if (error == true || res.statusCode() >= 300){
        QString str = tr("Download Failed: %1 %2").arg(res.statusCode()).arg(res.reasonPhrase());
        QMessageBox::critical(this, QString(), str);
        return;
    }

    QString docName = downloadURL.toString();
    int p = docName.lastIndexOf('/');
    if (p >= 0)
        docName = docName.mid(p+1);

    if (downloadNewTab){
        SgfDocument* doc = new SgfDocument(mugoApp()->defaultCodec());
        if (doc->read(docName, buffer->buffer(), true) == false){
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
        if (doc->read(docName, buffer->buffer(), false) == false){
            delete doc;
            return;
        }
        doc->setUrl(downloadURL.toString());
        addDocument(doc, board);
    }
}

/**
  Slot
  Count Territory Dialog finished
*/
void MainWindow::on_scoreDialog_finished(int result){
    BoardWidget* board = NULL;
    DocumentManager::iterator iter = docManager.begin();
    while (iter!= docManager.end()){
        if (iter->countTerritoryDialog == sender()){
            board = iter->boardWidget;
            break;
        }
        ++iter;
    }

    if (board == NULL)
        return;

    board->setScoreMode(BoardWidget::ScoreMode::noScore);
    updateMenu(true);
}
