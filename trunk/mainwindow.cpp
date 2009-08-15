#include <QSettings>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QtAlgorithms>
#include "appdef.h"
#include "sgf.h"
#include "ugf.h"
#include "mainwindow.h"
#include "gameinformationdialog.h"
#include "setupdialog.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE(go::nodePtr);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , annotation1(go::node::eNoAnnotation)
    , annotation2(go::node::eNoAnnotation)
    , annotation3(go::node::eNoAnnotation)
    , branchMode(false)
    , countTerritoryDialog(NULL)
    , undoGroup(this)
{
    ui->setupUi(this);

    // undo
    undoGroup.setActiveStack(&ui->boardWidget->undoStack);
    ui->undoView->setGroup(&undoGroup);
    ui->undoDockWidget->setVisible(false);

// set sound files
//#ifdef Q_WS_WIN
    QStringList soundPathList;
    soundPathList.push_back(qApp->applicationDirPath() + "/sounds/");
//#elif defined(Q_WS_MAC)
//#elif defined(Q_WS_X11)
//    QStringList soundPathList;
    soundPathList.push_back("/usr/share/" APPNAME "/sounds/");
    soundPathList.push_back("/usr/local/share/" APPNAME "/sounds/");
//#endif
    QStringList::iterator iter = soundPathList.begin();
    while (iter != soundPathList.end()){
        QFileInfo finfo( *iter + "stone.wav" );
        if (finfo.exists()){
            ui->boardWidget->setStoneSoundPath(finfo.filePath());
            break;
        }
        ++iter;
    }
    ui->boardWidget->setPlaySound(true);

    // recent files
    for (int i=0; i<MaxRecentFiles; ++i){
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        ui->menu_File->insertAction(ui->actionExit, recentFileActs[i]);
        connect( recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()) );
    }
    recentSeparator = ui->menu_File->insertSeparator(ui->actionExit);
    updateRecentFileActions();

    // undo, redo action
    QAction* undoAction = undoGroup.createUndoAction(this);
    QAction* redoAction = undoGroup.createRedoAction(this);
    undoAction->setIcon( QIcon(":/res/undo.png") );
    redoAction->setIcon( QIcon(":/res/redo.png") );
    ui->menu_Edit->insertAction(ui->menu_Edit->actions().at(0), redoAction);
    ui->menu_Edit->insertAction(redoAction, undoAction);
    ui->editToolBar->insertAction(ui->editToolBar->actions().at(0), redoAction);
    ui->editToolBar->insertAction(redoAction, undoAction);

    // window menu
    ui->menu_Window->addAction( ui->commentDockWidget->toggleViewAction() );
    ui->menu_Window->addAction( ui->branchDockWidget->toggleViewAction() );
    ui->menu_Window->addAction( ui->undoDockWidget->toggleViewAction() );

    // language menu
    QSettings settings(AUTHOR, APPNAME);
    QString language = settings.value("language").toString();
    if (language.isEmpty())
        ui->actionLanguageSystemDefault->setChecked(true);
    else if (language == "en")
        ui->actionLanguageEnglish->setChecked(true);
    else if (language == "ja_JP")
        ui->actionLanguageJapanese->setChecked(true);

    // tool bar
    ui->optionToolBar->insertAction( ui->optionToolBar->actions().at(0), ui->menuMoveNumber->menuAction() );
    ui->menuMoveNumber->menuAction()->setCheckable(true);
    ui->menuMoveNumber->menuAction()->setChecked( ui->actionShowMoveNumber->isChecked() );
    connect( ui->menuMoveNumber->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionShowMoveNumber_parent_triggered()) );

    setEncoding(ui->actionEncodingUTF8, "UTF-8");
    setShowMoveNumber(ui->actionNoMoveNumber, 0);
    setEditMode(ui->actionAlternateMove, BoardWidget::eAlternateMove);

    // status bar
    moveNumberLabel = new QLabel;
    moveNumberLabel->setFrameStyle(QFrame::StyledPanel|QFrame::Plain);
    moveNumberLabel->setToolTip(tr("Move Number"));
    ui->statusBar->addPermanentWidget(moveNumberLabel, 0);

    capturedLabel = new QLabel;
    capturedLabel->setFrameStyle(QFrame::StyledPanel|QFrame::Plain);
    capturedLabel->setToolTip(tr("Captured"));
    ui->statusBar->addPermanentWidget(capturedLabel, 0);

    // command line
    if (qApp->argc() > 1)
        fileOpen(qApp->argv()[1]);
    else
        fileNew();
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* e){
    if (fileClose())
        e->accept();
    else
        e->ignore();
}

void MainWindow::keyPressEvent(QKeyEvent* event){
    if (event->key() == Qt::Key_Delete)
        deleteNode();
}

/**
* Slot
* File -> New
*/
void MainWindow::on_actionNew_triggered(){
    fileNew();
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
* File -> Reload
*/
void MainWindow::on_actionReload_triggered(){
    if (maybeSave() == false)
        return;
    fileOpen(fileName, filter);
}

/**
* Slot
* File -> Save
*/
void MainWindow::on_actionSave_triggered(){
    fileSave();
}

/**
* Slot
* File -> Save As
*/
void MainWindow::on_actionSaveAs_triggered(){
    fileSaveAs();
}

/**
* Slot
* File -> Save Board As Picture
*/
void MainWindow::on_actionSaveBoardAsPicture_triggered()
{
    QString selectedFilter;
    QString fname = QFileDialog::getSaveFileName(this, QString(), QString(),
        tr("PNG image(*.png);;Bitmap image(*.bmp);;JPEG image(*.jpeg *.jpg);;TIFF image(*.tiff *.tif)"),
        &selectedFilter);
    if (fname.isEmpty())
        return;

    const char* format[] = { "PNG", "BMP", "JPG", "TIFF" };
    int n;

    if (selectedFilter.indexOf("*.png") >= 0)
        n = 0;
    else if (selectedFilter.indexOf("*.bmp") >= 0)
        n = 1;
    else if (selectedFilter.indexOf("*.jpg") >= 0)
        n = 2;
    else if (selectedFilter.indexOf("*.tiff") >= 0)
        n = 3;
    else
        return;

    int w = ui->boardWidget->width();
    int h = ui->boardWidget->height();
    w = h = std::min(w, h);
    QImage image(w, h, QImage::Format_RGB32);
    ui->boardWidget->paintBoard(&image);
    ui->boardWidget->paintStones(&image);
    image.save(fname, format[n]);

    // change image coordinate to display
    ui->boardWidget->repaintBoard();
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
    if (action && maybeSave())
        fileOpen(action->data().toString());
}

/**
* Slot
* Edit -> Game Information
*/
void MainWindow::on_actionGameInformation_triggered(){
    GameInformationDialog dlg(this, ui->boardWidget->getData().root.get());
    if (dlg.exec() != QDialog::Accepted)
        return;

    ui->boardWidget->setDirty(true);
    setCaption();
}

/**
* Slot
* Edit -> Pass
*/
void MainWindow::on_actionPass_triggered(){
    go::nodePtr currentNode = ui->boardWidget->getCurrentNode();
    go::nodePtr node;
    if (currentNode->isBlack())
        node = go::createWhiteNode(currentNode);
    else
        node = go::createBlackNode(currentNode);
    ui->boardWidget->addNode(currentNode, node);
    ui->boardWidget->setCurrentNode(node);
}

/**
* Slot
* Edit -> Delete
*/
void MainWindow::on_actionDelete_triggered(){
    deleteNode();
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
    setAnnotation1(ui->actionGoodMove, go::node::eGoodMove);
}

/**
* Slot
* Edit -> Annotation -> Very Good Move
*/
void MainWindow::on_actionVeryGoodMove_triggered(){
    setAnnotation1(ui->actionVeryGoodMove, go::node::eVeryGoodMove);
}

/**
* Slot
* Edit -> Annotation -> Bad Move
*/
void MainWindow::on_actionBadMove_triggered(){
    setAnnotation1(ui->actionBadMove, go::node::eBadMove);
}

/**
* Slot
* Edit -> Annotation -> Very Bad Move
*/
void MainWindow::on_actionVeryBadMove_triggered(){
    setAnnotation1(ui->actionVeryBadMove, go::node::eVeryBadMove);
}

/**
* Slot
* Edit -> Annotation -> Doubtful Move
*/
void MainWindow::on_actionDoubtfulMove_triggered(){
    setAnnotation1(ui->actionDoubtfulMove, go::node::eDoubtfulMove);
}

/**
* Slot
* Edit -> Annotation -> Interesting Move
*/
void MainWindow::on_actionInterestingMove_triggered(){
    setAnnotation1(ui->actionInterestingMove, go::node::eInterestingMove);
}

/**
* Slot
* Edit -> Annotation -> Even
*/
void MainWindow::on_actionEven_triggered(){
    setAnnotation2(ui->actionEven, go::node::eEven);
}

/**
* Slot
* Edit -> Annotation -> Good for Black
*/
void MainWindow::on_actionGoodForBlack_triggered(){
    setAnnotation2(ui->actionGoodForBlack, go::node::eGoodForBlack);
}

/**
* Slot
* Edit -> Annotation -> Very Good for Black
*/
void MainWindow::on_actionVeryGoodForBlack_triggered(){
    setAnnotation2(ui->actionVeryGoodForBlack, go::node::eVeryGoodForBlack);
}

/**
* Slot
* Edit -> Annotation -> Good for White
*/
void MainWindow::on_actionGoodForWhite_triggered(){
    setAnnotation2(ui->actionGoodForWhite, go::node::eGoodForWhite);
}

/**
* Slot
* Edit -> Annotation -> Very Good for White
*/
void MainWindow::on_actionVeryGoodForWhite_triggered(){
    setAnnotation2(ui->actionVeryGoodForWhite, go::node::eVeryGoodForWhite);
}

/**
* Slot
* Edit -> Annotation -> Unclear
*/
void MainWindow::on_actionUnclear_triggered(){
    setAnnotation2(ui->actionUnclear, go::node::eUnclear);
}

/**
* Slot
* Edit -> Annotation -> Hotspot
*/
void MainWindow::on_actionHotspot_triggered(){
    setAnnotation3(ui->actionHotspot, go::node::eHotspot);
}

/**
* Slot
* Edit -> Move Number -> Set
*/
void MainWindow::on_actionSetMoveNumber_triggered(){
    go::nodePtr node = ui->boardWidget->getCurrentNode();
    if (!node->isStone())
        return;

    QInputDialog dlg(this);
    dlg.setInputMode(QInputDialog::IntInput);
    dlg.setLabelText( tr("Input move number") );

    if (node->moveNumber > 0)
        dlg.setIntValue(node->moveNumber);

    if (dlg.exec() != QDialog::Accepted)
        return;

    ui->boardWidget->setMoveNumberCommand(node, dlg.intValue());
}

/**
* Slot
* Edit -> Move Number -> Unset
*/
void MainWindow::on_actionUnsetMoveNumber_triggered(){
    go::nodePtr node = ui->boardWidget->getCurrentNode();
    ui->boardWidget->unsetMoveNumberCommand(node);
}

/**
* Slot
* Edit -> Edit Node Name
*/
void MainWindow::on_actionEditNodeName_triggered(){
    go::nodePtr node = ui->boardWidget->getCurrentNode();
    QInputDialog dlg(this);
    dlg.setLabelText( tr("Input node name") );
    dlg.setTextValue(node->name);
    if (dlg.exec() != QDialog::Accepted)
        return;
    ui->boardWidget->setNodeNameCommand(node, dlg.textValue());
}

/**
* Slot
* Edit -> Rotate SGF Clockwise
*/
void MainWindow::on_actionRotateSgfClockwise_triggered(){
    ui->boardWidget->rotateSgf();
}

/**
* Slot
* Edit -> Flip SGF Holizontally
*/
void MainWindow::on_actionFlipSgfHorizontally_triggered(){
    ui->boardWidget->flipSgfHorizontally();
}

/**
* Slot
* Edit -> Flip SGF Vertically
*/
void MainWindow::on_actionFlipSgfVertically_triggered(){
    ui->boardWidget->flipSgfVertically();
}

/**
* Slot
* Advance -> Encoding -> UTF-8
*/
void MainWindow::on_actionEncodingUTF8_triggered(){
    setEncoding(ui->actionEncodingUTF8, "UTF-8");
}

/**
* Slot
* Advance -> Encoding -> ISO8859_1
*/
void MainWindow::on_actionISO8859_1_triggered(){
    setEncoding(ui->actionISO8859_1, "ISO-8859-1");
}

/**
* Slot
* Advance -> Encoding -> Windows_1252
*/
void MainWindow::on_actionWindows_1252_triggered(){
    setEncoding(ui->actionWindows_1252, "Windows-1252");
}

/**
* Slot
* Advance -> Encoding -> Chinese Simplified(GB2312)
*/
void MainWindow::on_actionEncodingGB2312_triggered(){
    setEncoding(ui->actionEncodingGB2312, "GB2312");
}

/**
* Slot
* Advance -> Encoding -> Chinese Traditional(Big5)
*/
void MainWindow::on_actionEncodingBig5_triggered(){
    setEncoding(ui->actionEncodingBig5, "Big5");
}

/**
* Slot
* Advance -> Encoding -> Shift_JIS
*/
void MainWindow::on_actionEncodingShiftJIS_triggered(){
    setEncoding(ui->actionEncodingShiftJIS, "Shift_JIS");
}

/**
* Slot
* Advance -> Encoding -> JIS
*/
void MainWindow::on_actionEncodingJIS_triggered(){
    setEncoding(ui->actionEncodingJIS, "ISO 2022-JP");
}

/**
* Slot
* Advance -> Encoding -> EUC-JP
*/
void MainWindow::on_actionEncodingEucJP_triggered(){
    setEncoding(ui->actionEncodingEucJP, "EUC-JP");
}

/**
* Slot
* Advance -> Encoding -> EUC-KR
*/
void MainWindow::on_actionEncodingKorean_triggered(){
    setEncoding(ui->actionEncodingKorean, "EUC-KR");
}

/**
* Slot
* Traverse -> First Move
*/
void MainWindow::on_actionFirstMove_triggered(){
    go::nodePtr node = ui->boardWidget->getCurrentNode();
    ui->boardWidget->setCurrentNode( node->goData->root );
}

/**
* Slot
* Traverse -> Fast Rewind
*/
void MainWindow::on_actionFastRewind_triggered(){
    go::nodePtr node = ui->boardWidget->getCurrentNode();
    if (node->parent == NULL)
        return;

    for (int i=0; i<5; ++i){
        if (node->parent)
            node = node->parent;
        else
            break;
    }
    ui->boardWidget->setCurrentNode(node);
}

/**
* Slot
* Traverse -> Previous Move
*/
void MainWindow::on_actionPreviousMove_triggered(){
    go::nodePtr node = ui->boardWidget->getCurrentNode();
    if (node->parent)
        ui->boardWidget->setCurrentNode(node->parent);
}

/**
* Slot
* Traverse -> Next Move
*/
void MainWindow::on_actionNextMove_triggered(){
    const go::nodeList& nodeList = ui->boardWidget->getCurrentNodeList();
    go::nodeList::const_iterator iter = qFind(nodeList.begin(), nodeList.end(), ui->boardWidget->getCurrentNode());
    if (iter != nodeList.end() && ++iter != nodeList.end())
        ui->boardWidget->setCurrentNode( *iter );
}

/**
* Slot
* Traverse -> Fast Forward
*/
void MainWindow::on_actionFastForward_triggered(){
    go::nodePtr node = ui->boardWidget->getCurrentNode();
    if (node->childNodes.empty())
        return;

    for (int i=0; i<5; ++i){
        if (!node->childNodes.empty())
            node = node->childNodes.front();
        else
            break;
    }
    ui->boardWidget->setCurrentNode(node);
}

/**
* Slot
* Traverse -> Move Last
*/
void MainWindow::on_actionMoveLast_triggered(){
    const go::nodeList& nodeList = ui->boardWidget->getCurrentNodeList();
    go::nodePtr node = nodeList.back();
    ui->boardWidget->setCurrentNode(node);
}

/**
* Slot
* Traverse -> Back to parent
*/
void MainWindow::on_actionBackToParent_triggered(){
    go::nodePtr node = ui->boardWidget->getCurrentNode();
    while(node->parent){
        node = node->parent;
        if (node->childNodes.size() > 1)
            break;
    }
    ui->boardWidget->setCurrentNode( node );
}

/**
* Slot
* Traverse -> Previous Branch
*/
void MainWindow::on_actionPreviousBranch_triggered(){
    go::nodePtr node   = ui->boardWidget->getCurrentNode();
    go::nodePtr parent = node->parent;
    while(parent){
        if (parent->childNodes.size() > 1)
            break;
        node = parent;
        parent = parent->parent;
    }

    if (parent == NULL)
        return;

    go::nodeList::iterator iter = qFind(parent->childNodes.begin(), parent->childNodes.end(), node);
    if (iter == parent->childNodes.begin())
        return;

    ui->boardWidget->setCurrentNode( *--iter );
}

/**
* Slot
* Traverse -> Next Branch
*/
void MainWindow::on_actionNextBranch_triggered(){
    go::nodePtr node   = ui->boardWidget->getCurrentNode();
    go::nodePtr parent = node->parent;
    while(parent){
        if (parent->childNodes.size() > 1)
            break;
        node = parent;
        parent = parent->parent;
    }

    if (parent == NULL)
        return;

    go::nodeList::iterator iter = qFind(parent->childNodes.begin(), parent->childNodes.end(), node);
    if (++iter == parent->childNodes.end())
        return;

    ui->boardWidget->setCurrentNode( *iter );
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

    go::nodePtr node = ui->boardWidget->findNodeFromMoveNumber( dlg.intValue() );
    if (node)
        ui->boardWidget->setCurrentNode(node);
}

/**
* Slot
* Traverse -> jump to clicked
*/
void MainWindow::on_actionJumpToClicked_triggered(){
    ui->boardWidget->setMoveToClicked( ui->actionJumpToClicked->isChecked() );
}

/**
* Slot
* View -> Move Number-> Show Move Number
*/
void MainWindow::on_actionShowMoveNumber_triggered(){
    ui->menuMoveNumber->menuAction()->setChecked( ui->actionShowMoveNumber->isChecked() );
    ui->boardWidget->setShowMoveNumber( ui->actionShowMoveNumber->isChecked() );
}

/**
* Slot
* View -> Move Number-> Show Move Number
*/
void MainWindow::on_actionShowMoveNumber_parent_triggered(){
    ui->actionShowMoveNumber->setChecked( ui->menuMoveNumber->menuAction()->isChecked() );
    on_actionShowMoveNumber_triggered();
}

/**
* Slot
* View -> Move Number-> No Move Number
*/
void MainWindow::on_actionNoMoveNumber_triggered(){
    setShowMoveNumber(ui->actionNoMoveNumber, 0);
}

/**
* Slot
* View -> Move Number-> Last 1 move
*/
void MainWindow::on_actionLast1Move_triggered(){
    setShowMoveNumber(ui->actionLast1Move, 1);
}

/**
* Slot
* View -> Move Number-> Last 2 moves
*/
void MainWindow::on_actionLast2Moves_triggered(){
    setShowMoveNumber(ui->actionLast2Moves, 2);
}

/**
* Slot
* View -> Move Number-> Last 5 moves
*/
void MainWindow::on_actionLast5Moves_triggered(){
    setShowMoveNumber(ui->actionLast5Moves, 5);
}

/**
* Slot
* View -> Move Number-> Last 10 moves
*/
void MainWindow::on_actionLast10Moves_triggered(){
    setShowMoveNumber(ui->actionLast10Moves, 10);
}

/**
* Slot
* View -> Move Number-> Last 20 moves
*/
void MainWindow::on_actionLast20Moves_triggered(){
    setShowMoveNumber(ui->actionLast20Moves, 20);
}

/**
* Slot
* View -> Move Number-> Last 50 moves
*/
void MainWindow::on_actionLast50Moves_triggered(){
    setShowMoveNumber(ui->actionLast50Moves, 50);
}

/**
* Slot
* View -> Move Number-> All Moves
*/
void MainWindow::on_actionAllMoves_triggered(){
    setShowMoveNumber(ui->actionAllMoves, -1);
}

/**
* Slot
* View -> Show Coordinates
*/
void MainWindow::on_actionShowCoordinate_triggered(){
    ui->boardWidget->setShowCoordinates( ui->actionShowCoordinate->isChecked() );
}

/**
* Slot
* View -> Show Coordinates with I
*/
void MainWindow::on_actionShowCoordinateI_triggered(){
    ui->boardWidget->setShowCoordinatesWithI( ui->actionShowCoordinateI->isChecked() );
    setTreeData();
}

/**
* Slot
* View -> Show Marker
*/
void MainWindow::on_actionShowMarker_triggered(){
    ui->boardWidget->setShowMarker( ui->actionShowMarker->isChecked() );
}

/**
* Slot
* View -> Show Branch Moves
*/
void MainWindow::on_actionShowBranchMoves_triggered(){
    ui->boardWidget->setShowBranchMoves( ui->actionShowBranchMoves->isChecked() );
}

/**
* Slot
* View -> Branch Mode
*/
void MainWindow::on_actionBranchMode_triggered(){
    branchMode = ui->actionBranchMode->isChecked();

    setTreeData();
}

/**
* Slot
* View -> Rotate Clockwise
*/
void MainWindow::on_actionRotateBoardClockwise_triggered(){
    ui->actionRotateBoardClockwise->setChecked( ui->boardWidget->rotateBoard() != 0 );
}

/**
* Slot
* View -> Flip Horizontally
*/
void MainWindow::on_actionFlipBoardHorizontally_triggered(){
    ui->boardWidget->flipBoardHorizontally( ui->actionFlipBoardHorizontally->isChecked() );
}

/**
* Slot
* View -> Flip Vertically
*/
void MainWindow::on_actionFlipBoardVertically_triggered(){
    ui->boardWidget->flipBoardVertically( ui->actionFlipBoardVertically->isChecked() );
}

/**
* Slot
* View -> Reset Board
*/
void MainWindow::on_actionResetBoard_triggered(){
    ui->boardWidget->resetBoard();
    ui->actionRotateBoardClockwise->setChecked(false);
    ui->actionFlipBoardHorizontally->setChecked(false);
    ui->actionFlipBoardVertically->setChecked(false);
}

/**
* Slot
* View -> Toolbars -> Main Toolbar
*/
void MainWindow::on_actionMainToolbar_triggered(){
    if (ui->actionMainToolbar->isChecked())
        ui->mainToolBar->show();
    else
        ui->mainToolBar->hide();
}

/**
* Slot
* View -> Toolbars -> Edit Toolbar
*/
void MainWindow::on_actionEditToolbar_triggered(){
    if (ui->actionEditToolbar->isChecked())
        ui->editToolBar->show();
    else
        ui->editToolBar->hide();
}

/**
* Slot
* View -> Toolbars -> Traverse Toolbar
*/
void MainWindow::on_actionNavigationToolbar_triggered(){
    if (ui->actionNavigationToolbar->isChecked())
        ui->navigationToolBar->show();
    else
        ui->navigationToolBar->hide();
}

/**
* Slot
* View -> Toolbars -> Option Toolbar
*/
void MainWindow::on_actionOptionToolbar_triggered()
{
    if (ui->actionOptionToolbar->isChecked())
        ui->optionToolBar->show();
    else
        ui->optionToolBar->hide();
}

/**
* Slot
* Tools -> Count Territoy
*/
void MainWindow::on_actionCountTerritory_triggered(){
    if (ui->actionCountTerritory->isChecked()){
        countTerritoryDialog = new CountTerritoryDialog(this);
        connect(countTerritoryDialog, SIGNAL(dialogClosed()), this, SLOT(scoreDialogClosed()));
        countTerritoryDialog->show();
    }
    else{
        delete countTerritoryDialog;
        countTerritoryDialog = NULL;
    }

    ui->boardWidget->setCountTerritoryMode(ui->actionCountTerritory->isChecked());
}

/**
* Slot
* Options -> Setup
*/
void MainWindow::on_actionSetup_triggered(){
    SetupDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    ui->boardWidget->readSettings();
    ui->boardWidget->repaintBoard();
}

/**
* Slot
* Options -> 19 x 19 Board
*/
void MainWindow::on_action19x19Board_triggered(){
    setBoardSize(19, 19);
}

/**
* Slot
* Options -> 13 x 13 Board
*/
void MainWindow::on_action13x13Board_triggered(){
    setBoardSize(13, 13);
}

/**
* Slot
* Options -> 9 x 9 Board
*/
void MainWindow::on_action9x9Board_triggered(){
    setBoardSize(9, 9);
}

/**
* Slot
* Options -> Custom Board Size
*/
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
            setBoardSize(iW, iH == 0 ? iW : iH);
}

/**
* Slot
* Options -> Play Sound
*/
void MainWindow::on_actionPlaySound_triggered(){
    ui->boardWidget->setPlaySound( ui->actionPlaySound->isChecked() );
}

/**
* Slot
* Options -> Language -> System Default
*/
void MainWindow::on_actionLanguageSystemDefault_triggered(){
    setLanguage( QString(), ui->actionLanguageSystemDefault );
}

/**
* Slot
* Options -> Language -> English
*/
void MainWindow::on_actionLanguageEnglish_triggered(){
    setLanguage("en", ui->actionLanguageEnglish);
}

/**
* Slot
* Options -> Language -> Japanese
*/
void MainWindow::on_actionLanguageJapanese_triggered(){
    setLanguage("ja_JP", ui->actionLanguageJapanese);
}

/**
* Slot
* Help -> About
*/
void MainWindow::on_actionAbout_triggered(){
    QMessageBox::about(this, tr(APPNAME), tr(APPNAME " version " VERSION "\n\nCopyright 2009 Naoya Sase."));
}

/**
* Slot
* Help -> About qt
*/
void MainWindow::on_actionAboutQT_triggered(){
    qApp->aboutQt();
}

/**
* Slot
* new node was created by BoardWidget.
*/
void MainWindow::on_boardWidget_nodeAdded(go::nodePtr /*parent*/, go::nodePtr node, bool /*select*/){
    addTreeWidget(node, true);
    setCaption();
}

/**
* Slot
* node was deleted by BoardWidget.
*/
void MainWindow::on_boardWidget_nodeDeleted(go::nodePtr node, bool /*deleteChildren*/){
    if (node->parent)
        remakeTreeWidget( nodeToTreeWidget[node->parent] );
    deleteTreeWidget(node);
    deleteTreeWidgetForMap(node);

    setCaption();
}

/**
* Slot
*/
void MainWindow::on_boardWidget_nodeModified(go::nodePtr node){
    setCaption();

    QTreeWidgetItem* treeWidget = nodeToTreeWidget[node];
    if (treeWidget == NULL)
        return;
    treeWidget->setText(0, createTreeText(node));
}

/**
* Slot
* current node was changed by BoardWidget.
*/
void MainWindow::on_boardWidget_currentNodeChanged(go::nodePtr node){
    setTreeWidget(node);
    ui->commentWidget->setPlainText(node->comment);
    setAnnotation(node->annotation);

    int b, w;
    ui->boardWidget->getCaptured(b, w);
    capturedLabel->setText(tr("Dead: Black %1 White %2").arg(b).arg(w));

    int num = ui->boardWidget->getMoveNumber();
    QString coord = ui->boardWidget->getXYString(node->getX(), node->getY());
    moveNumberLabel->setText(tr("LastMove: %1(%2)").arg(num).arg(coord));
}

/**
* Slot
* branch dock widget was showed or hid.
*/
void MainWindow::on_branchDockWidget_visibilityChanged(bool visible){
    ui->actionBranchWindow->setChecked(visible);
}

/**
* Slot
* node was changed on branch tree view.
*/
void MainWindow::on_branchWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/){
    if (current == NULL){
        ui->boardWidget->setCurrentNode();
        return;
    }

    QVariant v = current->data(0, Qt::UserRole);
    go::nodePtr n = v.value<go::nodePtr>();

    ui->boardWidget->setCurrentNode(n);
}

/**
* Slot
* comment dock widget was showed or hid.
*/
void MainWindow::on_boardWidget_updateTerritory(int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory, double komi){
    double bscorej = blackTerritory + dead_w + capturedWhite;
    double wscorej = whiteTerritory + dead_b + capturedBlack + komi;

    // japanese rule
    QString bj( tr("Black: %1 = %2(territories) + %3(captured)").arg(bscorej).arg(blackTerritory).arg(dead_w + capturedWhite) );
    QString wj;
    if (komi != 0.0)
        wj = tr("White: %1 = %2(territories) + %3(captured) + %4(komi)").arg(wscorej).arg(whiteTerritory).arg(dead_b + capturedBlack).arg(komi);
    else
        wj = tr("White: %1 = %2(territories) + %3(captured)").arg(wscorej).arg(whiteTerritory).arg(dead_b + capturedBlack);

    QString result;
    if (wscorej > bscorej)
        result = QString(tr("W+%1")).arg(wscorej - bscorej);
    else if (bscorej > wscorej)
        result = QString(tr("B+%1")).arg(bscorej - wscorej);
    else
        result = tr("Draw");

    QString s = tr("Japanese Rule") + ":\n" + wj + "\n" + bj + "\n" + result + "\n\n";


    // chinese rule
    double half = (blackTerritory + alive_b + whiteTerritory + alive_w) / 2.0;
    double bscorec = blackTerritory + alive_b - komi / 2.0;
    double wscorec = whiteTerritory + alive_w + komi / 2.0;

    QString bc, wc;
    if (komi > 0){
        bc = tr("Black: %1 = %2(point) - %3(komi) / 2").arg(bscorec).arg(blackTerritory + alive_b).arg(komi);
        wc = tr("White: %1 = %2(point) + %3(komi) / 2").arg(wscorec).arg(whiteTerritory + alive_w).arg(komi);
    }
    else{
        bc = tr("Black: %1 = %2(point) + %3(komi) / 2").arg(bscorec).arg(blackTerritory + alive_b).arg(komi);
        wc = tr("White: %1 = %2(point) - %3(komi) / 2").arg(wscorec).arg(whiteTerritory + alive_w).arg(komi);
    }

    if (wscorec > bscorec)
        result = QString(tr("W+%1")).arg(wscorec - half);
    else if (bscorej > wscorej)
        result = QString(tr("B+%1")).arg(bscorec - half);
    else
        result = tr("Draw");

    s += tr("Chinese Rule") + ":\n" + wc + "\n" + bc + "\n" + result;

    countTerritoryDialog->setScoreText(s);
}

/**
* Slot
* comment dock widget was showed or hid.
*/
void MainWindow::on_commentDockWidget_visibilityChanged(bool visible){
    ui->actionCommentWindow->setChecked(visible);
}

/**
* Slot
* comment was modified.
*/
void MainWindow::on_commentWidget_textChanged()
{
    go::nodePtr currentNode = ui->boardWidget->getCurrentNode();
    ui->boardWidget->setCommentCommand(currentNode, ui->commentWidget->toPlainText());
}

/**
* Slot
* score dialog was closed
*/
void MainWindow::scoreDialogClosed(){
    ui->actionCountTerritory->setChecked(false);
    on_actionCountTerritory_triggered();
}

/**
* set text to MainWindow's title bar
*
* if game information has player info, set player name to window text.
*/
void MainWindow::setCaption(){
    QString caption;
    if(fileName.isEmpty())
        caption = "Untitled";
    else{
        QFileInfo info(fileName);
        caption = info.fileName();
    }

    if (ui->boardWidget->isDirty())
        caption.append(" *");

    caption.append(" - ");

    go::informationPtr gameInfo = ui->boardWidget->getData().root;
    bool hasPlayerInfo = !gameInfo->whitePlayer.isEmpty() || !gameInfo->whiteRank.isEmpty() ||
                            !gameInfo->blackPlayer.isEmpty() || !gameInfo->blackRank.isEmpty();
    if (hasPlayerInfo){
        caption.append(gameInfo->whitePlayer);
        caption.push_back(' ');
        caption.append(gameInfo->whiteRank);
        caption.append(" (W) vs ");
        caption.append(gameInfo->blackPlayer);
        caption.push_back(' ');
        caption.append(gameInfo->blackRank);
        caption.append(" (B) (Result:");
        caption.append(gameInfo->result);
        caption.append(") - ");
    }
    caption.append(APPNAME);

    setWindowTitle(caption);
}

/**
* a new document is created if current document can be closed.
*/
bool MainWindow::fileNew(){
    return fileClose();
}

/**
* file open.
*/
bool MainWindow::fileOpen(){
    if (maybeSave() == false)
        return false;

    QString selectedFilter;
    QString fname = QFileDialog::getOpenFileName(this, QString(), QString(), tr("All Go Format(*.sgf *.ugf *.ugi);;sgf(*.sgf);;ugf(*.ugf *.ugi)"), &selectedFilter);
    if (fname.isEmpty())
        return  false;

    if (selectedFilter.indexOf("All Go Format") >= 0){
        QFileInfo info(fname);
        selectedFilter = info.suffix();
    }

    if (selectedFilter.indexOf("sgf") >= 0)
        return fileOpen(fname, "sgf");
    else if (selectedFilter.indexOf("ugf") >= 0 || selectedFilter.indexOf("ugi") >= 0)
        return fileOpen(fname, "ugf");
    else
        return false;
}

/**
* file open.
*/
bool MainWindow::fileOpen(const QString& fname){
    return fileOpen(fname, QFileInfo(fname).suffix());
}

/**
* file open.
*/
bool MainWindow::fileOpen(const QString& fname, const QString& filter){
    setCurrentFile(fname);
    this->filter = filter;

    if (filter == "sgf"){
        go::sgf sgf;
        sgf.read(fname, codec);
        ui->boardWidget->setData(sgf);
    }
    else if (filter == "ugf" || filter == "ugi"){
        go::ugf ugf;
        ugf.read(fname, codec);
        ui->boardWidget->setData(ugf);
    }
    else
        return false;

    ui->boardWidget->setDirty(false);

    setTreeData();
    setCaption();

    ui->actionReload->setEnabled(true);

    return true;
}

/**
* file save.
*/
bool MainWindow::fileSave(){
    if (fileName.isEmpty())
        return fileSaveAs();
    else
        return fileSaveAs(fileName);
}

/**
* file saveas.
*/
bool MainWindow::fileSaveAs(){
    QString fname = QFileDialog::getSaveFileName(this, QString(), QString(), tr("sgf(*.sgf)"));
    if (fname.isEmpty())
        return false;

    return fileSaveAs(fname);
}

/**
* file saveas.
*/
bool MainWindow::fileSaveAs(const QString& fname){
    fileName = fname;
    go::sgf sgf;
    ui->boardWidget->getData(sgf);
    sgf.save(fileName, codec);
    ui->boardWidget->setDirty(false);
    filter = "sgf";

    setCaption();

    ui->actionReload->setEnabled(true);

    return true;
}

/**
* file close.
*/
bool MainWindow::fileClose(){
    if (maybeSave() == false)
        return false;

    fileName.clear();
    filter = "sgf";
    ui->boardWidget->clear();

    setTreeData();
    setCaption();

    ui->actionReload->setEnabled(false);

    return true;
}

/**
*/
bool MainWindow::maybeSave(){
    if (!ui->boardWidget->isDirty())
        return true;
    QMessageBox::StandardButton ret =
    QMessageBox::warning(this, tr(APPNAME),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

/**
*/
void MainWindow::setTreeData(){
    ui->branchWidget->clear();
    nodeToTreeWidget.clear();

    addTreeWidget(ui->boardWidget->getData().root);
    ui->boardWidget->setCurrentNode();
    ui->boardWidget->repaint();
}

QTreeWidgetItem* MainWindow::addTreeWidget(go::nodePtr node, bool needRemake){
    QTreeWidgetItem* treeWidget = nodeToTreeWidget[node];
    QTreeWidgetItem* newWidget  = NULL;
    if (treeWidget == NULL)
        newWidget = createTreeWidget(node);
    QTreeWidgetItem* parentWidget  = (node->parent && nodeToTreeWidget[node->parent]) ? nodeToTreeWidget[node->parent] : ui->branchWidget->invisibleRootItem();
    QTreeWidgetItem* parentWidget2 = parentWidget->parent() ? parentWidget->parent() : ui->branchWidget->invisibleRootItem();
    go::nodePtr parentNode  = node->parent;
    go::nodePtr parentNode2 = getNode(parentWidget2);

    bool newBranch = (parentNode2 && parentNode2->childNodes.size() > 1) ||
                     (branchMode && parentNode && parentNode->childNodes.size() > 1) ||
                     (!branchMode && parentNode && parentNode->childNodes.front() != node);
    if (newBranch){
        if (newWidget){
            parentWidget->addChild(newWidget);
            if (needRemake)
                remakeTreeWidget(parentWidget);
        }
        else if(parentWidget->indexOfChild(treeWidget) < 0){
            QTreeWidgetItem* p = treeWidget->parent() ? treeWidget->parent() : ui->branchWidget->invisibleRootItem();
            p->removeChild(treeWidget);
            parentWidget->addChild(treeWidget);
        }
    }
    else{
        if (newWidget){
            int index = parentWidget2->indexOfChild(parentWidget);
            parentWidget2->insertChild(index+1, newWidget);
            if (needRemake)
                remakeTreeWidget(parentWidget2);
        }
        else if(parentWidget2->indexOfChild(treeWidget) < 0){
            QTreeWidgetItem* p = treeWidget->parent() ? treeWidget->parent() : ui->branchWidget->invisibleRootItem();
            p->removeChild(treeWidget);
            parentWidget2->addChild(treeWidget);
        }
    }

    go::nodeList::iterator iter = node->childNodes.begin();
    while (iter != node->childNodes.end()){
        addTreeWidget(*iter);
        ++iter;
    }

    return treeWidget;
}

QTreeWidgetItem* MainWindow::createTreeWidget(go::nodePtr node){
    // TreeItemを作成
    QTreeWidgetItem* nodeWidget = new QTreeWidgetItem( QStringList(createTreeText(node)) );

    QVariant v;
    v.setValue(node);
    nodeWidget->setData(0, Qt::UserRole, v);
    nodeToTreeWidget[node] = nodeWidget;

    // TreeItemにIconを設定
    if (node->isStone() && node->isBlack())
        nodeWidget->setIcon(0, QIcon(":/res/black_64.png"));
    else if (node->isStone() && node->isWhite())
        nodeWidget->setIcon(0, QIcon(":/res/white_64.png"));
    else
        nodeWidget->setIcon(0, QIcon(":/res/green_64.png"));

    return nodeWidget;
}

QTreeWidgetItem* MainWindow::remakeTreeWidget(QTreeWidgetItem* treeWidget){
    go::nodePtr node = getNode(treeWidget);
    if (node == NULL)
        return NULL;
    go::nodeList::iterator iter = node->childNodes.begin();
    while (iter != node->childNodes.end()){
        addTreeWidget(*iter);
        ++iter;
    }
    return NULL;
}

void MainWindow::deleteNode(){
    ui->boardWidget->deleteNodeCommand( ui->boardWidget->getCurrentNode() );
}

void MainWindow::deleteTreeWidget(go::nodePtr node){
    QTreeWidgetItem* treeWidget = nodeToTreeWidget[node];
    go::nodeList::iterator iter = node->childNodes.begin();
    while (iter != node->childNodes.end()){
        QTreeWidgetItem* treeWidget2 = nodeToTreeWidget[*iter];
        if (treeWidget2->parent() != treeWidget)
            deleteTreeWidget(*iter);
        ++iter;
    }
    delete treeWidget;
}

void MainWindow::deleteTreeWidgetForMap(go::nodePtr node){
    NodeToTreeWidgetType::iterator iter = nodeToTreeWidget.find(node);
    if (iter != nodeToTreeWidget.end())
        nodeToTreeWidget.erase(iter);

    go::nodeList::iterator iter2 = node->childNodes.begin();
    while (iter2 != node->childNodes.end()){
        deleteTreeWidgetForMap(*iter2);
        ++iter2;
    }
}

void MainWindow::setTreeWidget(go::nodePtr n){
    NodeToTreeWidgetType::iterator iter = nodeToTreeWidget.find(n);
    if (iter != nodeToTreeWidget.end())
        ui->branchWidget->setCurrentItem( iter.value() );
}

QString MainWindow::createTreeText(const go::nodePtr node){
    QString s;
    if (node->isStone()){
        if (node->isPass())
            s.append( tr("Pass") );
        else
            s.append( ui->boardWidget->getXYString(node->getX(), node->getY()) );
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

void MainWindow::setEncoding(QAction* action, const char* codecName){
    static QAction* actions[] = {
        ui->actionEncodingUTF8,
        ui->actionISO8859_1,
        ui->actionWindows_1252,
        ui->actionEncodingGB2312,
        ui->actionEncodingBig5,
        ui->actionEncodingKorean,
        ui->actionEncodingEucJP,
        ui->actionEncodingJIS,
        ui->actionEncodingShiftJIS,
    };
    static const int N = sizeof(actions) / sizeof(actions[0]);

    for (int i=0; i<N; ++i)
        actions[i]->setChecked(actions[i] == action);

    codec = QTextCodec::codecForName(codecName);
}

void MainWindow::setShowMoveNumber(QAction* action, int moveNumber){
    static QAction* actions[] = {
        ui->actionNoMoveNumber,
        ui->actionLast1Move,
        ui->actionLast2Moves,
        ui->actionLast5Moves,
        ui->actionLast10Moves,
        ui->actionLast20Moves,
        ui->actionLast50Moves,
        ui->actionAllMoves
    };
    static const int N = sizeof(actions) / sizeof(actions[0]);

    for (int i=0; i<N; ++i)
        actions[i]->setChecked(actions[i] == action);

    ui->boardWidget->setShowMoveNumber(moveNumber);
}

void MainWindow::setEditMode(QAction* action, BoardWidget::eEditMode editMode){
   static QAction* actions[] = {
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
    };
    static const int N = sizeof(actions) / sizeof(actions[0]);

    for (int i=0; i<N; ++i)
        actions[i]->setChecked(actions[i] == action);

    ui->boardWidget->setEditMode(editMode);
}

void MainWindow::setAnnotation(int annotation){
   static QAction* actions[] = {
        ui->actionGoodMove,
        ui->actionVeryGoodMove,
        ui->actionBadMove,
        ui->actionVeryBadMove,
        ui->actionDoubtfulMove,
        ui->actionInterestingMove,
        ui->actionEven,
        ui->actionGoodForBlack,
        ui->actionVeryGoodForBlack,
        ui->actionGoodForWhite,
        ui->actionVeryGoodForWhite,
        ui->actionUnclear,
        ui->actionHotspot,
    };
    static const int N = sizeof(actions) / sizeof(actions[0]);

    for (int i=0; i<N; ++i)
        actions[i]->setChecked( (annotation & (1 << i)) != 0 );

    annotation1 = annotation & 0x0000003f;
    annotation2 = annotation & 0x00000fc0;
    annotation3 = annotation & 0x00001000;
}

void MainWindow::setAnnotation1(QAction* action, int annotation){
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

    annotation1 = action->isChecked() ? annotation : 0;
    ui->boardWidget->setAnnotation(annotation1 | annotation2 | annotation3);
}

void MainWindow::setAnnotation2(QAction* action, int annotation){
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

    annotation2 = action->isChecked() ? annotation : 0;
    ui->boardWidget->setAnnotation(annotation1 | annotation2 | annotation3);
}

void MainWindow::setAnnotation3(QAction* action, int annotation){
    annotation3 = action->isChecked() ? annotation : 0;
    ui->boardWidget->setAnnotation(annotation1 | annotation2 | annotation3);
}

void MainWindow::setBoardSize(int xsize, int ysize){
    if (fileNew() == false)
        return;
    ui->boardWidget->setBoardSize(xsize, ysize);
}

void MainWindow::setCurrentFile(const QString& fname){
    fileName = fname;

    QSettings settings(AUTHOR, APPNAME);
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();
    settings.setValue("recentFileList", files);

    updateRecentFileActions();
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings(AUTHOR, APPNAME);
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

    recentSeparator->setVisible(numRecentFiles > 0);
}

void MainWindow::setLanguage(const QString& locale, QAction* act){
    QSettings settings(AUTHOR, APPNAME);
    settings.setValue("language", locale);

    QAction* actions[] = {
        ui->actionLanguageSystemDefault,
        ui->actionLanguageEnglish,
        ui->actionLanguageJapanese,
    };
    const int N = sizeof(actions) / sizeof(actions[0]);

    for (int i=0; i<N; ++i){
        actions[i]->setChecked( actions[i] == act );
    }
}
