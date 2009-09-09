#include <QSettings>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QtAlgorithms>
#include <QClipboard>
#include <QUrl>
#include <QPrinter>
#include <QPrintDialog>
#include "appdef.h"
#include "sgf.h"
#include "ugf.h"
#include "mainwindow.h"
#include "setupdialog.h"
#include "gameinformationdialog.h"
#include "playwithcomputerdialog.h"
#include "exportasciidialog.h"
#include "printoptiondialog.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE(go::nodePtr);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , annotation(go::node::eNoAnnotation)
    , moveAnnotation(go::node::eNoAnnotation)
    , nodeAnnotation(go::node::eNoAnnotation)
    , branchMode(false)
    , countTerritoryDialog(NULL)
    , undoGroup(this)
    , countTerritoryMode(false)
    , playWithComputerMode(false)
{
    ui->setupUi(this);

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
    QAction* encodingActions[] = {
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
        ui->actionWindows_1257,
        ui->actionWindows_1254,
        ui->actionWindows_1258,
        ui->actionWindows_1256,
        ui->actionWindows_1255,
        ui->actionKoi8_R,
        ui->actionKoi8_U,
        ui->actionEncodingGB2312,
        ui->actionEncodingBig5,
        ui->actionEncodingKorean,
        ui->actionEncodingEucJP,
        ui->actionEncodingJIS,
        ui->actionEncodingShiftJIS,
    };
    int encN = sizeof(encodingActions) / sizeof(encodingActions[0]);
    for (int i=0; i<encN; ++i){
        encodingGroup->addAction( encodingActions[i] );
        connect( encodingActions[i], SIGNAL(triggered()), this, SLOT(setEncoding()) );
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

    // default codec
    codec = QTextCodec::codecForName("UTF-8");

    // edit mode
    setEditMode(ui->actionAlternateMove, BoardWidget::eAlternateMove);

    // window settings
    setGeometry(x(), y(), settings.value("width", WIN_W).toInt(), settings.value("height", WIN_H).toInt());

    // for open URL
    http = new QHttp(this);
    connect( http, SIGNAL(readyRead(const QHttpResponseHeader&)), this, SLOT(openUrlReadReady(const QHttpResponseHeader&)) );
    connect( http, SIGNAL(dataReadProgress(int, int)), this, SLOT(openUrlReadProgress(int, int)) );
    connect( http, SIGNAL(done(bool)), this, SLOT(openUrlDone(bool)) );

    // marker
    ui->actionShowMoveNumber->setChecked( settings.value("marker/showMoveNumber", true).toBool() );
    on_actionShowMoveNumber_triggered();

    int moveNumber = settings.value("marker/moveNumber", 0).toInt();
    if (moveNumber == -1){
        ui->actionAllMoves->setChecked(true);
        on_actionAllMoves_triggered();
    }
    else if (moveNumber == 0){
        ui->actionNoMoveNumber->setChecked(true);
        on_actionNoMoveNumber_triggered();
    }
    else if (moveNumber == 1){
        ui->actionLast1Move->setChecked(true);
        on_actionLast1Move_triggered();
    }
    else if (moveNumber == 2){
        ui->actionLast2Moves->setChecked(true);
        on_actionLast2Moves_triggered();
    }
    else if (moveNumber == 5){
        ui->actionLast5Moves->setChecked(true);
        on_actionLast5Moves_triggered();
    }
    else if (moveNumber == 10){
        ui->actionLast10Moves->setChecked(true);
        on_actionLast10Moves_triggered();
    }
    else if (moveNumber == 20){
        ui->actionLast20Moves->setChecked(true);
        on_actionLast20Moves_triggered();
    }
    else if (moveNumber == 50){
        ui->actionLast50Moves->setChecked(true);
        on_actionLast50Moves_triggered();
    }

    // set sound files
    ui->actionPlaySound->setChecked( settings.value("sound/play").toBool() );
    ui->boardWidget->setPlaySound( ui->actionPlaySound->isChecked() );

    // recent files
    for (int i=0; i<MaxRecentFiles; ++i){
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        ui->menuRecentFiles->addAction(recentFileActs[i]);
        connect( recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()) );
    }
    updateRecentFileActions();

    // create undo/redo actions
    // undo
    undoGroup.setActiveStack(ui->boardWidget->getUndoStack());
    ui->undoView->setGroup(&undoGroup);
    ui->undoDockWidget->setVisible(false);

    undoAction = undoGroup.createUndoAction(this);
    redoAction = undoGroup.createRedoAction(this);
    undoAction->setIcon( QIcon(":/res/undo.png") );
    redoAction->setIcon( QIcon(":/res/redo.png") );
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), redoAction);
    ui->menuEdit->insertAction(redoAction, undoAction);
    ui->editToolBar->insertAction(ui->editToolBar->actions().at(0), redoAction);
    ui->editToolBar->insertAction(redoAction, undoAction);

    // create window menu
    ui->menuWindow->addAction( ui->commentDockWidget->toggleViewAction() );
    ui->menuWindow->addAction( ui->branchDockWidget->toggleViewAction() );
    ui->menuWindow->addAction( ui->undoDockWidget->toggleViewAction() );

    // language menu
    QString language = settings.value("language").toString();
    if (language.isEmpty())
        ui->actionLanguageSystemDefault->setChecked(true);
    else if (language == "en")
        ui->actionLanguageEnglish->setChecked(true);
    else if (language == "ja_JP")
        ui->actionLanguageJapanese->setChecked(true);

    // tool bar (option -> show move number)
    ui->optionToolBar->insertAction( ui->optionToolBar->actions().at(0), ui->menuShowMoveNumber->menuAction() );
    ui->menuShowMoveNumber->menuAction()->setCheckable(true);
    ui->menuShowMoveNumber->menuAction()->setChecked( ui->actionShowMoveNumber->isChecked() );
    connect( ui->menuShowMoveNumber->menuAction(), SIGNAL(triggered()), this, SLOT(on_actionShowMoveNumber_parent_triggered()) );

    // tool bar (edit -> stone & marker)
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

    // keyboard shortcut
    ui->actionNew->setShortcut( QKeySequence::New );
    ui->actionOpen->setShortcut( QKeySequence::Open );
    ui->actionSave->setShortcut( QKeySequence::Save );
    ui->actionSaveAs->setShortcut( QKeySequence::SaveAs );
    ui->actionPrint->setShortcut( QKeySequence::Print );
    undoAction->setShortcut( QKeySequence::Undo );
    redoAction->setShortcut( QKeySequence::Redo );
    ui->actionPreviousMove->setShortcut( QKeySequence::Back );
    ui->actionNextMove->setShortcut( QKeySequence::Forward );
    ui->actionPreviousBranch->setShortcut( QKeySequence::PreviousChild );
    ui->actionNextBranch->setShortcut( QKeySequence::NextChild );
    ui->actionMoveFirst->setShortcut( QKeySequence::MoveToStartOfLine );
    ui->actionMoveLast->setShortcut( QKeySequence::MoveToEndOfLine );
//    ui->actionFastRewind->setShortcut( QKeySequence::MoveToPreviousPage );
//    ui->actionFastForward->setShortcut( QKeySequence::MoveToNextPage );

    // count territory dialog
    countTerritoryDialog = new CountTerritoryDialog(this);
    connect(countTerritoryDialog, SIGNAL(finished(int)), this, SLOT(scoreDialogClosed(int)));

    // command line
    if (qApp->argc() > 1)
        fileOpen(qApp->argv()[1]);
    else
        fileNew();
}

MainWindow::~MainWindow(){
    QSettings settings;
    settings.setValue("width", frameSize().width());
    settings.setValue("height", frameSize().height());

    delete countTerritoryDialog;
    delete ui;
    delete http;
}

void MainWindow::closeEvent(QCloseEvent* e){
    if (fileClose())
        e->accept();
    else
        e->ignore();
}

void MainWindow::keyPressEvent(QKeyEvent* event){
    if (event->key() == Qt::Key_Delete)
        deleteNode(true);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
    else{
        QString localFile = event->mimeData()->urls().front().toLocalFile();
        QFileInfo f(localFile);
        if (f.suffix().compare("sgf", Qt::CaseInsensitive) == 0 || f.suffix().compare("ugf", Qt::CaseInsensitive) == 0 || f.suffix().compare("ugi", Qt::CaseInsensitive) == 0)
            event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    event->acceptProposedAction();
    fileOpen( event->mimeData()->urls().front().toLocalFile() );
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

void MainWindow::on_actionOpen_URL_triggered(){
    downloadBuff.clear();

    if (fileClose() == false)
        return;

    QInputDialog dlg(this);
    dlg.resize( 400, dlg.size().height() );
    dlg.setLabelText( tr("Enter the URL of a SGF file.") );
    if (dlg.exec() != QDialog::Accepted)
        return;

    QUrl url( dlg.textValue() );

    QHttp::ConnectionMode mode = url.scheme().toLower() == "https" ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;
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
}

/**
* Slot
* File -> Reload
*/
void MainWindow::on_actionReload_triggered(){
    fileOpen(fileName);
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
    ui->boardWidget->paintTerritories(&image);
    image.save(fname, format[n]);

    // change image coordinate to display
    ui->boardWidget->repaintBoard();
}

/**
* Slot
* File -> Export Ascii to Clipboard
*/
void MainWindow::on_actionExportAsciiToClipboard_triggered(){
    ExportAsciiDialog dlg(this, ui->boardWidget->getBuffer());
    dlg.exec();
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

    ui->boardWidget->print(printer, optionDialog.printOption());
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
void MainWindow::on_actionCopySGFtoClipboard_triggered(){
    QString str;
    QTextStream stream(&str, QIODevice::WriteOnly);

    go::sgf sgf;
    sgf.set(ui->boardWidget->getData());
    sgf.saveStream(stream);
    stream.flush();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(str);
}

/**
* Slot
* Edit -> Copy Current SGF to Clipboard
*/
void MainWindow::on_actionCopyCurrentSGFtoClipboard_triggered(){
    QString str("(");
    QString s;

    const go::nodeList& nodeList = ui->boardWidget->getCurrentNodeList();
    go::nodeList::const_iterator iter = nodeList.begin();
    while (iter != nodeList.end()){
        if (s.size() > 60){
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
* Edit -> Paste SGF from Clipboard
*/
void MainWindow::on_actionPasteSGFfromClipboard_triggered(){
    if (fileClose() == false)
        return;

    QClipboard *clipboard = QApplication::clipboard();
    QString str = clipboard->text();

    go::sgf sgf;
    QString::iterator first = str.begin();
    sgf.readStream(first, str.end());

    ui->boardWidget->setData(sgf);

    ui->boardWidget->setDirty(true);
    setTreeData();
    setCaption();
}

/**
* Slot
* Edit -> Paste SGF as Branch from Clipboard
*/
void MainWindow::on_actionPasteSGFasBranchfromClipboard_triggered(){
    QClipboard *clipboard = QApplication::clipboard();
    QString str = clipboard->text();

    go::sgf sgf;
    QString::iterator first = str.begin();
    sgf.readStream(first, str.end());

    ui->boardWidget->insertData(ui->boardWidget->getCurrentNode(), sgf);

    setTreeData();
    setCaption();
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
    ui->boardWidget->pass();
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
* Edit -> White First
*/
void MainWindow::on_actionWhiteFirst_triggered(){
    if (ui->actionWhiteFirst->isChecked())
        ui->boardWidget->whiteFirst(true);
    else
        ui->boardWidget->whiteFirst(false);
}

/**
* Slot
* Edit -> Rotate SGF Clockwise
*/
void MainWindow::on_actionRotateSgfClockwise_triggered(){
    ui->boardWidget->rotateSgfCommand();
}

/**
* Slot
* Edit -> Flip SGF Holizontally
*/
void MainWindow::on_actionFlipSgfHorizontally_triggered(){
    ui->boardWidget->flipSgfHorizontallyCommand();
}

/**
* Slot
* Edit -> Flip SGF Vertically
*/
void MainWindow::on_actionFlipSgfVertically_triggered(){
    ui->boardWidget->flipSgfVerticallyCommand();
}

/**
* Slot
* Edit -> Encoding
*/
void MainWindow::setEncoding(){
    const QAction* act[] = {
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
    };
    const char* str[] = {
        "UTF-8",
        "ISO-8859-1",
        "ISO-8859-2",
        "ISO-8859-3",
        "ISO-8859-4",
        "ISO-8859-5",
        "ISO-8859-6",
        "ISO-8859-7",
        "ISO-8859-8",
        "ISO-8859-9",
        "ISO-8859-10",
        "ISO-8859-11",
        "ISO-8859-13",
        "ISO-8859-14",
        "ISO-8859-15",
        "ISO-8859-16",
        "Windows-1250",
        "Windows-1251",
        "Windows-1252",
        "Windows-1253",
        "Windows-1254",
        "Windows-1255",
        "Windows-1256",
        "Windows-1257",
        "Windows-1258",
        "KOI8-R",
        "KOI8-U",
        "GB2312",
        "Big5",
        "Shift_JIS",
        "ISO 2022-JP",
        "EUC-JP",
        "EUC-KR",
    };
    int N = sizeof(act) / sizeof(act[0]);
    QAction *action = qobject_cast<QAction*>(sender());
    for (int i=0; i<N; ++i){
        if (act[i] == action){
            codec = QTextCodec::codecForName( str[i] );

            if (codec == NULL)
                qDebug() << "codec is null";
            else
                qDebug() << "change codec to " << codec->name();

            return;
        }
    }
}

/**
* Slot
* Traverse -> First Move
*/
void MainWindow::on_actionMoveFirst_triggered(){
    ui->boardWidget->setCurrentNode( ui->boardWidget->getData().root );
}

/**
* Slot
* Traverse -> Fast Rewind
*/
void MainWindow::on_actionFastRewind_triggered(){
    go::nodePtr node = ui->boardWidget->getCurrentNode();
    if (node->parent == NULL)
        return;

    for (int i=0; i<10; ++i){
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
    const go::nodeList& nodeList = ui->boardWidget->getCurrentNodeList();
    go::nodeList::const_iterator iter = qFind(nodeList.begin(), nodeList.end(), ui->boardWidget->getCurrentNode());

    go::nodePtr node = ui->boardWidget->getCurrentNode();
    for (int i=0; i<10; ++i)
        if (iter != nodeList.end() && ++iter != nodeList.end())
            node = *iter;
        else
            break;

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
    ui->menuShowMoveNumber->menuAction()->setChecked( ui->actionShowMoveNumber->isChecked() );
    ui->boardWidget->setShowMoveNumber( ui->actionShowMoveNumber->isChecked() );

    QSettings settings;
    settings.setValue("marker/showMoveNumber", ui->actionShowMoveNumber->isChecked());
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
    ui->boardWidget->setShowMoveNumber(0);

    QSettings settings;
    settings.setValue("marker/moveNumber", 0);
}

/**
* Slot
* View -> Move Number-> Last 1 move
*/
void MainWindow::on_actionLast1Move_triggered(){
    ui->boardWidget->setShowMoveNumber(1);

    QSettings settings;
    settings.setValue("marker/moveNumber", 1);
}

/**
* Slot
* View -> Move Number-> Last 2 moves
*/
void MainWindow::on_actionLast2Moves_triggered(){
    ui->boardWidget->setShowMoveNumber(2);

    QSettings settings;
    settings.setValue("marker/moveNumber", 2);
}

/**
* Slot
* View -> Move Number-> Last 5 moves
*/
void MainWindow::on_actionLast5Moves_triggered(){
    ui->boardWidget->setShowMoveNumber(5);

    QSettings settings;
    settings.setValue("marker/moveNumber", 5);
}

/**
* Slot
* View -> Move Number-> Last 10 moves
*/
void MainWindow::on_actionLast10Moves_triggered(){
    ui->boardWidget->setShowMoveNumber(10);

    QSettings settings;
    settings.setValue("marker/moveNumber", 10);
}

/**
* Slot
* View -> Move Number-> Last 20 moves
*/
void MainWindow::on_actionLast20Moves_triggered(){
    ui->boardWidget->setShowMoveNumber(20);

    QSettings settings;
    settings.setValue("marker/moveNumber", 20);
}

/**
* Slot
* View -> Move Number-> Last 50 moves
*/
void MainWindow::on_actionLast50Moves_triggered(){
    ui->boardWidget->setShowMoveNumber(50);

    QSettings settings;
    settings.setValue("marker/moveNumber", 50);
}

/**
* Slot
* View -> Move Number-> All Moves
*/
void MainWindow::on_actionAllMoves_triggered(){
    ui->boardWidget->setShowMoveNumber(-1);

    QSettings settings;
    settings.setValue("marker/moveNumber", -1);
}

/**
* Slot
* View -> Show Coordinates
*/
void MainWindow::on_actionShowCoordinate_triggered(){
    ui->boardWidget->setShowCoordinates( ui->actionShowCoordinate->isChecked() );

    QSettings settings;
    settings.setValue("marker/showCoordinate", ui->actionShowCoordinate->isChecked());
}

/**
* Slot
* View -> Show Coordinates with I
*/
void MainWindow::on_actionShowCoordinateI_triggered(){
    ui->boardWidget->setShowCoordinatesWithI( ui->actionShowCoordinateI->isChecked() );

    go::nodePtr currentNode = ui->boardWidget->getCurrentNode();

    setTreeData();

    ui->boardWidget->setCurrentNode(currentNode);

    QSettings settings;
    settings.setValue("marker/showCoordinateWithI", ui->actionShowCoordinateI->isChecked());
}

/**
* Slot
* View -> Show Marker
*/
void MainWindow::on_actionShowMarker_triggered(){
    ui->boardWidget->setShowMarker( ui->actionShowMarker->isChecked() );

    QSettings settings;
    settings.setValue("marker/showMarker", ui->actionShowMarker->isChecked());
}

/**
* Slot
* View -> Show Branch Moves
*/
void MainWindow::on_actionShowBranchMoves_triggered(){
    ui->boardWidget->setShowBranchMoves( ui->actionShowBranchMoves->isChecked() );

    QSettings settings;
    settings.setValue("marker/showBranchMoves", ui->actionShowMarker->isChecked());
}

/**
* Slot
* View -> Branch Mode
*/
void MainWindow::on_actionBranchMode_triggered(){
    branchMode = ui->actionBranchMode->isChecked();

    go::nodePtr currentNode = ui->boardWidget->getCurrentNode();
    QTreeWidgetItem* currentItem = ui->branchWidget->currentItem();

    remakeTreeWidget( ui->branchWidget->topLevelItem(0) );

//    ui->boardWidget->setCurrentNode(current);
    ui->branchWidget->setCurrentItem(currentItem);

//    setTreeData();
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
        setCountTerritoryMode();
        countTerritoryDialog->setInformationNode( ui->boardWidget->getData().root.get() );
        countTerritoryDialog->show();
    }
    else{
        setCountTerritoryMode(false);
        setCaption();
    }

    ui->boardWidget->setCountTerritoryMode(ui->actionCountTerritory->isChecked());
}

/**
* Slot
* Tools -> play with computer
*/
void MainWindow::on_actionPlayWithGnugo_triggered(){
    if (ui->actionPlayWithGnugo->isChecked()){
        PlayWithComputerDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted){
            ui->actionPlayWithGnugo->setChecked(false);
            return;
        }

        if (fileNew(dlg.size, dlg.size, dlg.handicap, dlg.komi) == false){
            ui->actionPlayWithGnugo->setChecked(false);
            return;
        }

        QString param;
        param.sprintf(" --mode gtp --boardsize %d --komi %.2f --handicap %d --level %d",
                        dlg.size, dlg.komi, dlg.handicap, dlg.level);
        param = '"' + dlg.path + '"' + param;
        qDebug() << param;
        gtpProcess.start(param, QIODevice::ReadWrite|QIODevice::Text);
        if (gtpProcess.state() == QProcess::NotRunning){
            ui->boardWidget->playWithComputer(NULL);
            QMessageBox::critical(this, APPNAME, tr("Can not launch computer go."));
            return;
        }

        playGame = new gtp(ui->boardWidget, dlg.isBlack ? go::black : go::white, gtpProcess);
        connect( playGame, SIGNAL(gameEnded()), this, SLOT(playGameEnded()) );
        setPlayWithComputerMode(true);
        ui->boardWidget->playWithComputer(playGame);
    }
    else{
        QMessageBox::StandardButton ret = QMessageBox::warning(this, APPNAME,
                                                                tr("Playing with computer can not be resumed.\n"
                                                                   "Are you sure you want to stop playing with computer?"),
                                                                QMessageBox::Ok|QMessageBox::Cancel);
        if (ret != QMessageBox::Ok){
            ui->actionPlayWithGnugo->setChecked(true);
            return;
        }

        endGame();
    }
}

/**
* Slot
* Tools -> Tutor Boss Sides
*/
void MainWindow::on_actionTutorBossSides_triggered(){
    if (ui->actionTutorBossSides->isChecked()){
        ui->boardWidget->setTutorMode(BoardWidget::eTutorBossSides);
        ui->actionTutorOneSide->setChecked(false);
    }
    else
        ui->boardWidget->setTutorMode(BoardWidget::eNoTutor);
}

/**
* Slot
* Tools -> Tutor One Side
*/
void MainWindow::on_actionTutorOneSide_triggered(){
    if (ui->actionTutorOneSide->isChecked()){
        ui->boardWidget->setTutorMode(BoardWidget::eTutorOneSide);
        ui->actionTutorBossSides->setChecked(false);
    }
    else
        ui->boardWidget->setTutorMode(BoardWidget::eNoTutor);
}

/**
* Slot
* Tools -> Play Sound
*/
void MainWindow::on_actionPlaySound_triggered(){
    QSettings setting;
    setting.setValue("sound/play", ui->actionPlaySound->isChecked());
    ui->boardWidget->setPlaySound( ui->actionPlaySound->isChecked() );
}

/**
* Slot
* Tools -> 19 x 19 Board
*/
void MainWindow::on_action19x19Board_triggered(){
    fileNew(19, 19);
}

/**
* Slot
* Tools -> 13 x 13 Board
*/
void MainWindow::on_action13x13Board_triggered(){
    fileNew(13, 13);
}

/**
* Slot
* Tools -> 9 x 9 Board
*/
void MainWindow::on_action9x9Board_triggered(){
    fileNew(9, 9);
}

/**
* Slot
* Tools -> Custom Board Size
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
            fileNew(iW, iH == 0 ? iW : iH);
}

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

    ui->boardWidget->readSettings();
    ui->boardWidget->repaintBoard();
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
void MainWindow::on_boardWidget_nodeDeleted(go::nodePtr node, bool deleteChildren){
    if (node->parent)
        remakeTreeWidget( nodeToTreeWidget[node->parent] );
    deleteTreeWidget(node, deleteChildren);

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
    setAnnotation(node->annotation, node->moveAnnotation, node->nodeAnnotation);

    int b, w;
    ui->boardWidget->getCaptured(b, w);
    capturedLabel->setText(tr("Dead: Black %1 White %2").arg(b).arg(w));

    int num = ui->boardWidget->getMoveNumber();
    QString coord = ui->boardWidget->getXYString(node->getX(), node->getY());
    moveNumberLabel->setText(tr("LastMove: %1(%2)").arg(num).arg(coord));
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
    countTerritoryDialog->setScore(alive_b, alive_w, dead_b, dead_w, capturedBlack, capturedWhite, blackTerritory, whiteTerritory, komi);
/*
    countTerritoryDialog->setScoreText(s);
*/
}

/**
* Slot
*/
void MainWindow::playGameEnded(){
    bool resign = playGame->isResign();

    ui->actionPlayWithGnugo->setChecked(false);
    endGame();

    if( !resign ){
        ui->actionCountTerritory->setChecked(true);
        on_actionCountTerritory_triggered();
    }
}

/**
* Slot
* comment was modified.
*/
void MainWindow::on_commentWidget_textChanged(){
    go::nodePtr currentNode = ui->boardWidget->getCurrentNode();
    ui->boardWidget->setCommentCommand(currentNode, ui->commentWidget->toPlainText());
}

/**
* Slot
* score dialog was closed
*/
void MainWindow::scoreDialogClosed(int){
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
bool MainWindow::fileNew(int xsize, int ysize, int handicap, double komi){
    if (fileClose() == false)
        return false;

    ui->boardWidget->setBoardSize(xsize, ysize);
    ui->boardWidget->getData().root->handicap = handicap;
    ui->boardWidget->getData().root->komi = komi;
    setTreeData();
    return true;
}

/**
* file open.
*/
bool MainWindow::fileOpen(){
    QString selectedFilter;
    QString fname = QFileDialog::getOpenFileName(this, QString(), QString(), tr("All Go Format(*.sgf *.ugf *.ugi);;sgf(*.sgf);;ugf(*.ugf *.ugi);;All Files(*.*)"), &selectedFilter);
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
bool MainWindow::fileOpen(const QString& fname, bool guessCodec){
    return fileOpen(fname, QFileInfo(fname).suffix(), guessCodec);
}

/**
* file open.
*/
bool MainWindow::fileOpen(const QString& fname, const QString& ext, bool guessCodec){
    if (maybeSave() == false)
        return false;

    setCurrentFile(fname);

    if (ext == "sgf"){
        go::sgf sgf;
        sgf.read(fname, codec, guessCodec);
        ui->boardWidget->setData(sgf);
    }
    else if (ext == "ugf" || ext == "ugi"){
        go::ugf ugf;
        ugf.read(fname, codec, guessCodec);
        ui->boardWidget->setData(ugf);
    }
    else
        return false;

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
    setCurrentFile(fname);
    go::sgf sgf;
    ui->boardWidget->getData(sgf);
    sgf.save(fileName, codec);
    ui->boardWidget->setDirty(false);

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
    QMessageBox::warning(this, APPNAME,
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
void MainWindow::setCurrentFile(const QString& fname){
    fileName = fname;

    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
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
    qDebug() << "openURLDone: " << error;
    delete progressDialog;

    if (error == true){
        downloadBuff.clear();
        return;
    }

    go::sgf sgf;
    sgf.read(downloadBuff, codec, true);

    downloadBuff.clear();
    ui->boardWidget->setData(sgf);

    setTreeData();
    setCaption();
}

/**
* slot
*/
void MainWindow::openUrlCancel(){
    http->abort();
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
            if (parentNode->childNodes.front() == node)
                parentWidget->insertChild(0, treeWidget);
            else
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
            bool isselected = treeWidget->isSelected();
            p->removeChild(treeWidget);
            parentWidget2->addChild(treeWidget);
            if (isselected)
                ui->branchWidget->setCurrentItem(treeWidget);
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
    // Create TreeItem Widget
    QTreeWidgetItem* nodeWidget = new QTreeWidgetItem( QStringList(createTreeText(node)) );

    QVariant v;
    v.setValue(node);
    nodeWidget->setData(0, Qt::UserRole, v);
    nodeToTreeWidget[node] = nodeWidget;

    // Set Icon to TreeItem Widget
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

void MainWindow::deleteNode(bool deleteChildren){
    if (countTerritoryMode || playWithComputerMode)
        return;

    ui->boardWidget->deleteNodeCommand( ui->boardWidget->getCurrentNode(), deleteChildren );
}

void MainWindow::deleteTreeWidget(go::nodePtr node, bool deleteChildren){
    QTreeWidgetItem* treeWidget = nodeToTreeWidget[node];
    go::nodeList::iterator iter = node->childNodes.begin();
    while (deleteChildren && iter != node->childNodes.end()){
        QTreeWidgetItem* treeWidget2 = nodeToTreeWidget[*iter];
        if (treeWidget2->parent() != treeWidget)
            deleteTreeWidget(*iter, deleteChildren);
        ++iter;
    }

    deleteTreeWidgetForMap(node);
    delete treeWidget;
}

void MainWindow::deleteTreeWidgetForMap(go::nodePtr node){
    NodeToTreeWidgetType::iterator iter = nodeToTreeWidget.find(node);
    if(iter == nodeToTreeWidget.end())
        return;

    go::nodeList::iterator iter2 = node->childNodes.begin();
    while (iter2 != node->childNodes.end()){
        NodeToTreeWidgetType::iterator iter3 = nodeToTreeWidget.find(*iter2);
        if(iter3 != nodeToTreeWidget.end())
            if ((*iter3)->parent() == *iter)
                deleteTreeWidgetForMap(*iter2);

        ++iter2;
    }

    nodeToTreeWidget.erase(iter);
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

    ui->boardWidget->setEditMode(editMode);
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

    this->annotation     = annotation;
    this->moveAnnotation = moveAnnotation;
    this->nodeAnnotation = nodeAnnotation;
}

void MainWindow::setAnnotation(QAction* action, int annotation){
    annotation = action->isChecked() ? annotation : 0;
    ui->boardWidget->setAnnotation(annotation);
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

    moveAnnotation = action->isChecked() ? annotation : 0;
    ui->boardWidget->setMoveAnnotation(moveAnnotation);
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

    nodeAnnotation = action->isChecked() ? annotation : 0;
    ui->boardWidget->setNodeAnnotation(nodeAnnotation);
}

void MainWindow::setCountTerritoryMode(bool on){
    countTerritoryMode = on;

    static QAction* act[] = {
        ui->actionNew,
        ui->actionOpen,
        ui->actionOpenURL,
        ui->actionSave,
        ui->actionSaveAs,
        ui->actionReload,
        ui->actionSaveBoardAsPicture,
        ui->actionExportAsciiToClipboard,
//        ui->actionExit,

        ui->actionCopySGFtoClipboard,
        ui->actionCopyCurrentSGFtoClipboard,
        ui->actionPasteSGFfromClipboard,
        ui->actionPasteSGFasBranchfromClipboard,
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
        ui->actionTutorBossSides,
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

        ui->actionMainToolbar,
        ui->actionNavigationToolbar,
        ui->actionEditToolbar,
        ui->actionOptionToolbar,

//        ui->actionAbout,
//        ui->actionAboutQT,

        ui->menuRecentFiles->menuAction(),
        ui->menuShowMoveNumber->menuAction(),
        ui->menuStoneMarkers->menuAction(),
        undoAction,
        redoAction,
    };
    static int N = sizeof(act) / sizeof(act[0]);
    static QVector<bool> status(N);

    if (on)
        for (int i=0; i<N; ++i){
            status[i] = act[i]->isEnabled();
            act[i]->setEnabled( false );
        }
    else
        for (int i=0; i<N; ++i)
            act[i]->setEnabled( status[i] );

    ui->commentWidget->setEnabled( !on );
    ui->branchWidget->setEnabled( !on );
    ui->undoView->setEnabled( !on );
}

void MainWindow::setPlayWithComputerMode(bool on){
    playWithComputerMode = on;

    static QAction* act[] = {
        ui->actionNew,
        ui->actionOpen,
        ui->actionOpenURL,
        ui->actionSave,
        ui->actionSaveAs,
        ui->actionReload,
        ui->actionSaveBoardAsPicture,
        ui->actionExportAsciiToClipboard,
//        ui->actionExit,

        ui->actionCopySGFtoClipboard,
        ui->actionCopyCurrentSGFtoClipboard,
        ui->actionPasteSGFfromClipboard,
        ui->actionPasteSGFasBranchfromClipboard,
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

        ui->actionShowMoveNumber,
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

        ui->actionCountTerritory,
//        ui->actionPlayWithGnugo,
        ui->actionTutorBossSides,
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

        ui->actionMainToolbar,
        ui->actionNavigationToolbar,
        ui->actionEditToolbar,
        ui->actionOptionToolbar,

//        ui->actionAbout,
//        ui->actionAboutQT,

        ui->menuRecentFiles->menuAction(),
        ui->menuShowMoveNumber->menuAction(),
        ui->menuStoneMarkers->menuAction(),
        undoAction,
        redoAction,
    };
    static int N = sizeof(act) / sizeof(act[0]);
    static QVector<bool> status(N);

    if (on){
        undoGroup.setActiveStack(0);
        for (int i=0; i<N; ++i){
            status[i] = act[i]->isEnabled();
            act[i]->setEnabled( false );
        }
    }
    else{
        undoGroup.setActiveStack(ui->boardWidget->getUndoStack());
        for (int i=0; i<N; ++i)
            act[i]->setEnabled( status[i] );
    }

    ui->commentWidget->setEnabled( !on );
    ui->branchWidget->setEnabled( !on );
    ui->undoView->setEnabled( !on );
}

void MainWindow::endGame(){
    gtpProcess.close();
    delete playGame;
    ui->boardWidget->playWithComputer(NULL);

    setPlayWithComputerMode(false);
}

void MainWindow::alertLanguageChanged(){
    QMessageBox::information(this, APPNAME, tr("Changing the language requires that application be restarted."));
}
