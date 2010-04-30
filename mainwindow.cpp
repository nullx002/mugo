#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>
#include <QtAlgorithms>
#include <QClipboard>
#include <QUrl>
#include <QHttp>
#include <QPrinter>
#include <QPrintDialog>
#include <QProgressDialog>
#include "appdef.h"
#include "sgf.h"
#include "ugf.h"
#include "gib.h"
#include "ngf.h"
#include "gtp.h"
#include "mainwindow.h"
#include "setupdialog.h"
#include "gameinformationdialog.h"
#include "playwithcomputerdialog.h"
#include "exportasciidialog.h"
#include "printoptiondialog.h"
#include "boardsizedialog.h"
#include "ui_mainwindow.h"


Q_DECLARE_METATYPE(go::nodePtr);
Q_DECLARE_METATYPE(go::informationPtr);



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , tabMenuGroups(this)
    , docIndex(0)
    , undoGroup(this)
    , countTerritoryMode(false)
    , playWithComputerMode(false)
    , stepsOfFastMove(FAST_MOVE_STEPS)
{
    ui->setupUi(this);

    OPEN_FILTER = tr("All Go Format(*.sgf *.ugf *.ugi *.gib *.ngf);;sgf(*.sgf);;ugf(*.ugf *.ugi);;gib(*.gib);;ngf(*.ngf);;All Files(*.*)");

    // hide dock view
    ui->undoDockWidget->setVisible(false);
    ui->collectionDockWidget->setVisible(false);

    // encoding
    codecActions.push_back( ui->actionEncodingUTF8 );
    codecActions.push_back( ui->actionISO8859_1 );
    codecActions.push_back( ui->actionISO8859_2 );
    codecActions.push_back( ui->actionISO8859_3 );
    codecActions.push_back( ui->actionISO8859_4 );
    codecActions.push_back( ui->actionISO8859_5 );
    codecActions.push_back( ui->actionISO8859_6 );
    codecActions.push_back( ui->actionISO8859_7 );
    codecActions.push_back( ui->actionISO8859_8 );
    codecActions.push_back( ui->actionISO8859_9 );
    codecActions.push_back( ui->actionISO8859_10 );
    codecActions.push_back( ui->actionISO8859_11 );
    codecActions.push_back( ui->actionISO8859_13 );
    codecActions.push_back( ui->actionISO8859_14 );
    codecActions.push_back( ui->actionISO8859_15 );
    codecActions.push_back( ui->actionISO8859_16 );
    codecActions.push_back( ui->actionWindows_1250 );
    codecActions.push_back( ui->actionWindows_1251 );
    codecActions.push_back( ui->actionWindows_1252 );
    codecActions.push_back( ui->actionWindows_1253 );
    codecActions.push_back( ui->actionWindows_1254 );
    codecActions.push_back( ui->actionWindows_1255 );
    codecActions.push_back( ui->actionWindows_1256 );
    codecActions.push_back( ui->actionWindows_1257 );
    codecActions.push_back( ui->actionWindows_1258 );
    codecActions.push_back( ui->actionKoi8_R );
    codecActions.push_back( ui->actionKoi8_U );
    codecActions.push_back( ui->actionEncodingGB2312 );
    codecActions.push_back( ui->actionEncodingBig5 );
    codecActions.push_back( ui->actionEncodingShiftJIS );
    codecActions.push_back( ui->actionEncodingJIS );
    codecActions.push_back( ui->actionEncodingEucJP );
    codecActions.push_back( ui->actionEncodingKorean );
    codecNames.push_back( "UTF-8" );
    codecNames.push_back( "ISO-8859-1" );
    codecNames.push_back( "ISO-8859-2" );
    codecNames.push_back( "ISO-8859-3" );
    codecNames.push_back( "ISO-8859-4" );
    codecNames.push_back( "ISO-8859-5" );
    codecNames.push_back( "ISO-8859-6" );
    codecNames.push_back( "ISO-8859-7" );
    codecNames.push_back( "ISO-8859-8" );
    codecNames.push_back( "ISO-8859-9" );
    codecNames.push_back( "ISO-8859-10" );
    codecNames.push_back( "TIS-620" );  // ISO-8859-11
    codecNames.push_back( "ISO-8859-13" );
    codecNames.push_back( "ISO-8859-14" );
    codecNames.push_back( "ISO-8859-15" );
    codecNames.push_back( "ISO-8859-16" );
    codecNames.push_back( "windows-1250" );
    codecNames.push_back( "windows-1251" );
    codecNames.push_back( "windows-1252" );
    codecNames.push_back( "windows-1253" );
    codecNames.push_back( "windows-1254" );
    codecNames.push_back( "windows-1255" );
    codecNames.push_back( "windows-1256" );
    codecNames.push_back( "windows-1257" );
    codecNames.push_back( "windows-1258" );
    codecNames.push_back( "KOI8-R" );
    codecNames.push_back( "KOI8-U" );
    codecNames.push_back( "GB2312" );
    codecNames.push_back( "Big5" );
    codecNames.push_back( "Shift_JIS" );
    codecNames.push_back( "ISO-2022-JP" );
    codecNames.push_back( "EUC-JP" );
    codecNames.push_back( "EUC-KR" );

    // action group
    QActionGroup* showMoveNumberGroup = new QActionGroup(this);
    showMoveNumberGroup->addAction(ui->actionNoMoveNumber);
    showMoveNumberGroup->addAction(ui->actionLast1Move);
    showMoveNumberGroup->addAction(ui->actionLast2Moves);
    showMoveNumberGroup->addAction(ui->actionLast5Moves);
    showMoveNumberGroup->addAction(ui->actionLast10Moves);
    showMoveNumberGroup->addAction(ui->actionLast20Moves);
    showMoveNumberGroup->addAction(ui->actionLast50Moves);
    showMoveNumberGroup->addAction(ui->actionAllMoves);

    QActionGroup* encodingGroup = new QActionGroup(this);
    for (int i=0; i<codecActions.size(); ++i){
        encodingGroup->addAction( codecActions[i] );
        connect( codecActions[i], SIGNAL(triggered()), this, SLOT(setEncoding()) );
    }

    QActionGroup* editGroup = new QActionGroup(this);
    editGroup->addAction(ui->actionAlternateMove);
    editGroup->addAction(ui->actionAddBlackStone);
    editGroup->addAction(ui->actionAddWhiteStone);
    editGroup->addAction(ui->actionAddEmpty);
    editGroup->addAction(ui->actionAddLabel);
    editGroup->addAction(ui->actionAddLabelManually);
    editGroup->addAction(ui->actionAddCircle);
    editGroup->addAction(ui->actionAddCross);
    editGroup->addAction(ui->actionAddSquare);
    editGroup->addAction(ui->actionAddTriangle);
    editGroup->addAction(ui->actionDeleteMarker);

    QActionGroup* languageGroup = new QActionGroup(this);
    languageGroup->addAction(ui->actionLanguageSystemDefault);
    languageGroup->addAction(ui->actionLanguageEnglish);
    languageGroup->addAction(ui->actionLanguageJapanese);


    QSettings settings;

    // codec
    QByteArray codecName = settings.value("codec", "UTF-8").toByteArray();
    defaultCodec = QTextCodec::codecForName(codecName);

    // for open URL
    http = new QHttp(this);
    connect( http, SIGNAL(readyRead(const QHttpResponseHeader&)), this, SLOT(openUrlReadReady(const QHttpResponseHeader&)) );
    connect( http, SIGNAL(dataReadProgress(int, int)), this, SLOT(openUrlReadProgress(int, int)) );
    connect( http, SIGNAL(done(bool)), this, SLOT(openUrlDone(bool)) );

    // recent files
    for (int i=0; i<MaxRecentFiles; ++i){
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        ui->menuRecentFiles->addAction(recentFileActs[i]);
        connect( recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()) );
    }
    updateRecentFileActions();

    // create undo/redo actions
    ui->undoView->setGroup(&undoGroup);

    undoAction = undoGroup.createUndoAction(this);
    redoAction = undoGroup.createRedoAction(this);
    undoAction->setIcon( QIcon(":/res/undo.png") );
    redoAction->setIcon( QIcon(":/res/redo.png") );
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), redoAction);
    ui->menuEdit->insertAction(redoAction, undoAction);
    ui->editToolBar->insertAction(ui->editToolBar->actions().at(0), redoAction);
    ui->editToolBar->insertAction(redoAction, undoAction);

    // create window menu
    ui->menuWindow->insertAction( ui->actionPreviousTab, ui->commentDockWidget->toggleViewAction() );
    ui->menuWindow->insertAction( ui->actionPreviousTab, ui->branchDockWidget->toggleViewAction() );
    ui->menuWindow->insertAction( ui->actionPreviousTab, ui->collectionDockWidget->toggleViewAction() );
    ui->menuWindow->insertAction( ui->actionPreviousTab, ui->undoDockWidget->toggleViewAction() );
    ui->menuWindow->insertSeparator( ui->actionPreviousTab );
    ui->menuToolbars->addAction( ui->mainToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->editToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->navigationToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->viewToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->collectionToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->playGameToolBar->toggleViewAction() );

    // language menu
    QString language = settings.value("language").toString();
    if (language.isEmpty())
        ui->actionLanguageSystemDefault->setChecked(true);
    else if (language == "en")
        ui->actionLanguageEnglish->setChecked(true);
    else if (language == "ja_JP")
        ui->actionLanguageJapanese->setChecked(true);

    // tool menu
    ui->actionPlaySound->setChecked( settings.value("sound/play", 1).toBool() );

    // toolbar (option -> show move number)
    ui->viewToolBar->insertAction( ui->viewToolBar->actions().at(0), ui->menuShowMoveNumber->menuAction() );
    ui->menuShowMoveNumber->menuAction()->setCheckable(true);
    ui->menuShowMoveNumber->menuAction()->setChecked( ui->actionShowMoveNumber->isChecked() );
    connect( ui->menuShowMoveNumber->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionShowMoveNumber_parent_triggered()) );

    // toolbar (collection)
    ui->collectionToolBar->insertAction(ui->actionCollectionExtract, ui->collectionDockWidget->toggleViewAction());
    ui->collectionToolBar->insertSeparator(ui->actionCollectionExtract);
    ui->collectionDockWidget->toggleViewAction()->setIcon( QIcon(":/res/gamelist.png") );

    // toolbar (edit -> stone & marker)
    ui->editToolBar->insertAction( ui->actionDeleteAfterCurrent, ui->menuStoneMarkers->menuAction() );
    ui->editToolBar->insertSeparator( ui->actionDeleteAfterCurrent );
    ui->menuStoneMarkers->menuAction()->setCheckable(true);
    ui->menuStoneMarkers->menuAction()->setIcon(ui->actionAddLabel->icon());
    connect( ui->menuStoneMarkers->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionAddLabel_triggered()) );

    // status bar
#ifdef Q_WS_WIN
    int style = QFrame::NoFrame|QFrame::Plain;
#else
    int style = QFrame::StyledPanel|QFrame::Plain;
#endif
    QLabel* messageLabel = new QLabel;
    messageLabel->setFrameStyle(style);
    ui->statusBar->addWidget(messageLabel, 1);

    moveNumberLabel = new QLabel;
    moveNumberLabel->setFrameStyle(style);
    moveNumberLabel->setToolTip(tr("Move Number"));
    ui->statusBar->addPermanentWidget(moveNumberLabel, 0);

    capturedLabel = new QLabel;
    capturedLabel->setFrameStyle(style);
    capturedLabel->setToolTip(tr("Captured"));
    ui->statusBar->addPermanentWidget(capturedLabel, 0);

    // game list
    ui->collectionWidget->header()->setSortIndicator(0, Qt::AscendingOrder);
    ui->collectionWidget->header()->resizeSection(0, 80);
    ui->collectionWidget->header()->resizeSection(3, 350);

    // keyboard shortcut
    ui->actionNew->setShortcut( QKeySequence::New );
    ui->actionOpen->setShortcut( QKeySequence::Open );
    ui->actionSave->setShortcut( QKeySequence::Save );
    ui->actionSaveAs->setShortcut( QKeySequence::SaveAs );
    ui->actionCloseTab->setShortcut( QKeySequence::Close );
    ui->actionPrint->setShortcut( QKeySequence::Print );
    undoAction->setShortcut( QKeySequence::Undo );
    redoAction->setShortcut( QKeySequence::Redo );
    ui->actionPreviousMove->setShortcut( QKeySequence::Back );
    ui->actionNextMove->setShortcut( QKeySequence::Forward );
//    ui->actionPreviousBranch->setShortcut( QKeySequence::PreviousChild );
//    ui->actionNextBranch->setShortcut( QKeySequence::NextChild );
//    ui->actionMoveFirst->setShortcut( QKeySequence::MoveToStartOfLine );
//    ui->actionMoveLast->setShortcut( QKeySequence::MoveToEndOfLine );
//    ui->actionFastRewind->setShortcut( QKeySequence::MoveToPreviousPage );
//    ui->actionFastForward->setShortcut( QKeySequence::MoveToNextPage );
    ui->actionPreviousTab->setShortcut( QKeySequence::PreviousChild );
    ui->actionNextTab->setShortcut( QKeySequence::NextChild );
    ui->actionCopySgfToClipboard->setShortcut( QKeySequence::Copy );
    ui->actionPasteSgfToNewTab->setShortcut( QKeySequence::Paste );
    ui->actionDeleteAfterCurrent->setShortcut( QKeySequence("Delete") );
    ui->actionDeleteOnlyCurrent->setShortcut( QKeySequence("Ctrl+Delete") );

    // window settings
    restoreGeometry( settings.value("mainwindowGeometry").toByteArray() );
    restoreState( settings.value("docksState").toByteArray() );
    ui->collectionWidget->header()->restoreState( settings.value("collectionState").toByteArray() );

    readSettings();

    // command line
    QStringList args = qApp->arguments();
    if (args.size() > 1)
        for (int i=1; i<args.size(); ++i)
            fileOpen(args[i]);
    else
        fileNew();

    //
    ui->boardTabWidget->removeTab(0);
}

MainWindow::~MainWindow(){
    QSettings settings;
    settings.setValue("mainwindowGeometry", saveGeometry());
    settings.setValue("docksState", saveState());
    settings.setValue("collectionState", ui->collectionWidget->header()->saveState());

    delete ui;
    delete http;
}

BoardWidget* MainWindow::currentBoard(){
    return qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
}

const BoardWidget* MainWindow::currentBoard() const{
    return qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
}

void MainWindow::closeEvent(QCloseEvent* e){
    for (int i=0; i<ui->boardTabWidget->count(); ++i){
        BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(i));
        if (maybeSave(board) == false){
            e->ignore();
            return;
        }
    }
    e->accept();
}

void MainWindow::keyPressEvent(QKeyEvent* /*event*/){
//    if (event->key() == Qt::Key_Delete)
//        deleteNode(true);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
/*
    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
    else{
*/
        const char* ext[] = {"sgf", "ugf", "ugi", "gib", "ngf"};
        int N = sizeof(ext) / sizeof(ext[0]);

        QString localFile = event->mimeData()->urls().front().toLocalFile();
        QFileInfo f(localFile);

        for (int i=0; i<N; ++i)
            if (f.suffix().compare(ext[i], Qt::CaseInsensitive) == 0){
                event->acceptProposedAction();
                break;
            }
//    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    event->acceptProposedAction();
    foreach(const QUrl& f, event->mimeData()->urls()){
        fileOpen( f.toLocalFile() );
    }
}

/**
* Slot
* File -> New
*/
void MainWindow::on_actionNew_triggered(){
    BoardSizeDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    fileNew(dlg.size, dlg.size);
}

/**
* Slot
* File -> Open
*/
void MainWindow::on_actionOpen_triggered(){
    fileOpen();
}

/**
* Slot
* File -> Open URL
*/
void MainWindow::on_actionOpenURL_triggered(){
    downloadBuff.clear();

    QInputDialog dlg(this);
    dlg.resize( 400, dlg.size().height() );
    dlg.setLabelText( tr("Enter the URL of a SGF file.") );
    if (dlg.exec() != QDialog::Accepted)
        return;

    fileNew();

    QUrl url( dlg.textValue() );
    urlOpen(url);
}

/**
* Slot
* File -> Reload
*/
void MainWindow::on_actionReload_triggered(){
    if ( maybeSave(currentBoard()) ){
        TabData& tabData = tabDatas[currentBoard()];
        tabData.branchWidget->clear();
        tabData.nodeToTree.clear();

        if (!tabData.fileName.isEmpty())
            fileOpen(tabData.fileName, false, false, true);
        else if (!tabData.url.isEmpty())
            urlOpen(tabData.url);
    }
}

/**
* Slot
* File -> Save
*/
void MainWindow::on_actionSave_triggered(){
    fileSave( currentBoard() );
}

/**
* Slot
* File -> Save As
*/
void MainWindow::on_actionSaveAs_triggered(){
    fileSaveAs( currentBoard() );
}

/**
* Slot
* File -> Save Board As Picture
*/
void MainWindow::on_actionSaveBoardAsPicture_triggered()
{
    QString selectedFilter;
    QString fname = getSaveFileName(this, QString(), QString(),
        tr("PNG image(*.png);;Bitmap image(*.bmp);;JPEG image(*.jpeg *.jpg);;TIFF image(*.tiff *.tif)"),
        &selectedFilter);
    if (fname.isEmpty())
        return;

    QFileInfo fi(fname);
    if (fi.suffix().isEmpty()){
        if (selectedFilter.indexOf("*.png") >= 0)
            fi.setFile(fname + ".png");
        else if (selectedFilter.indexOf("*.bmp") >= 0)
            fi.setFile(fname + ".bmp");
        else if (selectedFilter.indexOf("*.jpg") >= 0)
            fi.setFile(fname + ".jpg");
        else if (selectedFilter.indexOf("*.tiff") >= 0)
            fi.setFile(fname + ".tiff");
    }

    int w = currentBoard()->width();
    int h = currentBoard()->height();
    w = h = qMin(w, h);
    QImage image(w, h, QImage::Format_RGB32);
    currentBoard()->paintBoard(&image);
    currentBoard()->paintStones(&image);
    currentBoard()->paintTerritories(&image);
    image.save(fi.absoluteFilePath());

    // change image coordinate to display
    currentBoard()->repaintBoard();
}

/**
* Slot
* File -> Export Ascii to Clipboard
*/
void MainWindow::on_actionExportAsciiToClipboard_triggered(){
    ExportAsciiDialog dlg(this, currentBoard()->getBuffer());
    dlg.exec();
}

/**
* Slot
* File -> Collection -> Extract
*/
void MainWindow::on_actionCollectionExtract_triggered(){
    QTreeWidgetItem* item = ui->collectionWidget->currentItem();
    if (item == NULL)
        return;

    QVariant v = item->data(0, Qt::UserRole);
    go::informationPtr info = v.value<go::informationPtr>();

    go::sgf sgf;
    sgf.set(info);

    fileNew();

    currentBoard()->setData(sgf);
    currentBoard()->setDirty(true);
    setTreeData( currentBoard() );
    setCaption();
    updateCollection();
}

/**
* Slot
* File -> Collectio -> Import
*/
void MainWindow::on_actionCollectionImport_triggered(){
    QString fname = getOpenFileName(this, QString(), QString(), OPEN_FILTER);
    if (fname.isEmpty())
        return;

    TabData& tabData = tabDatas[currentBoard()];

    QTextCodec* codec = tabData.codec;
    go::fileBase* data = readFile(fname, codec, true);
    if (data == NULL)
        return;

    currentBoard()->addData(*data);
    delete data;

    setCaption();
    updateCollection();
}

/**
* Slot
* move up
*/
void MainWindow::on_actionCollectionMoveUp_triggered(){
    QTreeWidgetItem* item = ui->collectionWidget->currentItem();
    if (item == NULL)
        return;

    QVariant v = item->data(0, Qt::UserRole);
    go::informationPtr info = v.value<go::informationPtr>();

    go::informationList& rootList = currentBoard()->getData().rootList;
    go::informationList::iterator iter = qFind(rootList.begin(), rootList.end(), info);
    if (iter == rootList.end() || iter == rootList.begin())
        return;
    qSwap(*iter, *(iter-1));

    int currentNo = item->text(0).toInt();
    item->setText(0, QString().sprintf("%5d", --currentNo));

    for(int i=0; i<ui->collectionWidget->topLevelItemCount(); ++i){
        QTreeWidgetItem* item2 = ui->collectionWidget->topLevelItem(i);
        if (item2 == item)
            continue;

        int no = item2->text(0).toInt();
        if (no == currentNo){
            item2->setText(0, QString().sprintf("%5d", no+1));
            break;
        }
    }
    currentBoard()->setDirty(true);
    setCaption();
}

/**
* Slot
* move down
*/
void MainWindow::on_actionCollectionMoveDown_triggered(){
    QTreeWidgetItem* item = ui->collectionWidget->currentItem();
    if (item == NULL)
        return;

    QVariant v = item->data(0, Qt::UserRole);
    go::informationPtr info = v.value<go::informationPtr>();

    go::informationList& rootList = currentBoard()->getData().rootList;
    go::informationList::iterator iter = qFind(rootList.begin(), rootList.end(), info);
    if (iter == rootList.end() || iter+1 == rootList.end())
        return;
    qSwap(*iter, *(iter+1));

    int currentNo = item->text(0).toInt();
    item->setText(0, QString().sprintf("%5d", ++currentNo));

    for(int i=0; i<ui->collectionWidget->topLevelItemCount(); ++i){
        QTreeWidgetItem* item2 = ui->collectionWidget->topLevelItem(i);
        if (item2 == item)
            continue;

        int no = item2->text(0).toInt();
        if (no == currentNo){
            item2->setText(0, QString().sprintf("%5d", no-1));
            break;
        }
    }
    currentBoard()->setDirty(true);
    setCaption();
}

/**
* Slot
* delete from collection
*/
void MainWindow::on_actionDeleteSgfFromCollection_triggered(){
    QTreeWidgetItem* item = ui->collectionWidget->currentItem();
    if (item == NULL)
        return;

    QVariant v = item->data(0, Qt::UserRole);
    go::informationPtr info = v.value<go::informationPtr>();

    go::informationPtr& root= currentBoard()->getData().root;
    go::informationList& rootList = currentBoard()->getData().rootList;
    go::informationList::iterator iter = qFind(rootList.begin(), rootList.end(), info);
    if (*iter == root){
        QMessageBox::warning(this, QString(), tr("Remove sgf from collection failed because this sgf is editing."));
        return;
    }
    rootList.erase(iter);

    int currentNo = item->text(0).toInt();
    delete item;

    for (int i=0; i<ui->collectionWidget->topLevelItemCount(); ++i){
        QTreeWidgetItem* item2 = ui->collectionWidget->topLevelItem(i);
        int no = item2->text(0).toInt();
        if (no > currentNo)
            item2->setText(0, QString().sprintf("%5d", no-1));
    }

    currentBoard()->setDirty(true);
    setCaption();
}

/**
* Slot
* File -> Close Tab
*/
void MainWindow::on_actionCloseTab_triggered(){
    closeTab( ui->boardTabWidget->currentIndex() );
}

/**
* Slot
* File -> Close Tab
*/
void MainWindow::on_actionCloseAllTabs_triggered(){
    closeAllTab();
}

/**
* Slot
* File -> Print
*/
void MainWindow::on_actionPrint_triggered(){
    PrintOptionDialog optionDialog(this);
    if (optionDialog.exec() != QDialog::Accepted)
        return;

//    QPrinter printer(QPrinter::HighResolution);
    QPrinter printer(QPrinter::ScreenResolution);

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() != QDialog::Accepted)
        return;

    currentBoard()->print(printer, optionDialog.printOption(), optionDialog.movesPerPage());
}

/**
* Slot
* File -> Exit
*/
void MainWindow::on_actionExit_triggered(){
    close();
}

/**
* Slot
* File -> Recent Files
*/
void MainWindow::openRecentFile(){
    QAction *action = qobject_cast<QAction*>(sender());
    if (action)
        fileOpen(action->data().toString());
}

/**
* Slot
* Edit -> Copy SGF to Clipboard
*/
void MainWindow::on_actionCopySgfToClipboard_triggered(){
    QString str;
    QTextStream stream(&str, QIODevice::WriteOnly);

    go::sgf sgf;
    sgf.set(currentBoard()->getData().root);
    sgf.saveStream(stream);
    stream.flush();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(str);
}

/**
* Slot
* Edit -> Copy Current SGF Branch to Clipboard
*/
void MainWindow::on_actionCopyCurrentSgfToClipboard_triggered(){
    QString str("(");
    QString s;

    const go::nodeList& nodeList = currentBoard()->getCurrentNodeList();
    go::nodeList::const_iterator iter = nodeList.begin();
    while (iter != nodeList.end()){
        if (s.size() > SGF_LINEWIDTH){
            str.append(s);
            str.push_back('\n');
            s.clear();
        }

        go::sgf::node node;
        node.set(*iter);
        s.append( node.toString() );
        ++iter;
    }
    str.append(s);
    str.push_back(')');

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(str);
}

/**
* Slot
* Edit -> Paste SGF to New Tab
*/
void MainWindow::on_actionPasteSgfToNewTab_triggered(){
    QClipboard *clipboard = QApplication::clipboard();
    QString str = clipboard->text();

    go::sgf sgf;
    QString::iterator first = str.begin();
    sgf.readStream(first, str.end());

    fileNew();

    currentBoard()->setData(sgf);
    currentBoard()->setDirty(true);

    setTreeData( currentBoard() );
    setCaption();
    updateCollection();
}

/**
* Slot
* Edit -> Paste SGF to Collection
*/
void MainWindow::on_actionPasteSgfToCollection_triggered(){
    QClipboard *clipboard = QApplication::clipboard();
    QString str = clipboard->text();

    go::sgf sgf;
    QString::iterator first = str.begin();
    sgf.readStream(first, str.end());

    currentBoard()->addData(sgf);

    setTreeData( currentBoard() );
    setCaption();
    updateCollection();
}

/**
* Slot
* Edit -> Paste SGF as Branch from Clipboard
*/
/*
void MainWindow::on_actionPasteSgfAsBranchFromClipboard_triggered(){
    QClipboard *clipboard = QApplication::clipboard();
    QString str = clipboard->text();

    go::sgf sgf;
    QString::iterator first = str.begin();
    sgf.readStream(first, str.end());

    boardWidget->insertData(boardWidget->getCurrentNode(), sgf);

    setTreeData();
    setCaption();
}
*/

/**
* Slot
* Edit -> Game Information
*/
void MainWindow::on_actionGameInformation_triggered(){
    GameInformationDialog dlg(this, currentBoard()->getData().root.get());
    if (dlg.exec() != QDialog::Accepted)
        return;

    currentBoard()->setDirty(true);
    setCaption();
    updateCollection();
}

/**
* Slot
* Edit -> Pass
*/
void MainWindow::on_actionPass_triggered(){
    currentBoard()->pass();
}

/**
* Slot
* Edit -> Delete
*/
void MainWindow::on_actionDeleteAfterCurrent_triggered(){
    deleteNode(true);
}

void MainWindow::on_actionDeleteOnlyCurrent_triggered(){
    deleteNode(false);
}

/**
* Slot
* Edit -> Stone & Marker -> Alternate Move
*/
void MainWindow::on_actionAlternateMove_triggered(){
    setEditMode(ui->actionAlternateMove, BoardWidget::eAlternateMove);
}

/**
* Slot
* Edit -> Stone & Marker -> Add Black Stone
*/
void MainWindow::on_actionAddBlackStone_triggered(){
    setEditMode(ui->actionAddBlackStone, BoardWidget::eAddBlack);
}

/**
* Slot
* Edit -> Stone & Marker -> Add White Stone
*/
void MainWindow::on_actionAddWhiteStone_triggered(){
    setEditMode(ui->actionAddWhiteStone, BoardWidget::eAddWhite);
}

/**
* Slot
* Edit -> Stone & Marker -> Add Empty
*/
void MainWindow::on_actionAddEmpty_triggered(){
    setEditMode(ui->actionAddEmpty, BoardWidget::eAddEmpty);
}

/**
* Slot
* Edit -> Stone & Marker -> Add Label
*/
void MainWindow::on_actionAddLabel_triggered(){
    setEditMode(ui->actionAddLabel, BoardWidget::eLabelMark);
}

/**
* Slot
* Edit -> Stone & Marker -> Add Label Manually
*/
void MainWindow::on_actionAddLabelManually_triggered(){
    setEditMode(ui->actionAddLabelManually, BoardWidget::eManualMark);
}

/**
* Slot
* Edit -> Stone & Marker -> Add Circle
*/
void MainWindow::on_actionAddCircle_triggered(){
    setEditMode(ui->actionAddCircle, BoardWidget::eCircleMark);
}

/**
* Slot
* Edit -> Stone & Marker -> Add Cross
*/
void MainWindow::on_actionAddCross_triggered(){
    setEditMode(ui->actionAddCross, BoardWidget::eCrossMark);
}

/**
* Slot
* Edit -> Stone & Marker -> Add Square
*/
void MainWindow::on_actionAddSquare_triggered(){
    setEditMode(ui->actionAddSquare, BoardWidget::eSquareMark);
}

/**
* Slot
* Edit -> Stone & Marker -> Add Triangle
*/
void MainWindow::on_actionAddTriangle_triggered(){
    setEditMode(ui->actionAddTriangle, BoardWidget::eTriangleMark);
}

/**
* Slot
* Edit -> Stone & Marker -> Delete Marker
*/
void MainWindow::on_actionDeleteMarker_triggered(){
    setEditMode(ui->actionDeleteMarker, BoardWidget::eDeleteMarker);
}

/**
* Slot
* Edit -> Annotation -> Good Move
*/
void MainWindow::on_actionGoodMove_triggered(){
    setMoveAnnotation(ui->actionGoodMove, go::node::eGoodMove);
}

/**
* Slot
* Edit -> Annotation -> Very Good Move
*/
void MainWindow::on_actionVeryGoodMove_triggered(){
    setMoveAnnotation(ui->actionVeryGoodMove, go::node::eVeryGoodMove);
}

/**
* Slot
* Edit -> Annotation -> Bad Move
*/
void MainWindow::on_actionBadMove_triggered(){
    setMoveAnnotation(ui->actionBadMove, go::node::eBadMove);
}

/**
* Slot
* Edit -> Annotation -> Very Bad Move
*/
void MainWindow::on_actionVeryBadMove_triggered(){
    setMoveAnnotation(ui->actionVeryBadMove, go::node::eVeryBadMove);
}

/**
* Slot
* Edit -> Annotation -> Doubtful Move
*/
void MainWindow::on_actionDoubtfulMove_triggered(){
    setMoveAnnotation(ui->actionDoubtfulMove, go::node::eDoubtfulMove);
}

/**
* Slot
* Edit -> Annotation -> Interesting Move
*/
void MainWindow::on_actionInterestingMove_triggered(){
    setMoveAnnotation(ui->actionInterestingMove, go::node::eInterestingMove);
}

/**
* Slot
* Edit -> Annotation -> Even
*/
void MainWindow::on_actionEven_triggered(){
    setNodeAnnotation(ui->actionEven, go::node::eEven);
}

/**
* Slot
* Edit -> Annotation -> Good for Black
*/
void MainWindow::on_actionGoodForBlack_triggered(){
    setNodeAnnotation(ui->actionGoodForBlack, go::node::eGoodForBlack);
}

/**
* Slot
* Edit -> Annotation -> Very Good for Black
*/
void MainWindow::on_actionVeryGoodForBlack_triggered(){
    setNodeAnnotation(ui->actionVeryGoodForBlack, go::node::eVeryGoodForBlack);
}

/**
* Slot
* Edit -> Annotation -> Good for White
*/
void MainWindow::on_actionGoodForWhite_triggered(){
    setNodeAnnotation(ui->actionGoodForWhite, go::node::eGoodForWhite);
}

/**
* Slot
* Edit -> Annotation -> Very Good for White
*/
void MainWindow::on_actionVeryGoodForWhite_triggered(){
    setNodeAnnotation(ui->actionVeryGoodForWhite, go::node::eVeryGoodForWhite);
}

/**
* Slot
* Edit -> Annotation -> Unclear
*/
void MainWindow::on_actionUnclear_triggered(){
    setNodeAnnotation(ui->actionUnclear, go::node::eUnclear);
}

/**
* Slot
* Edit -> Annotation -> Hotspot
*/
void MainWindow::on_actionHotspot_triggered(){
    setAnnotation(ui->actionHotspot, go::node::eHotspot);
}

/**
* Slot
* Edit -> Move Number -> Set
*/
void MainWindow::on_actionSetMoveNumber_triggered(){
    go::nodePtr node = currentBoard()->getCurrentNode();
    if (!node->isStone())
        return;

    QInputDialog dlg(this);
    dlg.setInputMode(QInputDialog::IntInput);
    dlg.setLabelText( tr("Input move number") );

    if (node->moveNumber > 0)
        dlg.setIntValue(node->moveNumber);

    if (dlg.exec() != QDialog::Accepted)
        return;

    currentBoard()->setMoveNumberCommand(node, dlg.intValue());
}

/**
* Slot
* Edit -> Move Number -> Unset
*/
void MainWindow::on_actionUnsetMoveNumber_triggered(){
    go::nodePtr node = currentBoard()->getCurrentNode();
    currentBoard()->unsetMoveNumberCommand(node);
}

/**
* Slot
* Edit -> Edit Node Name
*/
void MainWindow::on_actionEditNodeName_triggered(){
    go::nodePtr node = currentBoard()->getCurrentNode();
    QInputDialog dlg(this);
    dlg.setLabelText( tr("Input node name") );
    dlg.setTextValue(node->name);
    if (dlg.exec() != QDialog::Accepted)
        return;
    currentBoard()->setNodeNameCommand(node, dlg.textValue());
}

/**
* Slot
* Edit -> White First
*/
void MainWindow::on_actionWhiteFirst_triggered(){
    if (ui->actionWhiteFirst->isChecked())
        currentBoard()->whiteFirst(true);
    else
        currentBoard()->whiteFirst(false);
}

/**
* Slot
* Edit -> Rotate SGF Clockwise
*/
void MainWindow::on_actionRotateSgfClockwise_triggered(){
    currentBoard()->rotateSgfCommand();
}

/**
* Slot
* Edit -> Flip SGF Holizontally
*/
void MainWindow::on_actionFlipSgfHorizontally_triggered(){
    currentBoard()->flipSgfHorizontallyCommand();
}

/**
* Slot
* Edit -> Flip SGF Vertically
*/
void MainWindow::on_actionFlipSgfVertically_triggered(){
    currentBoard()->flipSgfVerticallyCommand();
}

/**
* Slot
* Edit -> Encoding
*/
void MainWindow::setEncoding(){
    setEncoding( qobject_cast<QAction*>(sender()), true );
}

void MainWindow::setEncoding(QAction* action, bool saveToDefault){
    TabData& tabData = tabDatas[currentBoard()];
    tabData.encode = action;
    for (int i=0; i<codecActions.size(); ++i){
        if (codecActions[i] == action){
            tabData.codec = QTextCodec::codecForName( codecNames[i] );

            if (tabData.codec){
                qDebug() << "change codec to " << tabData.codec->name();
                if (saveToDefault){
                    defaultCodec = tabData.codec;
                    QSettings().setValue("codec", defaultCodec->name());
                }
            }
            else
                qDebug() << "codec is null";

            return;
        }
    }
}

void MainWindow::setEncoding(QTextCodec* codec){
    for (int i=0; i<codecNames.size(); ++i){
        if (strcasecmp(codec->name(), codecNames[i]) == 0){
            codecActions[i]->setChecked(true);
            setEncoding(codecActions[i]);
        }
    }
}

/**
* Slot
* Traverse -> First Move
*/
void MainWindow::on_actionMoveFirst_triggered(){
    currentBoard()->setCurrentNode( currentBoard()->getData().root );
}

/**
* Slot
* Traverse -> Fast Rewind
*/
void MainWindow::on_actionFastRewind_triggered(){
    go::nodePtr node = currentBoard()->getCurrentNode();
    if (node->parent() == NULL)
        return;

    for (int i=0; i<stepsOfFastMove; ++i){
        if (node->parent())
            node = node->parent();
        else
            break;
    }
    currentBoard()->setCurrentNode(node);
}

/**
* Slot
* Traverse -> Previous Move
*/
void MainWindow::on_actionPreviousMove_triggered(){
    go::nodePtr node = currentBoard()->getCurrentNode();
    if (node->parent())
        currentBoard()->setCurrentNode(node->parent());
}

/**
* Slot
* Traverse -> Next Move
*/
void MainWindow::on_actionNextMove_triggered(){
    const go::nodeList& nodeList = currentBoard()->getCurrentNodeList();
    go::nodeList::const_iterator iter = qFind(nodeList.begin(), nodeList.end(), currentBoard()->getCurrentNode());
    if (iter != nodeList.end() && ++iter != nodeList.end())
        currentBoard()->setCurrentNode( *iter );
}

/**
* Slot
* Traverse -> Fast Forward
*/
void MainWindow::on_actionFastForward_triggered(){
    const go::nodeList& nodeList = currentBoard()->getCurrentNodeList();
    go::nodeList::const_iterator iter = qFind(nodeList.begin(), nodeList.end(), currentBoard()->getCurrentNode());

    go::nodePtr node = currentBoard()->getCurrentNode();
    for (int i=0; i<stepsOfFastMove; ++i)
        if (iter != nodeList.end() && ++iter != nodeList.end())
            node = *iter;
        else
            break;

    currentBoard()->setCurrentNode(node);
}

/**
* Slot
* Traverse -> Move Last
*/
void MainWindow::on_actionMoveLast_triggered(){
    const go::nodeList& nodeList = currentBoard()->getCurrentNodeList();
    go::nodePtr node = nodeList.back();
    currentBoard()->setCurrentNode(node);
}

/**
* Slot
* Traverse -> Back to parent
*/
void MainWindow::on_actionBackToParent_triggered(){
    go::nodePtr node = currentBoard()->getCurrentNode();
    while(node->parent()){
        node = node->parent();
        if (node->childNodes.size() > 1)
            break;
    }
    currentBoard()->setCurrentNode( node );
}

/**
* Slot
* Traverse -> Previous Branch
*/
void MainWindow::on_actionPreviousBranch_triggered(){
    go::nodePtr node   = currentBoard()->getCurrentNode();
    go::nodePtr parent = node->parent();
    while(parent){
        if (parent->childNodes.size() > 1)
            break;
        node = parent;
        parent = parent->parent();
    }

    if (parent == NULL)
        return;

    go::nodeList::iterator iter = qFind(parent->childNodes.begin(), parent->childNodes.end(), node);
    if (iter == parent->childNodes.begin())
        return;

    currentBoard()->setCurrentNode( *--iter );
}

/**
* Slot
* Traverse -> Next Branch
*/
void MainWindow::on_actionNextBranch_triggered(){
    go::nodePtr node   = currentBoard()->getCurrentNode();
    go::nodePtr parent = node->parent();
    while(parent){
        if (parent->childNodes.size() > 1)
            break;
        node = parent;
        parent = parent->parent();
    }

    if (parent == NULL)
        return;

    go::nodeList::iterator iter = qFind(parent->childNodes.begin(), parent->childNodes.end(), node);
    if (++iter == parent->childNodes.end())
        return;

    currentBoard()->setCurrentNode( *iter );
}

/**
* Slot
* Traverse -> junp to move number
*/
void MainWindow::on_actionJumpToMoveNumber_triggered(){
    QInputDialog dlg(this);
    dlg.setInputMode( QInputDialog::IntInput );
    dlg.setLabelText( tr("Input move number") );
    if (dlg.exec() != QDialog::Accepted)
        return;

    go::nodePtr node = currentBoard()->findNodeFromMoveNumber( dlg.intValue() );
    if (node)
        currentBoard()->setCurrentNode(node);
}

/**
* Slot
* Traverse -> jump to clicked
*/
void MainWindow::on_actionJumpToClicked_triggered(){
    currentBoard()->setMoveToClicked( ui->actionJumpToClicked->isChecked() );
}

/**
* Slot
* View -> Move Number-> Show Move Number
*/
void MainWindow::on_actionShowMoveNumber_triggered(){
    ui->menuShowMoveNumber->menuAction()->setChecked( ui->actionShowMoveNumber->isChecked() );
    currentBoard()->setShowMoveNumber( ui->actionShowMoveNumber->isChecked() );

    QSettings settings;
    settings.setValue("marker/showMoveNumber", ui->actionShowMoveNumber->isChecked());
}

/**
* Slot
* View -> Move Number -> Reset move number in branch.
*/
void MainWindow::on_actionResetMoveNubmerInBranch_triggered(){
    TabData& tabData = tabDatas[currentBoard()];
    if (ui->actionResetMoveNubmerInBranch->isChecked())
        currentBoard()->setMoveNumberMode(tabData.branchMode ? BoardWidget::eResetInBranch : BoardWidget::eResetInVariation );
    else
        currentBoard()->setMoveNumberMode( BoardWidget::eSequential );
}

/**
* Slot
* View -> Move Number-> Show Move Number
*/
void MainWindow::on_actionShowMoveNumber_parent_triggered(){
    ui->actionShowMoveNumber->setChecked( ui->menuShowMoveNumber->menuAction()->isChecked() );
    on_actionShowMoveNumber_triggered();
}

/**
* Slot
* View -> Move Number-> No Move Number
*/
void MainWindow::on_actionNoMoveNumber_triggered(){
    currentBoard()->setShowMoveNumberCount(0);

    QSettings settings;
    settings.setValue("marker/moveNumber", 0);
}

/**
* Slot
* View -> Move Number-> Last 1 move
*/
void MainWindow::on_actionLast1Move_triggered(){
    currentBoard()->setShowMoveNumberCount(1);

    QSettings settings;
    settings.setValue("marker/moveNumber", 1);
}

/**
* Slot
* View -> Move Number-> Last 2 moves
*/
void MainWindow::on_actionLast2Moves_triggered(){
    currentBoard()->setShowMoveNumberCount(2);

    QSettings settings;
    settings.setValue("marker/moveNumber", 2);
}

/**
* Slot
* View -> Move Number-> Last 5 moves
*/
void MainWindow::on_actionLast5Moves_triggered(){
    currentBoard()->setShowMoveNumberCount(5);

    QSettings settings;
    settings.setValue("marker/moveNumber", 5);
}

/**
* Slot
* View -> Move Number-> Last 10 moves
*/
void MainWindow::on_actionLast10Moves_triggered(){
    currentBoard()->setShowMoveNumberCount(10);

    QSettings settings;
    settings.setValue("marker/moveNumber", 10);
}

/**
* Slot
* View -> Move Number-> Last 20 moves
*/
void MainWindow::on_actionLast20Moves_triggered(){
    currentBoard()->setShowMoveNumberCount(20);

    QSettings settings;
    settings.setValue("marker/moveNumber", 20);
}

/**
* Slot
* View -> Move Number-> Last 50 moves
*/
void MainWindow::on_actionLast50Moves_triggered(){
    currentBoard()->setShowMoveNumberCount(50);

    QSettings settings;
    settings.setValue("marker/moveNumber", 50);
}

/**
* Slot
* View -> Move Number-> All Moves
*/
void MainWindow::on_actionAllMoves_triggered(){
    currentBoard()->setShowMoveNumberCount(-1);

    QSettings settings;
    settings.setValue("marker/moveNumber", -1);
}

/**
* Slot
* View -> Show Coordinates
*/
void MainWindow::on_actionShowCoordinate_triggered(){
    currentBoard()->setShowCoordinates( ui->actionShowCoordinate->isChecked() );

    QSettings settings;
    settings.setValue("view/showCoordinate", ui->actionShowCoordinate->isChecked());
}

/**
* Slot
* View -> Show Coordinates with I
*/
void MainWindow::on_actionShowCoordinateI_triggered(){
    currentBoard()->setShowCoordinatesWithI( ui->actionShowCoordinateI->isChecked() );

    go::nodePtr currentNode = currentBoard()->getCurrentNode();

    setTreeData(currentBoard());

    currentBoard()->setCurrentNode(currentNode);

    QSettings settings;
    settings.setValue("view/showCoordinateWithI", ui->actionShowCoordinateI->isChecked());
}

/**
* Slot
* View -> Show Marker
*/
void MainWindow::on_actionShowMarker_triggered(){
    currentBoard()->setShowMarker( ui->actionShowMarker->isChecked() );

    QSettings settings;
    settings.setValue("marker/showMarker", ui->actionShowMarker->isChecked());
}

/**
* Slot
* View -> Show Branch Moves
*/
void MainWindow::on_actionShowBranchMoves_triggered(){
    currentBoard()->setShowBranchMoves( ui->actionShowBranchMoves->isChecked() );

    QSettings settings;
    settings.setValue("marker/showBranchMoves", ui->actionShowMarker->isChecked());
}

/**
* Slot
* View -> Branch Mode
*/
void MainWindow::on_actionBranchMode_triggered(){
    TabData& tabData = tabDatas[currentBoard()];

    tabData.branchMode = ui->actionBranchMode->isChecked();

    go::nodePtr currentNode = currentBoard()->getCurrentNode();
    QTreeWidgetItem* currentItem = tabData.branchWidget->currentItem();

    remakeTreeWidget( currentBoard(), tabData.branchWidget->topLevelItem(0) );

//    boardWidget->setCurrentNode(current);
    tabData.branchWidget->setCurrentItem(currentItem);

    BoardWidget::eMoveNumberMode moveNumberMode = tabData.branchMode ? BoardWidget::eResetInBranch : BoardWidget::eResetInVariation;
    currentBoard()->setMoveNumberMode( ui->actionResetMoveNubmerInBranch->isChecked() ? moveNumberMode : BoardWidget::eSequential );
}

/**
* Slot
* View -> Rotate Clockwise
*/
void MainWindow::on_actionRotateBoardClockwise_triggered(){
    ui->actionRotateBoardClockwise->setChecked( currentBoard()->rotateBoard() != 0 );
}

/**
* Slot
* View -> Flip Horizontally
*/
void MainWindow::on_actionFlipBoardHorizontally_triggered(){
    currentBoard()->flipBoardHorizontally( ui->actionFlipBoardHorizontally->isChecked() );
}

/**
* Slot
* View -> Flip Vertically
*/
void MainWindow::on_actionFlipBoardVertically_triggered(){
    currentBoard()->flipBoardVertically( ui->actionFlipBoardVertically->isChecked() );
}

/**
* Slot
* View -> Reset Board
*/
void MainWindow::on_actionResetBoard_triggered(){
    currentBoard()->resetBoard();
    ui->actionRotateBoardClockwise->setChecked(false);
    ui->actionFlipBoardHorizontally->setChecked(false);
    ui->actionFlipBoardVertically->setChecked(false);
}

/**
* Slot
* Tools -> Count Territoy
*/
void MainWindow::on_actionCountTerritory_triggered(){
    if (ui->actionCountTerritory->isChecked()){
        setCountTerritoryMode( currentBoard(), true );
        TabData& tabData = tabDatas[currentBoard()];
        tabData.countTerritoryDialog->setInformationNode( currentBoard()->getData().root.get() );
        tabData.countTerritoryDialog->show();
    }
    else{
        setCountTerritoryMode( currentBoard(), false);
        setCaption();
    }

    currentBoard()->setCountTerritoryMode(ui->actionCountTerritory->isChecked());
}

/**
* Slot
* Tools -> play with computer
*/
void MainWindow::on_actionPlayWithGnugo_triggered(){
    if (ui->actionPlayWithGnugo->isChecked()){
        // start new game.
        PlayWithComputerDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted){
            ui->actionPlayWithGnugo->setChecked(false);
            return;
        }

        // if select new game, create new tab
        bool isNewGame = dlg.startPosition == PlayWithComputerDialog::eNewGame;
        if (isNewGame){
            if (fileNew(dlg.size, dlg.size, dlg.handicap, dlg.komi) == false){
                ui->actionPlayWithGnugo->setChecked(false);
                return;
            }
            QString engineName = QFileInfo(dlg.path).baseName();
            currentBoard()->getData().root->blackPlayer = dlg.isBlack ? "User" : engineName;
            currentBoard()->getData().root->whitePlayer = dlg.isBlack ? engineName : "User";
            setCaption();
            updateCollection();
        }
        else{
            dlg.size = currentBoard()->getData().root->xsize;
            dlg.komi = currentBoard()->getData().root->komi;
        }

        // create new process
        QString param = '"' + dlg.path + "\" " + dlg.parameter;
        qDebug() << param;

        TabData& tabData = tabDatas[currentBoard()];
        QProcess* gtpProcess = new QProcess(this);
        gtpProcess->start(param, QIODevice::ReadWrite|QIODevice::Text);

        // if process does not launch, alert and return.
        if (gtpProcess->state() == QProcess::NotRunning){
            delete gtpProcess;
            QMessageBox::critical(this, APPNAME, tr("Can not launch computer go program."));
            return;
        }

        // start gtp communication
        delete tabData.playGame;
        tabData.playGame = new gtp(currentBoard(), dlg.isBlack ? go::black : go::white, dlg.size, dlg.komi, dlg.handicap, isNewGame, dlg.level, gtpProcess);
        connect( tabData.playGame, SIGNAL(gameEnded()), this, SLOT(playGameEnded()) );
        setPlayWithComputerMode(currentBoard(), true);
        currentBoard()->playWithComputer(tabData.playGame);
    }
    else{
        // stop playing game.
        if (stopGame(currentBoard()) == false){
            ui->actionPlayWithGnugo->setChecked(true);
            return;
        }
    }
}

/**
* Slot
* Tools -> Tutor Boss Sides
*/
void MainWindow::on_actionTutorBothSides_triggered(){
    if (ui->actionTutorBothSides->isChecked()){
        currentBoard()->setTutorMode(BoardWidget::eTutorBothSides);
        ui->actionTutorOneSide->setChecked(false);
    }
    else
        currentBoard()->setTutorMode(BoardWidget::eNoTutor);
}

/**
* Slot
* Tools -> Tutor One Side
*/
void MainWindow::on_actionTutorOneSide_triggered(){
    if (ui->actionTutorOneSide->isChecked()){
        currentBoard()->setTutorMode(BoardWidget::eTutorOneSide);
        ui->actionTutorBothSides->setChecked(false);
    }
    else
        currentBoard()->setTutorMode(BoardWidget::eNoTutor);
}

/**
* Slot
* Tools -> Play Sound
*/
void MainWindow::on_actionPlaySound_triggered(){
    QSettings setting;
    setting.setValue("sound/play", ui->actionPlaySound->isChecked());
    currentBoard()->setPlaySound( ui->actionPlaySound->isChecked() );
}

/**
* Slot
* Tools -> Custom Board Size
*/
/*
void MainWindow::on_actionCustomBoardSize_triggered(){
    QInputDialog dlg(this);
    dlg.setLabelText( tr("Input new board size. board size must be between 2-52.") );

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString s = dlg.textValue();
    QString w, h;
    QString::iterator iter=s.begin();
    for(; iter!= s.end(); ++iter){
        if (iter->isDigit())
            w.push_back(*iter);
        else{
            ++iter;
            break;
        }
    }
    for(; iter!= s.end(); ++iter){
        if (iter->isDigit())
            h.push_back(*iter);
        else
            break;
    }

    int iW = w.toInt();
    int iH = h.toInt();
    if (iW >= 2 && iW <= 52)
        if (iH == 0 || (iH >= 2 && iH <= 52))
            fileNew(iW, iH == 0 ? iW : iH);
}
*/

/**
* Slot
* Tools -> Language -> System Default
*/
void MainWindow::on_actionLanguageSystemDefault_triggered(){
    QSettings settings;
    settings.remove("language");
    alertLanguageChanged();
}

/**
* Slot
* Tools -> Language -> English
*/
void MainWindow::on_actionLanguageEnglish_triggered(){
    QSettings settings;
    settings.setValue("language", "en");
    alertLanguageChanged();
}

/**
* Slot
* Tools -> Language -> Japanese
*/
void MainWindow::on_actionLanguageJapanese_triggered(){
    QSettings settings;
    settings.setValue("language", "ja_JP");
    alertLanguageChanged();
}

/**
* Slot
* Tools -> Options
*/
void MainWindow::on_actionOptions_triggered(){
    SetupDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    readSettings();
}

/**
* Slot
* Tools -> Clear Settings
*/
void MainWindow::on_actionClearSettings_triggered(){
    if ( QMessageBox::question(this, QString(), tr("Are you sure you want to clear the configuration?"), QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
        return;

    QSettings settings;
    settings.clear();
    readSettings();
}

/**
* Slot
* Window -> Previous Tab
*/
void MainWindow::on_actionPreviousTab_triggered(){
    if (ui->boardTabWidget->count() <= 1)
        return;

    int index = ui->boardTabWidget->currentIndex() - 1;
    if (index < 0)
        index = ui->boardTabWidget->count() - 1;
    ui->boardTabWidget->setCurrentIndex(index);
}

/**
* Slot
* Window->Next Tab
*/
void MainWindow::on_actionNextTab_triggered(){
    if (ui->boardTabWidget->count() <= 1)
        return;

    int index = ui->boardTabWidget->currentIndex() + 1;
    if (index >= ui->boardTabWidget->count())
        index = 0;
    ui->boardTabWidget->setCurrentIndex(index);
}

/**
* Slot
* Help -> About
*/
void MainWindow::on_actionAbout_triggered(){
    QMessageBox::about(this, APPNAME, tr(APPNAME " version " VERSION "\n\nCopyright 2009 " AUTHOR));
}

/**
* Slot
* Help -> About qt
*/
void MainWindow::on_actionAboutQT_triggered(){
    qApp->aboutQt();
}

/**
* SLot
* change tab page
*/
void MainWindow::on_boardTabWidget_currentChanged(QWidget* widget){
    if (widget == NULL){
        fileNew();
        return;
    }

    BoardWidget* board = qobject_cast<BoardWidget*>(widget);
    TabDataMap::iterator iter = tabDatas.begin();
    while (iter != tabDatas.end()){
        iter->branchWidget->setVisible(iter.key() == board);
        iter->countTerritoryDialog->setVisible(false);
        ++iter;
    }

    board->setFocus(Qt::OtherFocusReason);
    tabDatas[board].menuAction->setChecked(true);

    // undo
    undoGroup.setActiveStack(board->getUndoStack());

    ui->commentWidget->setPlainText(board->getCurrentNode()->comment);

    setCaption();
    updateMenu();
    updateCollection();
}

void MainWindow::on_boardTabWidget_tabCloseRequested(int index){
    closeTab(index);
}

/**
* Slot
* board data was cleared.
*/
void MainWindow::boardCleared(){
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    setTreeData(board);
}

/**
* Slot
* new node was created by BoardWidget.
*/
void MainWindow::nodeAdded(go::nodePtr parent, go::nodePtr node, bool /*select*/){
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    if (parent->childNodes.back() == node)
        addTreeWidget(board, node, true);
    else
        remakeTreeWidget( board, tabDatas[board].nodeToTree[node->parent()] );
    setCaption();
}

/**
* Slot
* node was deleted by BoardWidget.
*/
void MainWindow::nodeDeleted(go::nodePtr node, bool deleteChildren){
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    deleteTreeWidget(board, node, deleteChildren);
    if (node->parent())
        remakeTreeWidget( board, tabDatas[board].nodeToTree[node->parent()] );
    setCaption();
}

/**
* Slot
*/
void MainWindow::nodeModified(go::nodePtr node){
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());

    setCaption();

    QTreeWidgetItem* treeWidget = tabDatas[board].nodeToTree[node];
    if (treeWidget == NULL)
        return;
    treeWidget->setText(0, createTreeText(board, node));
}

/**
* Slot
* current node was changed by BoardWidget.
*/
void MainWindow::currentNodeChanged(go::nodePtr node){
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());

    setTreeWidget(board, node);
    ui->commentWidget->setPlainText(node->comment);
    setAnnotation(node->annotation, node->moveAnnotation, node->nodeAnnotation);

    int b, w;
    board->getCaptured(b, w);
    capturedLabel->setText(tr("Dead: Black %1 White %2").arg(b).arg(w));

    int num = board->getMoveNumber();
    QString coord = board->getXYString(node->getX(), node->getY());
    moveNumberLabel->setText(tr("LastMove: %1(%2)").arg(num).arg(coord));

    if (ui->commentWidget->hasFocus())
        tabDatas[board].branchWidget->setFocus();
}

/**
* Slot
* comment dock widget was showed or hid.
*/
void MainWindow::updateTerritory(int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory, double komi){
    tabDatas[currentBoard()].countTerritoryDialog->setScore(alive_b, alive_w, dead_b, dead_w, capturedBlack, capturedWhite, blackTerritory, whiteTerritory, komi);
}

/**
* Slot
* node was changed on branch tree view.
*/
void MainWindow::branchWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/){
    if (current == NULL){
        currentBoard()->setCurrentNode();
        return;
    }

    QTreeWidget* branchWidget = current->treeWidget();
    BoardWidget* boardWidget = NULL;
    TabDataMap::iterator iter = tabDatas.begin();
    while (iter != tabDatas.end()){
        TabData& data = iter.value();
        if (data.branchWidget == branchWidget){
            boardWidget = iter.key();
            break;
        }
        ++iter;
    }

    if (boardWidget == NULL)
        return;

    QVariant v = current->data(0, Qt::UserRole);
    go::nodePtr n = v.value<go::nodePtr>();

    boardWidget->setCurrentNode(n);
}

/**
* Slot
* context menu for branch widget
*/
void MainWindow::branchWidget_customContextMenuRequested(const QPoint& pos){
    QTreeWidget* tree = qobject_cast<QTreeWidget*>( sender() );
    QTreeWidgetItem* current = tree->currentItem();
    if (current == NULL)
        return;

    QVariant v = current->data(0, Qt::UserRole);
    go::nodePtr n = v.value<go::nodePtr>();
    if ( dynamic_cast<go::informationNode*>(n.get()) != NULL ){
        QMenu menu(this);
        menu.addAction( ui->actionGameInformation );
        menu.exec( tree->mapToGlobal(pos) );
    }
    else{
        QMenu menu(this);
        menu.addAction( ui->actionEditNodeName );
        menu.addMenu( ui->menuAnnotation );
        menu.addMenu( ui->menuEditMoveNumber );
        menu.addSeparator();
        menu.addAction( ui->actionDeleteAfterCurrent );
        menu.addAction( ui->actionDeleteOnlyCurrent );

/*
        if (n->parent()->childNodes.size() > 1){
            menu.addSeparator();
            menu.addAction(ui->actionBranchMoveUp);
            menu.addAction(ui->actionBranchMoveDown);
        }
*/

        menu.exec( tree->mapToGlobal(pos) );
    }
}

/**
* Slot
* current branch move up
*/
void MainWindow::on_actionBranchMoveUp_triggered(){
}

/**
* Slot
* current branch move down
*/
void MainWindow::on_actionBranchMoveDown_triggered(){
}

/**
* Slot
*/
void MainWindow::playGameEnded(){
    PlayGame* game = qobject_cast<PlayGame*>( sender() );

    BoardWidget* boardWidget = NULL;
    TabData* tabData = NULL;
    TabDataMap::iterator iter = tabDatas.begin();
    while (iter != tabDatas.end()){
        TabData& data = iter.value();
        if (data.playGame == game){
            boardWidget = iter.key();
            tabData = &iter.value();
            break;
        }
        ++iter;
    }

    if (tabData == NULL || tabData->playGame == NULL)
        return;

    bool abort  = tabData->playGame->isAbort();
    bool resign = tabData->playGame->isResign();
    endGame(boardWidget);

    if (boardWidget != currentBoard())
        return;

    ui->actionPlayWithGnugo->setChecked(false);
    if( !abort && !resign ){
        ui->actionCountTerritory->setChecked(true);
        on_actionCountTerritory_triggered();

        setCaption();
        updateCollection();
    }
    else if (resign){
        boardWidget->getData().root->result = game->winner() == go::white ? "W+R" : "B+R";
        if (game->winner() == game->color())
            QMessageBox::information(boardWidget, APPNAME, tr("Computer resigns."));

        setCaption();
        updateCollection();
    }
}

/**
* Slot
*/
void MainWindow::on_actionGamePass_triggered(){
    on_actionPass_triggered();
}

/**
* Slot
*/
void MainWindow::on_actionGameResign_triggered(){
    if (QMessageBox::warning(this, QString(), tr("Are you sure you want to resign?"), QMessageBox::Ok|QMessageBox::Cancel) != QMessageBox::Ok)
        return;

    PlayGame* game = tabDatas[currentBoard()].playGame;
    game->winner( game->color() == go::black ? go::white : go::black );
    game->quit(true);
}

/**
* Slot
*/
void MainWindow::on_actionGameUndo_triggered(){
    go::nodePtr node = currentBoard()->getCurrentNode();
    for (int i=0; i<2 && node; ++i)
        node = node->parent();
    if (node)
        currentBoard()->undo();
}

/**
* Slot
* comment was modified.
*/
void MainWindow::on_commentWidget_textChanged(){
    go::nodePtr currentNode = currentBoard()->getCurrentNode();
    currentBoard()->setCommentCommand(currentNode, ui->commentWidget->toPlainText());
}

/*
void MainWindow::on_collectionWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem*){
}
*/

/*
void MainWindow::on_collectionWidget_itemDoubleClicked(QTreeWidgetItem* item, int column){
}
*/

/** Slot
* game changed by gamelist window
*/
void MainWindow::on_collectionWidget_itemActivated(QTreeWidgetItem* item, int /*column*/){
    if (item == NULL)
        return;

    QVariant v = item->data(0, Qt::UserRole);
    go::informationPtr info = v.value<go::informationPtr>();
    currentBoard()->setRoot(info);
    setTreeData( currentBoard() );
    setCaption();

    currentBoard()->setFocus(Qt::OtherFocusReason);
}

/**
* Slot
* score dialog was closed
*/
void MainWindow::scoreDialogClosed(int){
    ui->actionCountTerritory->setChecked(false);
    on_actionCountTerritory_triggered();

    setCaption();
    updateCollection();
}

/**
* set text to MainWindow's title bar
*
* if game information has player info, set player name to window text.
*/
void MainWindow::setCaption(){
    go::informationPtr gameInfo = currentBoard()->getData().root;
    bool hasPlayerInfo = !gameInfo->whitePlayer.isEmpty() || !gameInfo->whiteRank.isEmpty() ||
                            !gameInfo->blackPlayer.isEmpty() || !gameInfo->blackRank.isEmpty();

    QString caption;
    if (hasPlayerInfo)
        caption = tr("%1 %2 (W) vs %4 %5 (B) (Result:%7)")
                        .arg(gameInfo->whitePlayer)
                        .arg(gameInfo->whiteRank)
                        .arg(gameInfo->blackPlayer)
                        .arg(gameInfo->blackRank)
                        .arg(gameInfo->result);

    if (!hasPlayerInfo)
        caption = tabDatas[currentBoard()].documentName;

    if (!gameInfo->gameName.isEmpty()){
        caption.append(" - ");
        caption.append(gameInfo->gameName);
    }
    else if (!gameInfo->event.isEmpty()){
        caption.append(" - ");
        caption.append(gameInfo->event);
    }

    if (currentBoard()->isDirty())
        caption.append(" *");

    caption.append(" - ");
    caption.append(APPNAME);

    setWindowTitle(caption);

    QString docName = tabDatas[currentBoard()].documentName;
    if (currentBoard()->isDirty())
        docName.append(" *");
    int tabIndex = ui->boardTabWidget->indexOf(currentBoard());
    ui->boardTabWidget->setTabText(tabIndex, docName);

    tabDatas[currentBoard()].menuAction->setText(docName);
}

/**
*/
void MainWindow::updateMenu(){
    setCountTerritoryMode( currentBoard(), false );
    setPlayWithComputerMode( currentBoard(), false );

    BoardWidget* boardWidget = currentBoard();
    TabData* tabData = &tabDatas[boardWidget];

    if (tabData->fileName.isEmpty())
        ui->actionReload->setEnabled(false);

    switch(boardWidget->getEditMode()){
        case BoardWidget::eAlternateMove:
            setEditMode(ui->actionAlternateMove, BoardWidget::eAlternateMove);
            break;
        case BoardWidget::eAddBlack:
            setEditMode(ui->actionAddBlackStone, BoardWidget::eAddBlack);
            break;
        case BoardWidget::eAddWhite:
            setEditMode(ui->actionAddWhiteStone, BoardWidget::eAddWhite);
            break;
        case BoardWidget::eAddEmpty:
            setEditMode(ui->actionAddEmpty, BoardWidget::eAddEmpty);
            break;
        case BoardWidget::eLabelMark:
            setEditMode(ui->actionAddLabel, BoardWidget::eLabelMark);
            break;
        case BoardWidget::eManualMark:
            setEditMode(ui->actionAddLabelManually, BoardWidget::eManualMark);
            break;
        case BoardWidget::eCrossMark:
            setEditMode(ui->actionAddCross, BoardWidget::eCrossMark);
            break;
        case BoardWidget::eCircleMark:
            setEditMode(ui->actionAddCircle, BoardWidget::eCircleMark);
            break;
        case BoardWidget::eSquareMark:
            setEditMode(ui->actionAddSquare, BoardWidget::eSquareMark);
            break;
        case BoardWidget::eTriangleMark:
            setEditMode(ui->actionAddTriangle, BoardWidget::eTriangleMark);
            break;
        case BoardWidget::eDeleteMarker:
            setEditMode(ui->actionDeleteMarker, BoardWidget::eDeleteMarker);
            break;
        case BoardWidget::eCountTerritory:
        case BoardWidget::ePlayGame:
            break;
    };

    setAnnotation(boardWidget->getCurrentNode()->annotation, boardWidget->getCurrentNode()->moveAnnotation, boardWidget->getCurrentNode()->nodeAnnotation);

    ui->actionWhiteFirst->setChecked( boardWidget->whiteFirst() );

    tabData->encode->setChecked(true);

    ui->menuShowMoveNumber->menuAction()->setChecked( boardWidget->getShowMoveNumber() );
    ui->actionResetMoveNubmerInBranch->setChecked( boardWidget->getMoveNumberMode() != BoardWidget::eSequential );
    ui->actionShowMoveNumber->setChecked( boardWidget->getShowMoveNumber() );
    ui->actionNoMoveNumber->setChecked( boardWidget->getShowMoveNumberCount() == 0 );
    ui->actionLast1Move->setChecked( boardWidget->getShowMoveNumberCount() == 1 );
    ui->actionLast2Moves->setChecked( boardWidget->getShowMoveNumberCount() == 2 );
    ui->actionLast5Moves->setChecked( boardWidget->getShowMoveNumberCount() == 5 );
    ui->actionLast10Moves->setChecked( boardWidget->getShowMoveNumberCount() == 10 );
    ui->actionLast20Moves->setChecked( boardWidget->getShowMoveNumberCount() == 20 );
    ui->actionLast50Moves->setChecked( boardWidget->getShowMoveNumberCount() == 50 );
    ui->actionAllMoves->setChecked( boardWidget->getShowMoveNumberCount() == -1 );

    ui->actionShowCoordinate->setChecked( boardWidget->getShowCoordinates() );
    ui->actionShowCoordinateI->setChecked( boardWidget->getShowCoordinatesWithI() );
    ui->actionShowMarker->setChecked( boardWidget->getShowMarker() );
    ui->actionShowBranchMoves->setChecked( boardWidget->getShowBranchMoves() );
    ui->actionRotateBoardClockwise->setChecked( boardWidget->getRotateBoard() != 0 );
    ui->actionFlipBoardHorizontally->setChecked( boardWidget->getFlipBoardHorizontally() );
    ui->actionFlipBoardVertically->setChecked( boardWidget->getFlipBoardVertically() );

    ui->actionBranchMode->setChecked( tabData->branchMode );
    ui->actionAutomaticReplay->setChecked( boardWidget->isAutoReplay() );
    ui->actionTutorBothSides->setChecked( boardWidget->getTutorMode() == BoardWidget::eTutorBothSides );
    ui->actionTutorOneSide->setChecked( boardWidget->getTutorMode() == BoardWidget::eTutorOneSide );

    if (boardWidget->getEditMode() == BoardWidget::eCountTerritory){
        setCountTerritoryMode(currentBoard(), true);
        ui->actionCountTerritory->setChecked(true);
        tabData->countTerritoryDialog->setVisible(true);
    }
    else{
        ui->actionCountTerritory->setChecked(false);
        tabData->countTerritoryDialog->setVisible(false);
    }

    if (boardWidget->getEditMode() == BoardWidget::ePlayGame){
        setPlayWithComputerMode(currentBoard(), true);
        ui->actionPlayWithGnugo->setChecked( true );
    }
    else
        ui->actionPlayWithGnugo->setChecked( false );
}

/**
*/
void MainWindow::updateCollection(){
    int no = 0;
    ui->collectionWidget->clear();
    foreach(const go::informationPtr& info, currentBoard()->getData().rootList){
        QStringList strs;

        QString noStr;
        noStr.sprintf("%5d", ++no);

        QString name = info->gameName.isEmpty() ? info->event : info->gameName;
        strs << noStr << info->blackPlayer << info->whitePlayer << name << info->date << info->result;
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->collectionWidget, strs);

        QVariant v;
        v.setValue(info);
        item->setData(0, Qt::UserRole, v);
        ui->collectionWidget->addTopLevelItem(item);

        if (currentBoard()->getData().root == info){
            ui->collectionWidget->setCurrentItem(item);
            ui->collectionWidget->scrollToItem(item);
        }
    }
}

void MainWindow::addDocument(BoardWidget* board){
    QTreeWidget* tree = new QTreeWidget(ui->branchDockWidgetContents);
    connect( tree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(branchWidget_customContextMenuRequested(const QPoint&)) );
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    tree->setHeaderHidden(true);
    tree->setIndentation(17);
    ui->branchLayout->addWidget(tree);

    TabData& data = tabDatas[board];
    data.menuAction = new QAction(board);
    tabMenuGroups.addAction(data.menuAction);
    data.menuAction->setCheckable(true);

    data.branchWidget = tree;
    data.documentName = tr("Untitled-%1").arg(docIndex);
    data.codec = defaultCodec;
    data.encode = ui->actionEncodingUTF8;
    data.countTerritoryDialog = new CountTerritoryDialog(this);

    ui->menuWindow->addAction(data.menuAction);
    connect(data.menuAction, SIGNAL(triggered()), this, SLOT(onTabChangeRequest()));
    connect(data.countTerritoryDialog, SIGNAL(finished(int)), this, SLOT(scoreDialogClosed(int)));

    ui->boardTabWidget->addTab(board, data.documentName);

    setDocument(board);

    // board widget
    connect(board, SIGNAL(cleared()), this, SLOT(boardCleared()));
    connect(board, SIGNAL(nodeAdded(go::nodePtr,go::nodePtr,bool)), this, SLOT(nodeAdded(go::nodePtr,go::nodePtr,bool)));
    connect(board, SIGNAL(nodeDeleted(go::nodePtr,bool)), this, SLOT(nodeDeleted(go::nodePtr, bool)));
    connect(board, SIGNAL(nodeModified(go::nodePtr)), this, SLOT(nodeModified(go::nodePtr)));
    connect(board, SIGNAL(currentNodeChanged(go::nodePtr)), this, SLOT(currentNodeChanged(go::nodePtr)));
    connect(board, SIGNAL(updateTerritory(int,int,int,int,int,int,int,int,double)), this, SLOT(updateTerritory(int,int,int,int,int,int,int,int,double)));
    connect(board, SIGNAL(automaticReplayEnded()), this, SLOT(automaticReplay_ended()));

    // branch widget
    connect(tree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(branchWidget_currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

void MainWindow::setDocument(BoardWidget* board){
    ui->boardTabWidget->setCurrentWidget(board);
    undoGroup.setActiveStack(board->getUndoStack());
}

/**
* a new document is created if current document can be closed.
*/
bool MainWindow::fileNew(int xsize, int ysize, int handicap, double komi){
    ++docIndex;
    BoardWidget* board = new BoardWidget;
    addDocument( board );

    // edit mode
    setEditMode(ui->actionAlternateMove, BoardWidget::eAlternateMove);

    // board
    board->setBoardSize(xsize, ysize);
    board->getData().root->handicap = handicap;
    board->getData().root->komi = komi;

    setEncoding(defaultCodec);

    setTreeData(board);
    updateCollection();

    return true;
}

/**
* file open.
*/
bool MainWindow::fileOpen(){
    QString fname = getOpenFileName(this, QString(), QString(), OPEN_FILTER);
    if (fname.isEmpty())
        return  false;

    return fileOpen(fname);
}

/**
* file open.
*/
bool MainWindow::fileOpen(const QString& fname, bool guessCodec, bool newTab, bool forceOpen){
    if (!forceOpen){
        TabDataMap::iterator iter = tabDatas.begin();
        while (iter != tabDatas.end()){
            if (iter->fileName.compare(fname, Qt::CaseInsensitive) == 0){
                ui->boardTabWidget->setCurrentWidget(iter.key());
                return true;
            }
            ++iter;
        }
    }

    QTextCodec* codec = newTab == true ? defaultCodec : tabDatas[currentBoard()].codec;

    go::fileBase* data = readFile(fname, codec, guessCodec);
    if (data == NULL)
        return false;

    BoardWidget* board = currentBoard();
    if (newTab){
        board = new BoardWidget;
        addDocument(board);
    }
    board->setData(*data);
    delete data;

    setFileName(board, fname);
    setTreeData(board);
    setCaption();
    updateCollection();

    ui->actionReload->setEnabled(true);

    setEncoding(codec);

    return true;
}

bool MainWindow::urlOpen(const QUrl& url){
    tabDatas[currentBoard()].url = url;
    tabDatas[currentBoard()].fileName.clear();
    QString fname = url.toString();
    int p = fname.lastIndexOf('/');
    if (p >= 0)
        tabDatas[currentBoard()].documentName = fname.mid(p+1);

    QHttp::ConnectionMode mode =
    url.scheme().toLower() == "https" ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;
    http->setHost(url.host(), mode, url.port() == -1 ? 0 : url.port());

    QByteArray path = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
    if (path.isEmpty())
        path = "/";
    http->get(path);

    progressDialog = new QProgressDialog(tr("Downloading SGF File"), "cancel", 0, 100, this);
    connect(progressDialog, SIGNAL(canceled()), this, SLOT(openUrlCancel()));
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->show();
    progressDialog->setValue(0);

    ui->actionReload->setEnabled(true);

    return true;
}

go::fileBase* MainWindow::readFile(const QString& fname, QTextCodec*& codec, bool guessCodec){
#define READ_FILE(FORMAT){\
    go::FORMAT* data = new go::FORMAT;\
    if (data->read(fname, codec, guessCodec)){\
        codec = data->codec;\
        return data;\
    }\
    else{\
        delete data;\
        return NULL;\
    }\
}

    QFileInfo info(fname);
    QString ext = info.suffix().toLower();

    if (ext.compare("sgf") == 0){
        READ_FILE(sgf);
    }
    else if (ext.compare("ugf") == 0 || ext.compare("ugi") == 0){
        READ_FILE(ugf);
    }
    else if (ext.compare("gib") == 0){
        READ_FILE(gib);
    }
    else if (ext.compare("ngf") == 0){
        READ_FILE(ngf);
    }
    else
        return NULL;
}

/**
* file save.
*/
bool MainWindow::fileSave(BoardWidget* boardWidget){
    const QString& fileName = tabDatas[boardWidget].fileName;
    QFileInfo fi(fileName);
    if (fileName.isEmpty() || fi.suffix().compare("sgf", Qt::CaseInsensitive) != 0)
        return fileSaveAs(boardWidget);
    else
        return fileSaveAs(boardWidget, fileName);
}

/**
* file saveas.
*/
bool MainWindow::fileSaveAs(BoardWidget* boardWidget){
    QString filter = tr("sgf(*.sgf)");

    TabData& tabData = tabDatas[boardWidget];
    QString defaultName;
    if (!tabData.fileName.isEmpty())
        defaultName = tabData.fileName;
    else{
        defaultName = getDefaultSaveName();
        if (defaultName.isEmpty())
            defaultName = tabData.documentName;
    }

    QFileInfo fi(defaultName);
    fi.setFile(fi.absoluteDir(), fi.baseName() + ".sgf");

    defaultName = fi.absoluteFilePath();
    QString fname = getSaveFileName(this, QString(), defaultName, filter);
    if (fname.isEmpty())
        return false;

    fi.setFile(fname);
    if (fi.suffix().isEmpty())
        fname.append(".sgf");

    return fileSaveAs(boardWidget, fname);
}

/**
* file saveas.
*/
bool MainWindow::fileSaveAs(BoardWidget* boardWidget, const QString& fname){
    setFileName(boardWidget, fname);
    go::sgf sgf;
    boardWidget->getData(sgf);
    sgf.save(fname, tabDatas[boardWidget].codec);
    boardWidget->setDirty(false);

    setCaption();

    ui->actionReload->setEnabled(true);

    return true;
}

/**
* tab close.
*/
bool MainWindow::closeTab(int index){
    // fileClose() will be processed to current tab(variable boardWidget, tabData)
    // if index is not current tab, set current tab to boardWidget and tabData.
    BoardWidget* boardWidget = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(index));
    TabData*     tabData     = &tabDatas[boardWidget];

    if (maybeSave(boardWidget) == false)
        return false;

    // delete boardWidget's datas.
    boardWidget->clear();
    delete tabData->menuAction;
    delete tabData->playGame;
    delete tabData->branchWidget;
    delete tabData->countTerritoryDialog;
    tabDatas.remove(boardWidget);

    // delete boardWidget after QTabWidget::removeTab
    ui->boardTabWidget->removeTab(index);
    delete boardWidget;

    return true;
}

/**
* all tab close.
*/
bool MainWindow::closeAllTab(){
    int count = ui->boardTabWidget->count();
    for (int i=0; i<count; ++i){
        BoardWidget* boardWidget = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(i));
        if (maybeSave(boardWidget) == false)
            return false;
    }

    for (int i=0; i<count; ++i){
        BoardWidget* boardWidget = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(0));
        boardWidget->setDirty(false);
        closeTab(0);
    }

    return true;
}

/**
*/
bool MainWindow::maybeSave(BoardWidget* boardWidget){
    if (stopGame(boardWidget) == false)
        return false;

    if (!boardWidget->isDirty())
        return true;

    QMessageBox::StandardButton ret =
    QMessageBox::warning(this, APPNAME,
                               tr("%1 has been modified.\n"
                                  "Do you want to save your changes?").arg(tabDatas[boardWidget].documentName),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave(boardWidget);
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

/**
*/
void MainWindow::setFileName(BoardWidget* boardWidget, const QString& fname){
    tabDatas[boardWidget].fileName = fname;
    tabDatas[boardWidget].documentName = QFileInfo(fname).fileName();
    tabDatas[boardWidget].url.clear();

#if defined(Q_WS_WIN)
    tabDatas[boardWidget].fileName.replace("\\", "/");
#endif

    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(tabDatas[boardWidget].fileName);
    files.prepend(tabDatas[boardWidget].fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();
    settings.setValue("recentFileList", files);

    updateRecentFileActions();
}

/**
*/
void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = QFileInfo(files[i]).fileName();
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);
}

/**
* slot
* receive open url data.
*/
void MainWindow::openUrlReadReady(const QHttpResponseHeader& resp){
    switch (resp.statusCode()) {
        case 200:                   // Ok
        case 301:                   // Moved Permanently
        case 302:                   // Found
        case 303:                   // See Other
        case 307:                   // Temporary Redirect
            // these are not error conditions
            break;

        default:
            QMessageBox::information(this, APPNAME,
                                     tr("Download failed: %1.")
                                     .arg(resp.reasonPhrase()));
            http->abort();
    }

    QByteArray ba = http->readAll();
    downloadBuff.append(ba);
}

/**
* slot
*/
void MainWindow::openUrlReadProgress(int done, int total){
    progressDialog->setRange(0, total);
    progressDialog->setValue(done);
}

/**
* slot
*/
void MainWindow::openUrlDone(bool error){
    qDebug() << "openUrlDone: " << error;
    delete progressDialog;

    if (error == true){
        downloadBuff.clear();
        return;
    }

    go::sgf sgf;
    sgf.read(downloadBuff, defaultCodec, true);

    downloadBuff.clear();
    currentBoard()->setData(sgf);

    setTreeData(currentBoard());
    setCaption();
    updateCollection();
}

/**
* slot
*/
void MainWindow::openUrlCancel(){
    http->abort();
}

/**
* slot
*/
void MainWindow::onTabChangeRequest(){
    QWidget* widget = qobject_cast<QWidget*>(sender()->parent());
    ui->boardTabWidget->setCurrentWidget(widget);
}

/**
*/
void MainWindow::setTreeData(BoardWidget* board){
    TabData& tabData = tabDatas[board];
    tabData.branchWidget->clear();
    tabData.nodeToTree.clear();

    addTreeWidget(board, board->getData().root);
    board->setCurrentNode();
    board->repaint();
}

QTreeWidgetItem* MainWindow::addTreeWidget(BoardWidget* board, go::nodePtr node, bool needRemake){
    TabData& tabData = tabDatas[board];
    QTreeWidgetItem* treeWidget = tabData.nodeToTree[node];
    QTreeWidgetItem* newWidget  = NULL;
    if (treeWidget == NULL)
        newWidget = createTreeWidget(board, node);
    QTreeWidgetItem* parentWidget  = (node->parent() && tabData.nodeToTree[node->parent()]) ? tabData.nodeToTree[node->parent()] : tabData.branchWidget->invisibleRootItem();
    QTreeWidgetItem* parentWidget2 = parentWidget->parent() ? parentWidget->parent() : tabData.branchWidget->invisibleRootItem();
    go::nodePtr parentNode  = node->parent();
    go::nodePtr parentNode2 = getNode(parentWidget2);

    bool newBranch = false;
    int  index = 0;

    if (parentNode2 && parentNode2->childNodes.size() > 1){
        newBranch = true;
        go::nodeList::iterator iter = qFind(node->parent()->childNodes.begin(), node->parent()->childNodes.end(), node);
        index = std::distance(node->parent()->childNodes.begin(), iter);
    }
    else if (tabData.branchMode && parentNode && parentNode->childNodes.size() > 1){
        newBranch = true;
        go::nodeList::iterator iter = qFind(node->parent()->childNodes.begin(), node->parent()->childNodes.end(), node);
        index = std::distance(node->parent()->childNodes.begin(), iter);
    }
    else if(!tabData.branchMode && parentNode && parentNode->childNodes.front() != node){
        newBranch = true;
        go::nodeList::iterator iter = qFind(node->parent()->childNodes.begin(), node->parent()->childNodes.end(), node);
        index = std::distance(node->parent()->childNodes.begin(), iter) - 1;
    }

    if (newBranch){
        if (newWidget){
            if (parentWidget->childCount() > index)
                parentWidget->insertChild(index, newWidget);
            else
                parentWidget->addChild(newWidget);
            if (needRemake)
                remakeTreeWidget(board, parentWidget);
        }
        else if(parentWidget->indexOfChild(treeWidget) != index){
            QTreeWidgetItem* p = treeWidget->parent() ? treeWidget->parent() : tabData.branchWidget->invisibleRootItem();
            p->removeChild(treeWidget);
            if (parentWidget->childCount() > index)
                parentWidget->insertChild(index, treeWidget);
            else
                parentWidget->addChild(treeWidget);
        }
    }
    else{
        if (newWidget){
            int index = parentWidget2->indexOfChild(parentWidget);
            parentWidget2->insertChild(index+1, newWidget);
            if (needRemake)
                remakeTreeWidget(board, parentWidget2);
        }
        else if(parentWidget2->indexOfChild(treeWidget) < 0){
            QTreeWidgetItem* p = treeWidget->parent() ? treeWidget->parent() : tabData.branchWidget->invisibleRootItem();
            bool isselected = treeWidget->isSelected();
            p->removeChild(treeWidget);
            parentWidget2->addChild(treeWidget);
            if (isselected)
                tabData.branchWidget->setCurrentItem(treeWidget);
        }
    }

    go::nodeList::iterator iter = node->childNodes.begin();
    while (iter != node->childNodes.end()){
        addTreeWidget(board, *iter);
        ++iter;
    }

    return treeWidget;
}

QTreeWidgetItem* MainWindow::createTreeWidget(BoardWidget* board, go::nodePtr node){
    // Create TreeItem Widget
    QTreeWidgetItem* nodeWidget = new QTreeWidgetItem( QStringList(createTreeText(board, node)) );

    static QIcon treeIconGreen(":/res/green_64.png");
    static QIcon treeIconBlack(":/res/black_64.png");
    static QIcon treeIconWhite(":/res/white_64.png");

    QVariant v;
    v.setValue(node);
    nodeWidget->setData(0, Qt::UserRole, v);
    tabDatas[board].nodeToTree[node] = nodeWidget;

    // Set Icon to TreeItem Widget
    if (node->isStone() && node->isBlack())
        nodeWidget->setIcon(0, treeIconBlack);
    else if (node->isStone() && node->isWhite())
        nodeWidget->setIcon(0, treeIconWhite);
    else
        nodeWidget->setIcon(0, treeIconGreen);

    return nodeWidget;
}

QTreeWidgetItem* MainWindow::remakeTreeWidget(BoardWidget* board, QTreeWidgetItem* treeWidget){
    go::nodePtr node = getNode(treeWidget);
    if (node == NULL)
        return NULL;
    go::nodeList::iterator iter = node->childNodes.begin();
    while (iter != node->childNodes.end()){
        addTreeWidget(board, *iter);
        ++iter;
    }
    return NULL;
}

void MainWindow::deleteNode(bool deleteChildren){
    if (countTerritoryMode || playWithComputerMode)
        return;

    currentBoard()->deleteNodeCommand( currentBoard()->getCurrentNode(), deleteChildren );
}

void MainWindow::deleteTreeWidget(BoardWidget* board, go::nodePtr node, bool deleteChildren){
    QTreeWidgetItem* treeWidget = tabDatas[board].nodeToTree[node];
    go::nodeList::iterator iter = node->childNodes.begin();
    while (iter != node->childNodes.end()){
        QTreeWidgetItem* treeWidget2 = tabDatas[board].nodeToTree[*iter];
        if (treeWidget2->parent() != treeWidget)
            deleteTreeWidget(board, *iter, deleteChildren);
        ++iter;
    }

    deleteTreeWidgetForMap(board, node);
    delete treeWidget;
}

void MainWindow::deleteTreeWidgetForMap(BoardWidget* board, go::nodePtr node){
    NodeToTreeWidgetType& nodeToTree = tabDatas[board].nodeToTree;
    NodeToTreeWidgetType::iterator iter = nodeToTree.find(node);
    if(iter == nodeToTree.end())
        return;

    go::nodeList::iterator iter2 = node->childNodes.begin();
    while (iter2 != node->childNodes.end()){
        NodeToTreeWidgetType::iterator iter3 = nodeToTree.find(*iter2);
        if(iter3 != nodeToTree.end())
            if ((*iter3)->parent() == *iter || (*iter3)->parent() == (*iter)->parent())
                deleteTreeWidgetForMap(board, *iter2);

        ++iter2;
    }

    nodeToTree.erase(iter);
}

void MainWindow::setTreeWidget(BoardWidget* board, go::nodePtr n){
    NodeToTreeWidgetType::iterator iter = tabDatas[board].nodeToTree.find(n);
    if (iter != tabDatas[board].nodeToTree.end())
        tabDatas[board].branchWidget->setCurrentItem( iter.value() );
}

QString MainWindow::createTreeText(BoardWidget* board, const go::nodePtr node){
    QString s;
    if (node->isStone()){
        if (node->isPass())
            s.append( tr("Pass") );
        else
            s.append( board->getXYString(node->getX(), node->getY()) );
    }

    if (!s.isEmpty())
        s.push_back(' ');
    s.append( node->toString() );

    if (s.isEmpty())
        s = tr("Other");

    return s;
}

go::nodePtr MainWindow::getNode(QTreeWidgetItem* treeWidget){
    if (treeWidget == NULL)
        return go::nodePtr();

    QVariant v = treeWidget->data(0, Qt::UserRole);
    return v.value<go::nodePtr>();
}

void MainWindow::setEditMode(QAction* action, BoardWidget::eEditMode editMode){
    action->setChecked(true);
    ui->menuStoneMarkers->menuAction()->setChecked( !ui->actionAlternateMove->isChecked() );

    if (action != ui->actionAlternateMove){
        ui->menuStoneMarkers->menuAction()->setIcon( action->icon() );
        ui->menuStoneMarkers->menuAction()->disconnect();
        if (action == ui->actionAddBlackStone)
            connect( ui->menuStoneMarkers->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionAddBlackStone_triggered()) );
        else if (action == ui->actionAddWhiteStone)
            connect( ui->menuStoneMarkers->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionAddWhiteStone_triggered()) );
        else if (action == ui->actionAddEmpty)
            connect( ui->menuStoneMarkers->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionAddEmpty_triggered()) );
        else if (action == ui->actionAddLabel)
            connect( ui->menuStoneMarkers->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionAddLabel_triggered()) );
        else if (action == ui->actionAddLabelManually)
            connect( ui->menuStoneMarkers->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionAddLabelManually_triggered()) );
        else if (action == ui->actionAddCircle)
            connect( ui->menuStoneMarkers->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionAddCircle_triggered()) );
        else if (action == ui->actionAddCross)
            connect( ui->menuStoneMarkers->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionAddCross_triggered()) );
        else if (action == ui->actionAddSquare)
            connect( ui->menuStoneMarkers->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionAddSquare_triggered()) );
        else if (action == ui->actionAddTriangle)
            connect( ui->menuStoneMarkers->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionAddTriangle_triggered()) );
        else if (action == ui->actionDeleteMarker)
            connect( ui->menuStoneMarkers->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionDeleteMarker_triggered()) );
    }

    currentBoard()->setEditMode(editMode);
}

void MainWindow::setAnnotation(int annotation, int moveAnnotation, int nodeAnnotation){
   static QAction* actions[] = {
        ui->actionHotspot,
    };
    static const int N = sizeof(actions) / sizeof(actions[0]);
    for (int i=0; i<N; ++i)
        actions[i]->setChecked( i+1 == annotation );

    static QAction* moveActions[] = {
        ui->actionGoodMove,
        ui->actionVeryGoodMove,
        ui->actionBadMove,
        ui->actionVeryBadMove,
        ui->actionDoubtfulMove,
        ui->actionInterestingMove,
    };
    static const int moveN = sizeof(moveActions) / sizeof(moveActions[0]);
    for (int i=0; i<moveN; ++i)
        moveActions[i]->setChecked( i+1 == moveAnnotation );

    static QAction* nodeActions[] = {
        ui->actionEven,
        ui->actionGoodForBlack,
        ui->actionVeryGoodForBlack,
        ui->actionGoodForWhite,
        ui->actionVeryGoodForWhite,
        ui->actionUnclear,
    };
    static const int nodeN = sizeof(nodeActions) / sizeof(nodeActions[0]);
    for (int i=0; i<nodeN; ++i)
        nodeActions[i]->setChecked( i+1 == nodeAnnotation );
}

void MainWindow::setAnnotation(QAction* action, int annotation){
    currentBoard()->setAnnotation(action->isChecked() ? annotation : 0);
}

void MainWindow::setMoveAnnotation(QAction* action, int annotation){
   static QAction* actions[] = {
        ui->actionGoodMove,
        ui->actionVeryGoodMove,
        ui->actionBadMove,
        ui->actionVeryBadMove,
        ui->actionDoubtfulMove,
        ui->actionInterestingMove,
    };
    static const int N = sizeof(actions) / sizeof(actions[0]);

    for (int i=0; i<N; ++i)
        if (actions[i] != action)
            actions[i]->setChecked( false );

    currentBoard()->setMoveAnnotation(action->isChecked() ? annotation : 0);
}

void MainWindow::setNodeAnnotation(QAction* action, int annotation){
   static QAction* actions[] = {
        ui->actionEven,
        ui->actionGoodForBlack,
        ui->actionVeryGoodForBlack,
        ui->actionGoodForWhite,
        ui->actionVeryGoodForWhite,
        ui->actionUnclear,
    };
    static const int N = sizeof(actions) / sizeof(actions[0]);

    for (int i=0; i<N; ++i)
        if (actions[i] != action)
            actions[i]->setChecked( false );

    currentBoard()->setNodeAnnotation(action->isChecked() ? annotation : 0);
}

void MainWindow::setCountTerritoryMode(BoardWidget* board, bool on){
    countTerritoryMode = on;

    static QAction* act[] = {
//        ui->actionNew,
//        ui->actionOpen,
//        ui->actionOpenDuplicate,
//        ui->actionOpenURL,
        ui->actionSave,
        ui->actionSaveAs,
        ui->actionReload,
        ui->actionSaveBoardAsPicture,
        ui->actionExportAsciiToClipboard,
//        ui->actionCloseTab,
//        ui->actionCloseAllTabs,
//        ui->actionPrint,
//        ui->actionExit,

        ui->actionCopySgfToClipboard,
        ui->actionCopyCurrentSgfToClipboard,
        ui->actionPasteSgfToNewTab,
        ui->actionPasteSgfToCollection,
//        ui->actionPasteSgfAsBranchFromClipboard,
        ui->actionGameInformation,
        ui->actionDeleteAfterCurrent,
        ui->actionDeleteOnlyCurrent,
        ui->actionPass,
        ui->actionAlternateMove,
        ui->actionAddBlackStone,
        ui->actionAddWhiteStone,
        ui->actionAddEmpty,
        ui->actionAddLabel,
        ui->actionAddCircle,
        ui->actionAddCross,
        ui->actionAddSquare,
        ui->actionAddTriangle,
        ui->actionDeleteMarker,
        ui->actionGoodMove,
        ui->actionVeryGoodMove,
        ui->actionBadMove,
        ui->actionVeryBadMove,
        ui->actionInterestingMove,
        ui->actionDoubtfulMove,
        ui->actionEven,
        ui->actionGoodForBlack,
        ui->actionVeryGoodForBlack,
        ui->actionGoodForWhite,
        ui->actionVeryGoodForWhite,
        ui->actionUnclear,
        ui->actionHotspot,
        ui->actionSetMoveNumber,
        ui->actionUnsetMoveNumber,
        ui->actionEditNodeName,
        ui->actionWhiteFirst,
        ui->actionRotateSgfClockwise,
        ui->actionFlipSgfHorizontally,
        ui->actionFlipSgfVertically,
        ui->actionEncodingUTF8,
        ui->actionISO8859_1,
        ui->actionISO8859_2,
        ui->actionISO8859_3,
        ui->actionISO8859_4,
        ui->actionISO8859_5,
        ui->actionISO8859_6,
        ui->actionISO8859_7,
        ui->actionISO8859_8,
        ui->actionISO8859_9,
        ui->actionISO8859_10,
        ui->actionISO8859_11,
        ui->actionISO8859_13,
        ui->actionISO8859_14,
        ui->actionISO8859_15,
        ui->actionISO8859_16,
        ui->actionWindows_1250,
        ui->actionWindows_1251,
        ui->actionWindows_1252,
        ui->actionWindows_1253,
        ui->actionWindows_1254,
        ui->actionWindows_1255,
        ui->actionWindows_1256,
        ui->actionWindows_1257,
        ui->actionWindows_1258,
        ui->actionKoi8_R,
        ui->actionKoi8_U,
        ui->actionEncodingGB2312,
        ui->actionEncodingBig5,
        ui->actionEncodingShiftJIS,
        ui->actionEncodingJIS,
        ui->actionEncodingEucJP,
        ui->actionEncodingKorean,

        ui->actionMoveFirst,
        ui->actionFastRewind,
        ui->actionPreviousMove,
        ui->actionNextMove,
        ui->actionFastForward,
        ui->actionMoveLast,
        ui->actionBackToParent,
        ui->actionPreviousBranch,
        ui->actionNextBranch,
        ui->actionJumpToMoveNumber,
        ui->actionJumpToClicked,

        ui->actionShowMoveNumber,
        ui->actionResetMoveNubmerInBranch,
        ui->actionNoMoveNumber,
        ui->actionLast1Move,
        ui->actionLast2Moves,
        ui->actionLast5Moves,
        ui->actionLast10Moves,
        ui->actionLast20Moves,
        ui->actionLast50Moves,
        ui->actionAllMoves,
        ui->actionShowCoordinate,
        ui->actionShowCoordinateI,
        ui->actionShowMarker,
        ui->actionShowBranchMoves,
        ui->actionBranchMode,
        ui->actionRotateBoardClockwise,
        ui->actionFlipBoardHorizontally,
        ui->actionFlipBoardVertically,
        ui->actionResetBoard,

//        ui->actionCountTerritory,
        ui->actionPlayWithGnugo,
        ui->actionAutomaticReplay,
        ui->actionTutorBothSides,
        ui->actionTutorOneSide,
        ui->actionPlaySound,
        ui->action19x19Board,
        ui->action13x13Board,
        ui->action9x9Board,
        ui->actionCustomBoardSize,
        ui->actionLanguageSystemDefault,
        ui->actionLanguageEnglish,
        ui->actionLanguageJapanese,
        ui->actionOptions,

        ui->actionCollectionExtract,
        ui->actionCollectionImport,
        ui->actionCollectionMoveDown,
        ui->actionCollectionMoveUp,
        ui->actionDeleteSgfFromCollection,

//        ui->actionGamePass,
//        ui->actionGameResign,
//        ui->actionGameUndo,

//        ui->actionAbout,
//        ui->actionAboutQT,

        ui->menuRecentFiles->menuAction(),
        ui->menuShowMoveNumber->menuAction(),
        ui->menuStoneMarkers->menuAction(),
        undoAction,
        redoAction,
    };
    static int N = sizeof(act) / sizeof(act[0]);
    QVector<bool>& status = tabDatas[board].countTerritoryMenuStatus;
    if (status.size() < N){
        status.resize(N);
        qFill(status, true);
    }
    if (on)
        for (int i=0; i<N; ++i){
            status[i] = act[i]->isEnabled();
            act[i]->setEnabled( false );
        }
    else
        for (int i=0; i<N; ++i)
            act[i]->setEnabled( status[i] );

    if (board == currentBoard()){
        ui->commentWidget->setEnabled( !on );
        tabDatas[currentBoard()].branchWidget->setEnabled( !on );
        ui->undoView->setEnabled( !on );
        ui->collectionWidget->setEnabled( !on );
    }
}

void MainWindow::setPlayWithComputerMode(BoardWidget* board, bool on){
    playWithComputerMode = on;

    static QAction* act[] = {
//        ui->actionNew,
//        ui->actionOpen,
//        ui->actionOpenDuplicate,
//        ui->actionOpenURL,
        ui->actionSave,
        ui->actionSaveAs,
        ui->actionReload,
        ui->actionSaveBoardAsPicture,
        ui->actionExportAsciiToClipboard,
//        ui->actionCloseTab,
//        ui->actionCloseAllTabs,
        ui->actionPrint,
//        ui->actionExit,

        ui->actionCopySgfToClipboard,
        ui->actionCopyCurrentSgfToClipboard,
        ui->actionPasteSgfToNewTab,
        ui->actionPasteSgfToCollection,
//        ui->actionPasteSgfAsBranchFromClipboard,
        ui->actionGameInformation,
        ui->actionDeleteAfterCurrent,
        ui->actionDeleteOnlyCurrent,
//        ui->actionPass,
        ui->actionAlternateMove,
        ui->actionAddBlackStone,
        ui->actionAddWhiteStone,
        ui->actionAddEmpty,
        ui->actionAddLabel,
        ui->actionAddCircle,
        ui->actionAddCross,
        ui->actionAddSquare,
        ui->actionAddTriangle,
        ui->actionDeleteMarker,
        ui->actionGoodMove,
        ui->actionVeryGoodMove,
        ui->actionBadMove,
        ui->actionVeryBadMove,
        ui->actionInterestingMove,
        ui->actionDoubtfulMove,
        ui->actionEven,
        ui->actionGoodForBlack,
        ui->actionVeryGoodForBlack,
        ui->actionGoodForWhite,
        ui->actionVeryGoodForWhite,
        ui->actionUnclear,
        ui->actionHotspot,
        ui->actionSetMoveNumber,
        ui->actionUnsetMoveNumber,
        ui->actionEditNodeName,
        ui->actionWhiteFirst,
        ui->actionRotateSgfClockwise,
        ui->actionFlipSgfHorizontally,
        ui->actionFlipSgfVertically,
        ui->actionEncodingUTF8,
        ui->actionISO8859_1,
        ui->actionISO8859_2,
        ui->actionISO8859_3,
        ui->actionISO8859_4,
        ui->actionISO8859_5,
        ui->actionISO8859_6,
        ui->actionISO8859_7,
        ui->actionISO8859_8,
        ui->actionISO8859_9,
        ui->actionISO8859_10,
        ui->actionISO8859_11,
        ui->actionISO8859_13,
        ui->actionISO8859_14,
        ui->actionISO8859_15,
        ui->actionISO8859_16,
        ui->actionWindows_1250,
        ui->actionWindows_1251,
        ui->actionWindows_1252,
        ui->actionWindows_1253,
        ui->actionWindows_1254,
        ui->actionWindows_1255,
        ui->actionWindows_1256,
        ui->actionWindows_1257,
        ui->actionWindows_1258,
        ui->actionKoi8_R,
        ui->actionKoi8_U,
        ui->actionEncodingGB2312,
        ui->actionEncodingBig5,
        ui->actionEncodingShiftJIS,
        ui->actionEncodingJIS,
        ui->actionEncodingEucJP,
        ui->actionEncodingKorean,

        ui->actionMoveFirst,
        ui->actionFastRewind,
        ui->actionPreviousMove,
        ui->actionNextMove,
        ui->actionFastForward,
        ui->actionMoveLast,
        ui->actionBackToParent,
        ui->actionPreviousBranch,
        ui->actionNextBranch,
        ui->actionJumpToMoveNumber,
        ui->actionJumpToClicked,

//        ui->actionShowMoveNumber,
//        ui->actionResetMoveNubmerInBranch,
//        ui->actionNoMoveNumber,
//        ui->actionLast1Move,
//        ui->actionLast2Moves,
//        ui->actionLast5Moves,
//        ui->actionLast10Moves,
//        ui->actionLast20Moves,
//        ui->actionLast50Moves,
//        ui->actionAllMoves,
//        ui->actionShowCoordinate,
//        ui->actionShowCoordinateI,
//        ui->actionShowMarker,
//        ui->actionShowBranchMoves,
        ui->actionBranchMode,
        ui->actionRotateBoardClockwise,
        ui->actionFlipBoardHorizontally,
        ui->actionFlipBoardVertically,
        ui->actionResetBoard,

        ui->actionCountTerritory,
//        ui->actionPlayWithGnugo,
        ui->actionAutomaticReplay,
        ui->actionTutorBothSides,
        ui->actionTutorOneSide,
        ui->actionPlaySound,
        ui->action19x19Board,
        ui->action13x13Board,
        ui->action9x9Board,
        ui->actionCustomBoardSize,
        ui->actionLanguageSystemDefault,
        ui->actionLanguageEnglish,
        ui->actionLanguageJapanese,
        ui->actionOptions,

        ui->actionCollectionExtract,
        ui->actionCollectionImport,
        ui->actionCollectionMoveDown,
        ui->actionCollectionMoveUp,
        ui->actionDeleteSgfFromCollection,

//        ui->actionGamePass,
//        ui->actionGameResign,
//        ui->actionGameUndo,

//        ui->actionAbout,
//        ui->actionAboutQT,

        ui->menuRecentFiles->menuAction(),
//        ui->menuShowMoveNumber->menuAction(),
        ui->menuStoneMarkers->menuAction(),

        undoAction,
        redoAction,
    };
    static int N = sizeof(act) / sizeof(act[0]);
    QVector<bool>& status = tabDatas[board].countTerritoryMenuStatus;
    if (status.size() < N){
        status.resize(N);
        qFill(status, true);
    }

    if (on){
        ui->actionPlayWithGnugo->setChecked(true);
        ui->actionGamePass->setEnabled(true);
        ui->actionGameResign->setEnabled(true);
        ui->actionGameUndo->setEnabled(true);
        undoGroup.setActiveStack(0);
        for (int i=0; i<N; ++i){
            status[i] = act[i]->isEnabled();
            act[i]->setEnabled( false );
        }
    }
    else{
        ui->actionPlayWithGnugo->setChecked(false);
        ui->actionGamePass->setEnabled(false);
        ui->actionGameResign->setEnabled(false);
        ui->actionGameUndo->setEnabled(false);
        if (board == currentBoard())
            undoGroup.setActiveStack(board->getUndoStack());
        for (int i=0; i<N; ++i)
            act[i]->setEnabled( status[i] );
    }

    if (board == currentBoard()){
        ui->commentWidget->setEnabled( !on );
        tabDatas[board].branchWidget->setEnabled( !on );
        ui->undoView->setEnabled( !on );
        ui->collectionWidget->setEnabled( !on );
    }
}

void MainWindow::endGame(BoardWidget* board){
    board->playWithComputer(NULL);
    tabDatas[board].playGame = NULL;
    setPlayWithComputerMode(board, false);
}

void MainWindow::alertLanguageChanged(){
    QMessageBox::information(this, APPNAME, tr("Changing the language requires that application be restarted."));
}

QString MainWindow::getDefaultSaveName() const{

#define REPLACE(K, V){\
    if (saveName.indexOf(K) >= 0){\
        saveName.replace(K, V);\
        if (!V.isEmpty())\
            replaced = true;\
    }\
}

    QString saveName = QSettings().value("saveName", SAVE_NAME).toString();

    go::informationPtr i = currentBoard()->getData().root;
    QString date = i->date;
    date.replace("/", "-");

    bool replaced = false;

    REPLACE("%PB%", i->blackPlayer);
    REPLACE("%BR%", i->blackRank);
    REPLACE("%BT%", i->blackTeam);
    REPLACE("%PW%", i->whitePlayer);
    REPLACE("%WR%", i->whiteRank);
    REPLACE("%WT%", i->whiteTeam);
    REPLACE("%DT%", date);
    REPLACE("%GN%", i->name);
    REPLACE("%RO%", i->round);
    REPLACE("%EV%", i->event);
    REPLACE("%PC%", i->place);
    REPLACE("%RE%", i->result);

    if (replaced == false)
        return QString();

    saveName.remove('\\');
    saveName.remove('/');
    saveName.remove(':');
    saveName.remove('<');
    saveName.remove('>');
    saveName.remove('|');
    saveName.remove('"');

    return saveName;
}

QString getOpenFileName(QWidget* parent, const QString& caption, const QString& dir, const QString& filter, QString* selectedFilter, QFileDialog::Options options){
#if defined(Q_WS_X11)
    QFileDialog dlg(parent, caption, dir, filter);
    dlg.setOptions(options);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFile);

    if (dlg.exec() != QDialog::Accepted)
        return QString();

    if (selectedFilter)
        *selectedFilter = dlg.selectedFilter();

    if (dlg.selectedFiles().size() == 0)
        return QString();

    return dlg.selectedFiles()[0];
#else
    return QFileDialog::getOpenFileName(parent, caption, dir, filter, selectedFilter, options);
#endif
}

QString getSaveFileName(QWidget* parent, const QString& caption, const QString& dir, const QString& filter, QString* selectedFilter, QFileDialog::Options options){
#if defined(Q_WS_X11)
    QFileDialog dlg(parent, caption, dir, filter);
    dlg.setOptions(options);
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec() != QDialog::Accepted)
        return QString();

    if (selectedFilter)
        *selectedFilter = dlg.selectedFilter();

    if (dlg.selectedFiles().size() == 0)
        return QString();

    return dlg.selectedFiles()[0];
#else
    return QFileDialog::getSaveFileName(parent, caption, dir, filter, selectedFilter, options);
#endif
}

void MainWindow::on_actionAutomaticReplay_triggered(){
    currentBoard()->autoReplay();
    ui->actionAutomaticReplay->setChecked( currentBoard()->isAutoReplay() );
}

void MainWindow::automaticReplay_ended(){
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    if (currentBoard() == board)
        ui->actionAutomaticReplay->setChecked( false );
}

void MainWindow::readSettings(){
    QSettings settings;

    stepsOfFastMove = settings.value("navigation/stepsOfFastMove", FAST_MOVE_STEPS).toInt();

    for (int i=0; i<ui->boardTabWidget->count(); ++i){
        BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->widget(i));
        if (board){
            board->readSettings();
            board->repaintBoard();
        }
    }
}

bool MainWindow::stopGame(BoardWidget* boardWidget){
    if (tabDatas[boardWidget].playGame == NULL)
        return true;

    QMessageBox::StandardButton ret = QMessageBox::warning(this, APPNAME,
                                                tr("Are you sure you want to stop playing with computer?"),
                                                QMessageBox::Ok|QMessageBox::Cancel);

    if (ret != QMessageBox::Ok)
        return false;

    tabDatas[boardWidget].playGame->abort();

    return true;
}
