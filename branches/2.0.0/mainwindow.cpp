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
#include <QTextCodec>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QLabel>
#include <QComboBox>
#include <QTreeWidget>
#include <QPlainTextEdit>
#include <QClipboard>
#include <QStandardItemModel>
#include "mugoapp.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gameinformationdialog.h"
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
    setNewDocumentName(doc);
    createNewTab(doc);

    return true;
}

/**
  open exist file in new tab.
  if file is already opened, document will be active.
*/
bool MainWindow::fileOpen(const QString& fname, QTextCodec* codec, bool guessCodec){
    // find document from opened document list,
    // and show document if document found.
    DocViewData::iterator iter = docView.begin();
    while (iter != docView.end()){
        if (iter.key()->fileInfo() == fname){
            ui->boardTabWidget->setCurrentWidget( docView[iter.key()].boardWidget );
            break;
        }
        ++iter;
    }

    // open document
    GoDocument* doc = openDocument(fname, codec, guessCodec);
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
    delete view.collectionModel;
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
    // Encoding
    createEncodingMenu();

    // File
    ui->actionNew->setShortcut(QKeySequence::New);
    ui->actionOpen->setShortcut(QKeySequence::Open);
    ui->actionCloseTab->setShortcut(QKeySequence::Close);
    ui->actionSave->setShortcut(QKeySequence::Save);
    ui->actionSaveAs->setShortcut(QKeySequence::SaveAs);
    ui->actionExit->setShortcut(QKeySequence::Quit);

    // Edit
    ui->actionCopySGFToClipboard->setShortcut(QKeySequence::Copy);
    ui->actionPasteSGFToNewTab->setShortcut(QKeySequence::Paste);

    // Edit -> undo/redo
    undoAction = undoGroup.createUndoAction(this);
    undoAction->setShortcut(QKeySequence::Undo);
//    undoAction->setIcon( QIcon(":/res/undo.png") );
    redoAction = undoGroup.createRedoAction(this);
    redoAction->setShortcut(QKeySequence::Redo);
//    redoAction->setIcon( QIcon(":/res/redo.png") );
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), redoAction);
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), undoAction);
//    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), redoAction);
//    ui->menuEdit->insertAction(redoAction, undoAction);
//    ui->editToolBar->insertAction(ui->editToolBar->actions().at(0), redoAction);
//    ui->editToolBar->insertAction(redoAction, undoAction);

    // Collection
    ui->collectionDockWidget->toggleViewAction()->setIcon( QIcon(":/res/gamelist.png") );
    ui->collectionToolBar->insertAction( ui->collectionToolBar->actions().at(0), ui->collectionDockWidget->toggleViewAction() );

    // Window
    ui->menuWindow->insertAction( ui->menuWindow->actions().at(1), ui->undoDockWidget->toggleViewAction() );
    ui->menuWindow->insertAction( ui->menuWindow->actions().at(1), ui->branchDockWidget->toggleViewAction() );
    ui->menuWindow->insertAction( ui->menuWindow->actions().at(1), ui->commentDockWidget->toggleViewAction() );

    // Window -> Toolbars
    ui->menuToolbars->addAction( ui->fileToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->editToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->navigationToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->viewToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->collectionToolBar->toggleViewAction() );
    ui->menuToolbars->addAction( ui->playToolBar->toggleViewAction() );
}

/**
  create reload menu
*/
void MainWindow::createEncodingMenu(){
    QAction* actions[] = {
        ui->actionSystem,
        ui->actionUTF8,
        ui->actionISO_8859_1,
        ui->actionISO_8859_15,
        ui->actionWindows_1252,
        ui->actionISO_8859_14,
        ui->actionISO_8859_7,
        ui->actionWindows_1253,
        ui->actionISO_8859_10,
        ui->actionISO_8859_3,
        ui->actionISO_8859_4,
        ui->actionISO_8859_13,
        ui->actionWindows_1257,
        ui->actionISO_8859_2,
        ui->actionWindows_1250,
        ui->actionISO_8859_5,
        ui->actionWindows_1251,
        ui->actionKOI8_R,
        ui->actionKOI8_U,
        ui->actionISO_8859_16,
        ui->actionISO_8859_11,
        ui->actionISO_8859_9,
        ui->actionWindows_1254,
        ui->actionWindows_1258,
        ui->actionISO_8859_6,
        ui->actionWindows_1256,
        ui->actionWindows_1255,
        ui->actionISO_8859_8,
        ui->actionGB2312,
        ui->actionBig5,
        ui->actionEucKR,
        ui->actionEucJP,
        ui->actionISO_2022_JP,
        ui->actionShiftJIS,
    };

    const char* codecs[] = {
        "System",
        "UTF8",
        "ISO-8859-1",
        "ISO-8859-15",
        "Windows_1252",
        "ISO_8859_14",
        "ISO_8859_7",
        "Windows_1253",
        "ISO_8859_10",
        "ISO_8859_3",
        "ISO_8859_4",
        "ISO_8859_13",
        "Windows_1257",
        "ISO_8859_2",
        "Windows_1250",
        "ISO_8859_5",
        "Windows_1251",
        "KOI8_R",
        "KOI8_U",
        "ISO_8859_16",
        "ISO_8859_11",
        "ISO_8859_9",
        "Windows_1254",
        "Windows_1258",
        "ISO_8859_6",
        "Windows_1256",
        "Windows_1255",
        "ISO8859_8",
        "GB2312",
        "Big5",
        "EucKR",
        "EucJP",
        "ISO-2022-JP",
        "ShiftJIS",
    };

    int N = sizeof(actions) / sizeof(actions[0]);

    QActionGroup* group = new QActionGroup(this);
    QList<QAction*> actionList;
    QList<QTextCodec*> codecList;

    for (int i=0; i<N; ++i){
        group->addAction(actions[i]);
        actionList.push_back(actions[i]);
        codecList.push_back(QTextCodec::codecForName(codecs[i]));
    }
    mugoApp()->setEncodingActions(actionList);
    mugoApp()->setCodecs(codecList);

/*
"System"

"UTF-8"

"ISO-8859-1"
"ISO-8859-15"
"windows-1252"
"ISO-8859-14"
"ISO-8859-7"
"windows-1253"
"ISO-8859-10"
"ISO-8859-3"

"ISO-8859-4"
"ISO-8859-13"
"windows-1257"
"ISO-8859-2"
"windows-1250"
"ISO-8859-5"
"windows-1251"
"KOI8-R"
"KOI8-U"
"ISO-8859-16"

"TIS-620" (ISO-8859-11)
"ISO-8859-9"
"windows-1254"
"windows-1258"

"ISO-8859-6"
"windows-1256"
"windows-1255"
"ISO-8859-8"

"GB2312"
"Big5"

"EUC-KR"

"EUC-JP"
"ISO-2022-JP"
"Shift_JIS"




"UTF-32LE"
"UTF-32BE"
"UTF-32"
"UTF-16LE"
"UTF-16BE"
"UTF-16"
"roman8"
"TIS-620"
"WINSAMI2"
"Apple Roman"
"IBM866"
"IBM874"
"IBM850"
"Iscii-Mlm"
"Iscii-Knd"
"Iscii-Tlg"
"Iscii-Tml"
"Iscii-Ori"
"Iscii-Gjr"
"Iscii-Pnj"
"Iscii-Bng"
"Iscii-Dev"
"TSCII"
"GB18030"
"GBK"
"cp949"
"Big5-HKSCS"
*/
}

/**
  show document in new tab
*/
bool MainWindow::createNewTab(Document* doc){
    GoDocument* goDoc = qobject_cast<GoDocument*>(doc);
    if (goDoc){
        connect(goDoc, SIGNAL(dirtyChanged(bool)), SLOT(on_sgfDocument_dirtyChanged(bool)));
        connect(goDoc, SIGNAL(gameAdded(const Go::NodePtr&)), SLOT(on_sgfDocument_gameAdded(const Go::NodePtr&)));
        connect(goDoc, SIGNAL(gameDeleted(const Go::NodePtr&, int)), SLOT(on_sgfDocument_gameDeleted(const Go::NodePtr&, int)));
        connect(goDoc, SIGNAL(nodeAdded(const Go::NodePtr&, const Go::NodePtr&)), SLOT(on_sgfDocument_nodeAdded(const Go::NodePtr&, const Go::NodePtr&)));
        connect(goDoc, SIGNAL(nodeDeleted(const Go::NodePtr&, const Go::NodePtr&)), SLOT(on_sgfDocument_nodeDeleted(const Go::NodePtr&, const Go::NodePtr&)));
        connect(goDoc, SIGNAL(nodeModified(const Go::NodePtr&, const Go::NodePtr&)), SLOT(on_sgfDocument_nodeModified(const Go::NodePtr&, const Go::NodePtr&)));
        connect(goDoc, SIGNAL(informationChanged(const Go::NodePtr&, const Go::InformationPtr&)), SLOT(on_sgfDocument_informationChanged(const Go::NodePtr&, const Go::InformationPtr&)));
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

        // new collection model
        QStandardItemModel* model = new QStandardItemModel(0, 5);
        model->setHorizontalHeaderLabels(QStringList() << tr("White") << tr("Black") << tr("Result") << tr("Date") << tr("Game Name"));
        createCollectionModel(goDoc, model);
        data.collectionModel = model;

        // new board widget
        BoardWidget* board = new BoardWidget;
        data.boardWidget = board;
        connect(board, SIGNAL(gameChanged(const Go::NodePtr&)), SLOT(on_board_gameChanged(const Go::NodePtr&)));
        connect(board, SIGNAL(nodeChanged(const Go::NodePtr&)), SLOT(on_board_nodeChanged(const Go::NodePtr&)));
        board->setDocument(goDoc);
        ui->boardTabWidget->addTab(board, goDoc->name());
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
  open document from file, view isn't be created.
*/
/**
  open exist file in new tab.
  if file is already opened, document will be active.
*/
GoDocument* MainWindow::openDocument(const QString& fname, QTextCodec* codec, bool guessCodec){
    // if codec isn't designated, default codec of application is used.
    if (codec == NULL)
        codec = mugoApp()->defaultCodec();

    // open document if document isn't opened.
    QFileInfo info(fname);
    QString ext = info.suffix().toLower();

    SgfDocument* doc = NULL;
    if (ext.compare("sgf") == 0){
        doc = new SgfDocument(this);
        if (doc->open(fname, codec, guessCodec) == false){
            delete doc;
            return NULL;
        }
    }

    return doc;
}

/**
  Reload document with specified codec
*/
bool MainWindow::reload(QTextCodec* codec)
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return false;

    // file name
    QFileInfo fileInfo = board->document()->fileInfo();
    if (!fileInfo.isReadable())
        return false;

    // reload
    if (closeDocument(board->document()) == false)
        return false;
    return fileOpen(fileInfo.filePath(), codec, false);
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
void MainWindow::updateAllViews(GoDocument* doc){
    ViewData& view = docView[doc];
    updateNodeView(view, view.boardWidget->currentNode());

    Go::InformationPtr info = view.boardWidget->rootInformation();
    QString title;
    if (info->whitePlayer().isEmpty() ==false && info->blackPlayer().isEmpty() == false)
        title = tr("%1 %2(W) vs %3 %4(B)").arg(info->whitePlayer()).arg(info->whiteRank()).arg(info->blackPlayer()).arg(info->blackRank());
    else
        title = doc->name();
    setWindowTitle(title);
}

/**
  udpate views by node
*/
void MainWindow::updateNodeView(ViewData& view, const Go::NodePtr& node){
    updateCommentView(view, node);
}

/**
  update comment view
*/
void MainWindow::updateCommentView(ViewData& view, const Go::NodePtr& node){
    if (view.boardWidget->currentNode() != node)
        return;

    if (view.commentEdit->toPlainText() != node->comment())
        view.commentEdit->setPlainText(node->comment());
}

/**
  create tree items in branch view
*/
void MainWindow::createBranchItems(Document* doc, const Go::NodePtr& game){
    // view data
    ViewData& view = docView[doc];

    // get branch view
    QTreeWidget* branch = view.branchWidget;
    if (branch == NULL)
        return;

    // create items in branch view
    view.nodeToTreeItem.clear();
    branch->clear();
    createBranchItems(doc, branch->invisibleRootItem(), game);
}

/**
  create tree items in branch view
*/
void MainWindow::createBranchItems(Document* doc, QTreeWidgetItem* parent, const Go::NodePtr& node){
    QTreeWidgetItem* item = createBranchItem(doc, node);
    parent->addChild(item);
    foreach (const Go::NodePtr& child, node->children())
        if (shouldNest(child))
            createBranchItems(doc, item, child);
        else
            createBranchItems(doc, parent, child);
}

/**
  add tree item in branch view
*/
void MainWindow::addBranchItem(Document* doc, QTreeWidget* branch, const Go::NodePtr& node){
    // create new tree item
    QTreeWidgetItem* item = createBranchItem(doc, node);

    ViewData& view = docView[doc];
    rebuildBranchItems(view, node->parent() ? node->parent() : node);
}

/**
  node should be nested.
*/
bool MainWindow::shouldNest(const Go::NodePtr& node){
    Go::NodePtr parent = node->parent();
    if (!parent)
        return false;
    else if (parent->children().size() > 1)
        return true;

    Go::NodePtr parentOfParent = parent->parent();
    if (!parentOfParent)
        return false;
    else if (parentOfParent->children().size() > 1)
        return true;

    return false;
}

/**
  re-create tree view to node and descendent node
*/
void MainWindow::rebuildBranchItems(ViewData& view, const Go::NodePtr& node){
    QTreeWidgetItem* item = view.nodeToTreeItem[node];
    QTreeWidgetItem* parentItem = view.nodeToTreeItem[node->parent()];
    if (parentItem){
        if (shouldNest(node) == false)
            parentItem = getParentItem(parentItem);

        QTreeWidgetItem* currentParent = getParentItem(item);
        if (currentParent != parentItem){
            if (currentParent)
                currentParent->removeChild(item);
            parentItem->addChild(item);
        }
    }

    foreach(const Go::NodePtr& child, node->children())
        rebuildBranchItems(view, child);
}

/**
  create tree item for branch view
*/
QTreeWidgetItem* MainWindow::createBranchItem(Document* doc, const Go::NodePtr& node){
    static QIcon green(":/res/green_64.png");
    static QIcon black(":/res/black_128.png");
    static QIcon white(":/res/white_128.png");

    ViewData& view = docView[doc];
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
    if (node->isStone() && !node->isPass())
        nodeName.push_back( Go::coordinateString(view.boardWidget->xsize(), view.boardWidget->ysize(), node->x(), node->y(), false) );
    else if (node->isStone())
        nodeName.push_back( tr("Pass") );
    else if (node->information())
        nodeName.push_back( tr("Game Info") );

    if (node->name().isEmpty() == false)
        nodeName.push_back(node->name());

    if (node->comment().isEmpty() == false)
        nodeName.push_back( tr("Comment") );

    // Node Annotation
    switch(node->nodeAnnotation()){
        case Go::Node::eGoodForBlack:
            nodeName.push_back( tr("[Good for Black]") );
            break;
        case Go::Node::eVeryGoodForBlack:
            nodeName.push_back( tr("[Very Good for Black]") );
            break;
        case Go::Node::eGoodForWhite:
            nodeName.push_back( tr("[Good for White]") );
            break;
        case Go::Node::eVeryGoodForWhite:
            nodeName.push_back( tr("[Very Good for White]") );
            break;
        case Go::Node::eEven:
            nodeName.push_back( tr("[Even]") );
            break;
        case Go::Node::eUnclear:
            nodeName.push_back( tr("[Unclear]") );
            break;
    }
    switch(node->nodeAnnotation2()){
        case Go::Node::eHotspot:
            nodeName.push_back( tr("[Hotspot]") );
            break;
    }

    // Move Annotation
    switch(node->moveAnnotation()){
        case Go::Node::eGoodMove:
            nodeName.push_back( tr("[Good Move]") );
            break;
        case Go::Node::eVeryGoodMove:
            nodeName.push_back( tr("[Very Good Move]") );
            break;
        case Go::Node::eBadMove:
            nodeName.push_back( tr("[Bad Move]") );
            break;
        case Go::Node::eVeryBadMove:
            nodeName.push_back( tr("[Very Bad Move]") );
            break;
        case Go::Node::eDoubtful:
            nodeName.push_back( tr("[Doubtful]") );
            break;
        case Go::Node::eInteresting:
            nodeName.push_back( tr("[Interesting]") );
            break;
    }

    // estimated score
    if (node->hasEstimatedScore()){
        if (node->estimatedScore() > 0)
            nodeName.push_back( tr("(B+%1)").arg(node->estimatedScore()) );
        else if (node->estimatedScore() < 0)
            nodeName.push_back( tr("(W+%1)").arg(node->estimatedScore() * -1) );
        else
            nodeName.push_back( tr("(Even)") );
    }

    item->setText(0, nodeName.join(" "));

    // set node to tree item data
    item->setData(0, Qt::UserRole, QVariant::fromValue<Go::NodePtr>(node));
    view.nodeToTreeItem[node] = item;

    return item;
}

/**
  get parent of item.
  if item is top level item, return invisible root item.
*/
QTreeWidgetItem* MainWindow::getParentItem(QTreeWidgetItem* item){
    QTreeWidgetItem* parent = item->parent();
    if (parent)
        return parent;

    QTreeWidget* tree = item->treeWidget();
    if (tree)
        return tree->invisibleRootItem();

    return NULL;
}

/**
  create model for collection view
*/
void MainWindow::createCollectionModel(GoDocument* doc, QStandardItemModel* model){
    for (int i=0; i<doc->gameList.size(); ++i){
        Go::NodePtr& game = doc->gameList[i];
        QList<QStandardItem*> items = createCollectionRow(game);
        model->appendRow(items);
    }
}

/**
  create collection row items
*/
QList<QStandardItem*> MainWindow::createCollectionRow(const Go::NodePtr& game){
    QList<QStandardItem*> items;
    items.push_back( new QStandardItem(game->information()->whitePlayer()) );
    items.push_back( new QStandardItem(game->information()->blackPlayer()) );
    items.push_back( new QStandardItem(game->information()->result()) );
    items.push_back( new QStandardItem(game->information()->date()) );

    const QString& gameName = game->information()->gameName();
    const QString& event = game->information()->event();
    if (gameName.isEmpty() == false && event.isEmpty() == true)
        items.push_back( new QStandardItem(gameName) );
    else if (gameName.isEmpty() == true && event.isEmpty() == false)
        items.push_back( new QStandardItem(event) );
    else if (gameName.isEmpty() == false && event.isEmpty() == false)
        items.push_back( new QStandardItem(gameName + ':' + event) );

    return items;
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
  File -> Open URL
*/
void MainWindow::on_actionOpenURL_triggered()
{
    QMessageBox::information(this, "mugo2", "Not Implemented");
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
  File -> Export Board as Image
*/
void MainWindow::on_actionExportBoardAsImage_triggered()
{
    QMessageBox::information(this, "mugo2", "Not Implemented");
}

/**
  File -> Export Ascii to Clipboard
*/
void MainWindow::on_actionExportAsciiToClipboard_triggered()
{
    QMessageBox::information(this, "mugo2", "Not Implemented");
}

/**
  File -> Print
*/
void MainWindow::on_actionPrint_triggered()
{
    QMessageBox::information(this, "mugo2", "Not Implemented");
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
  File -> Collection -> Import
  add file into sgf collection
*/
void MainWindow::on_actionCollectionImport_triggered()
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // show open file dialog
    QString fname;
    QTextCodec* codec = NULL;
    if (getOpenFileName(fname, codec) == false)
        return;

    // open document
    GoDocument* doc = openDocument(fname, codec, true);
    if (doc == NULL)
        return;

    // add game into sgf collection
    board->document()->undoStack()->push( new AddGameCommand(board->document(), doc->gameList) );
}

/**
  File -> Collection -> Extract
  extract game from sgf collection
*/
void MainWindow::on_actionCollectionExtract_triggered()
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // selected index
    QModelIndex index = ui->collectionTreeView->currentIndex();

    // create new game
    Go::NodePtr srcGame = board->document()->gameList[index.row()];
    Go::NodePtr dstGame;
    Go::copyNode(dstGame, srcGame);
    Go::NodeList gameList;
    gameList.push_back(dstGame);

    // show new game
    GoDocument* doc = new SgfDocument(gameList);
    setNewDocumentName(doc);
    createNewTab(doc);
    doc->modifyDocument();
}

/**
  File -> Reload -> System
*/
void MainWindow::on_actionSystem_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionSystem);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> UTF-8
*/
void MainWindow::on_actionUTF8_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionUTF8);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-1
*/
void MainWindow::on_actionISO_8859_1_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_1);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-15
*/
void MainWindow::on_actionISO_8859_15_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_15);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> Windows-1252
*/
void MainWindow::on_actionWindows_1252_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionWindows_1252);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-14
*/
void MainWindow::on_actionISO_8859_14_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_14);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-7
*/
void MainWindow::on_actionISO_8859_7_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_7);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> Windows-1253
*/
void MainWindow::on_actionWindows_1253_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionWindows_1253);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-10
*/
void MainWindow::on_actionISO_8859_10_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_10);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-3
*/
void MainWindow::on_actionISO_8859_3_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_3);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-4
*/
void MainWindow::on_actionISO_8859_4_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_4);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-13
*/
void MainWindow::on_actionISO_8859_13_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_13);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> Windows-1257
*/
void MainWindow::on_actionWindows_1257_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionWindows_1257);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-2
*/
void MainWindow::on_actionISO_8859_2_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_2);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> Windows-1250
*/
void MainWindow::on_actionWindows_1250_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionWindows_1250);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-5
*/
void MainWindow::on_actionISO_8859_5_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_5);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> Windows-1251
*/
void MainWindow::on_actionWindows_1251_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionWindows_1251);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> KOI8-R
*/
void MainWindow::on_actionKOI8_R_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionKOI8_R);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> KOI8-U
*/
void MainWindow::on_actionKOI8_U_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionKOI8_U);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-16
*/
void MainWindow::on_actionISO_8859_16_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_16);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-11
*/
void MainWindow::on_actionISO_8859_11_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_11);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-9
*/
void MainWindow::on_actionISO_8859_9_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_9);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> Windows-1254
*/
void MainWindow::on_actionWindows_1254_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionWindows_1254);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> Windows-1258
*/
void MainWindow::on_actionWindows_1258_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionWindows_1258);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-6
*/
void MainWindow::on_actionISO_8859_6_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_6);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> Windows-1256
*/
void MainWindow::on_actionWindows_1256_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionWindows_1256);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> Windows-1255
*/
void MainWindow::on_actionWindows_1255_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionWindows_1255);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> ISO-8859-8
*/
void MainWindow::on_actionISO8859_8_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_8859_8);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> GB2312
*/
void MainWindow::on_actionGB2312_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionGB2312);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> Big5
*/
void MainWindow::on_actionBig5_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionBig5);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> EUC-KR
*/
void MainWindow::on_actionEucKR_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionEucKR);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> EUC-JP
*/
void MainWindow::on_actionEucJP_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionEucJP);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> JIS
*/
void MainWindow::on_actionISO_2022_JP_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionISO_2022_JP);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  File -> Reload -> Shift_JIS
*/
void MainWindow::on_actionShiftJIS_triggered()
{
    // get codec
    int i = mugoApp()->encodingActions().indexOf(ui->actionShiftJIS);
    QTextCodec* codec = mugoApp()->codecs().at(i);
    reload(codec);
}

/**
  Edit -> Copy SGF to Clipboard
  sgf of all tree to clipboard
*/
void MainWindow::on_actionCopySGFToClipboard_triggered()
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // output only current game
    Go::NodeList game;
    game.push_back(board->currentGame());

    // create sgf
    Go::Sgf sgf(game);
    QString s;
    QTextStream str(&s);
    sgf.save(str);

    // write sgf to clipboard
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(s);
}

/**
  Edit -> Copy Current Branch to Clipboard
  sgf of current brnach to clipboard
*/
void MainWindow::on_actionCopyCurrentBranchToClipboard_triggered()
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // create node tree of current branch
    Go::NodePtr root, current;
    Go::NodeList nodeList = board->currentNodeList();
    foreach(const Go::NodePtr& node, nodeList){
        Go::NodePtr newNode( new Go::Node(*node) );
        if (!root)
            root = current = newNode;
        else{
            current->children().push_back(newNode);
            current = newNode;
        }
        current->children().clear();

        if (current->information())
            current->setInformation( Go::InformationPtr(new Go::Information(*node->information())) );
    }

    // output only current branch
    Go::NodeList game;
    game.push_back(root);

    // create sgf
    Go::Sgf sgf(game);
    QString s;
    QTextStream str(&s);
    sgf.save(str);

    // write sgf to clipboard
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(s);
}

/**
  Edit -> Paste SGF to New Tab
*/
void MainWindow::on_actionPasteSGFToNewTab_triggered()
{
    // read clipboard
    QClipboard* clipboard = QApplication::clipboard();
    QString text = clipboard->text();

    // parse sgf
    Go::NodeList gameList;
    Go::Sgf sgf(gameList);
    if (sgf.load(text) == false)
        return;

    // open new document from clipboard
    SgfDocument* doc = new SgfDocument(gameList, this);
    setNewDocumentName(doc);
    createNewTab(doc);
    doc->modifyDocument();
}

/**
  Edit -> Paste SGF into Collection
*/
void MainWindow::on_actionPasteSGFIntoCollection_triggered()
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // read clipboard
    QClipboard* clipboard = QApplication::clipboard();
    QString text = clipboard->text();

    // parse sgf
    Go::NodeList gameList;
    Go::Sgf sgf(gameList);
    if (sgf.load(text) == false)
        return;

    // add game into collection
    board->document()->undoStack()->push( new AddGameCommand(board->document(), gameList) );
}

/**
  Edit -> Game Information
*/
void MainWindow::on_actionGameInformation_triggered()
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // show dialog
    GameInformationDialog dlg(this, board->document(), board->currentGame());
    dlg.exec();
}

void MainWindow::on_actionDeleteAfterCurrent_triggered()
{

}

void MainWindow::on_actionDeleteOnlyCurrent_triggered()
{

}

void MainWindow::on_actionPass_triggered()
{

}

void MainWindow::on_actionAlternateMove_triggered()
{

}

void MainWindow::on_actionAddBlackStones_triggered()
{
}

void MainWindow::on_actionAddWhiteStones_triggered()
{
}

void MainWindow::on_actionAddEmpty_triggered()
{
}

void MainWindow::on_actionAddLabel_triggered()
{
}

void MainWindow::on_actionAddLabelManually_triggered()
{
}

void MainWindow::on_actionAddCircle_triggered()
{
}

void MainWindow::on_actionAddTriangle_triggered()
{
}

void MainWindow::on_actionAddSquare_triggered()
{
}

void MainWindow::on_actionAddCross_triggered()
{
}

void MainWindow::on_actionDeleteMarker_triggered()
{
}

void MainWindow::on_actionGoodMove_triggered()
{
}

void MainWindow::on_actionVeryGoodMove_triggered()
{
}

void MainWindow::on_actionBadMove_triggered()
{
}

void MainWindow::on_actionVeryBadMove_triggered()
{
}

void MainWindow::on_actionDoubtfulMove_triggered()
{
}

void MainWindow::on_actionInterestingMove_triggered()
{
}

void MainWindow::on_actionEven_triggered()
{
}

void MainWindow::on_actionGoodForBlack_triggered()
{
}

void MainWindow::on_actionVeryGoodforBlack_triggered()
{
}

void MainWindow::on_actionGoodforWhite_triggered()
{
}

void MainWindow::on_actionVeryGoodforWhite_triggered()
{
}

void MainWindow::on_actionUnclear_triggered()
{
}

void MainWindow::on_actionHotspot_triggered()
{
}

void MainWindow::on_actionSetMoveNumber_triggered()
{
}

void MainWindow::on_actionUnsetMoveNumber_triggered()
{
}

void MainWindow::on_actionEditNodeName_triggered()
{
}

void MainWindow::on_actionWhiteFirst_triggered()
{
}

void MainWindow::on_actionRotateSGFClockwise_triggered()
{
}

void MainWindow::on_actionFlipSGFVertically_triggered()
{
}

void MainWindow::on_actionFlipSGFHorizontally_triggered()
{
}

/**
  dirty flag changed
*/
void MainWindow::on_sgfDocument_dirtyChanged(bool dirty){
    // get document
    GoDocument* doc = qobject_cast<GoDocument*>(sender());
    if (doc == NULL)
        return;

    // get view
    ViewData& view = docView[doc];

    // create tab text string
    QString name = doc->name();
    if (dirty)
        name += " *";

    // set tab text
    int index = ui->boardTabWidget->indexOf(view.boardWidget);
    ui->boardTabWidget->setTabText(index, name);
}

/**
  game added
*/
void MainWindow::on_sgfDocument_gameAdded(const Go::NodePtr& game){
    // get document
    GoDocument* doc = qobject_cast<GoDocument*>(sender());
    if (doc == NULL)
        return;

    // get view
    ViewData& view = docView[doc];

    // create collection model
    QList<QStandardItem*> items = this->createCollectionRow(game);
    view.collectionModel->appendRow(items);
}

/**
  game deleted
*/
void MainWindow::on_sgfDocument_gameDeleted(const Go::NodePtr&, int index){
    // get document
    GoDocument* doc = qobject_cast<GoDocument*>(sender());
    if (doc == NULL)
        return;

    // get view
    ViewData& view = docView[doc];

/*
    // create collection model
    view.collectionModel->setRowCount(0);
    foreach(const Go::NodePtr& game, doc->gameList){
        QList<QStandardItem*> items = this->createCollectionRow(game);
        view.collectionModel->appendRow(items);
    }
*/
    // remove deleted game from collection model
    view.collectionModel->removeRow(index);
}

/**
  node modified
*/
void MainWindow::on_sgfDocument_nodeModified(const Go::NodePtr& game, const Go::NodePtr& node){
    // get document
    GoDocument* doc = qobject_cast<GoDocument*>(sender());
    if (doc == NULL)
        return;

    // get view data
    ViewData& view = docView[doc];

    // if added node isn't in the current game, there isn't needed to update view.
    if (game != view.boardWidget->currentGame())
        return;

    // update view
    updateNodeView(view, node);
}

/**
  node added
*/
void MainWindow::on_sgfDocument_nodeAdded(const Go::NodePtr& game, const Go::NodePtr& node){
    // get document
    GoDocument* doc = qobject_cast<GoDocument*>(sender());
    if (doc == NULL)
        return;

    // get view data
    ViewData& view = docView[doc];

    // if added node isn't in the current game, there isn't needed to update view.
    if (game != view.boardWidget->currentGame())
        return;

    // add node in branch view
    QTreeWidget* branch = view.branchWidget;
    addBranchItem(doc, branch, node);
}

/**
  node deleted
*/
void MainWindow::on_sgfDocument_nodeDeleted(const Go::NodePtr& game, const Go::NodePtr& node){
    // get document
    GoDocument* doc = qobject_cast<GoDocument*>(sender());
    if (doc == NULL)
        return;

    // get view data
    ViewData& view = docView[doc];

    // if added node isn't in the current game, there isn't needed to update view.
    if (game != view.boardWidget->currentGame())
        return;

    // delete tree item in branch view
    ViewData::NodeTreeMap::iterator iter = view.nodeToTreeItem.find(node);
    if (iter == view.nodeToTreeItem.end())
        return;
    QTreeWidgetItem* item = iter.value();
    view.nodeToTreeItem.erase(iter);
    delete item;

    // re-create tree view items
    this->rebuildBranchItems(view, node->parent());
}

/**
  game information changed
*/
void MainWindow::on_sgfDocument_informationChanged(const Go::NodePtr&, const Go::InformationPtr&){
    // get document
    GoDocument* doc = qobject_cast<GoDocument*>(sender());
    if (doc == NULL)
        return;

    // update views
    updateAllViews(doc);
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
    updateAllViews(board->document());

    // change collection view
    ViewData& view = docView[board->document()];
    int index = board->document()->gameList.indexOf(board->currentGame());
    ui->collectionTreeView->setModel( view.collectionModel );
    ui->collectionTreeView->setCurrentIndex( view.collectionModel->index(index, 0) );
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
    // get sender board
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    if (board == NULL)
        return;

    // create items in brahch view
    createBranchItems(board->document(), game);

    // update all views
    updateAllViews(board->document());
}

/**
  current node changed
*/
void MainWindow::on_board_nodeChanged(const Go::NodePtr& node){
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(sender());
    if (board == NULL)
        return;

    // select node in branch view
    ViewData& view = docView[board->document()];
    QTreeWidgetItem* item = view.nodeToTreeItem[node];
    if (item && view.branchWidget->currentItem() != item)
        view.branchWidget->setCurrentItem(item);

    // update view
    updateCommentView(view, node);
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
        lastCommentCommand = new SetCommentCommand(sgfDoc, view.boardWidget->currentGame(), view.boardWidget->currentNode(), view.commentEdit->toPlainText());
        doc->undoStack()->push(lastCommentCommand);
    }
    else
        lastCommentCommand->setComment(view.commentEdit->toPlainText());

    lastCommentNode = view.boardWidget->currentNode().data();
}

/**
  current item was changed in branch view
*/
void MainWindow::on_branchWidget_currentItemChanged(QTreeWidgetItem* current,QTreeWidgetItem* /*previous*/){
    if (current == NULL)
        return;

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

/**
  game changed in collection view
*/
void MainWindow::on_collectionTreeView_activated(QModelIndex index)
{
    // get active board widget
    BoardWidget* board = qobject_cast<BoardWidget*>(ui->boardTabWidget->currentWidget());
    if (board == NULL)
        return;

    // game change in board
    board->document()->undoStack()->push( new ChangeGameCommand(board->document(), board, board->document()->gameList[index.row()]) );
}
