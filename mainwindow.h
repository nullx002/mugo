#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QTreeWidgetItem>
#include <QTextCodec>
#include "appdef.h"
#include "godata.h"
#include "boardwidget.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	typedef QMap<go::node*, QTreeWidgetItem*> NodeToTreeWidgetType;


    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setCaption();

protected:
    virtual void closeEvent(QCloseEvent* e);
    virtual void keyPressEvent(QKeyEvent* event);

private:
    bool fileNew();
    bool fileOpen();
    bool fileOpen(const QString& fname);
    bool fileSave();
    bool fileSaveAs();
    bool fileSaveAs(const QString& fname);
    bool fileClose();
    bool maybeSave();

    void setEncoding(QAction* action, const char* codecName);
    void setShowMoveNumber(QAction* action, int moveNumber);
    void setEditMode(QAction* action, BoardWidget::eEditMode editMode);

    void setTreeData();
    QTreeWidgetItem* addTreeWidget(go::node& n);
    QTreeWidgetItem* addTreeWidget(QTreeWidgetItem* parentWidget, go::node& node);
    QTreeWidgetItem* remakeTreeWidget(QTreeWidgetItem* currentWidget);
    QTreeWidgetItem* createTreeWidget(QTreeWidgetItem* parentWidget, go::node& node);
    void deleteTreeWidget();
    void deleteTreeWidget(QTreeWidgetItem* treeWidget);
    void deleteTreeWidget(go::node* node);
    void deleteTreeWidgetForMap(go::node* node);
    void setTreeWidget(go::node* n);
    QString createTreeText(const go::node* node);

    go::node* getNode(QTreeWidgetItem* treeWidget);

    Ui::MainWindow *ui;
    QTextCodec* codec;
    QString documentName;
    QString fileName;
    NodeToTreeWidgetType nodeToTreeWidget;

private slots:
    // File menu
    void on_actionPass_triggered();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionReload_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSaveBoardAsPicture_triggered();
    void on_actionExit_triggered();

    // Edit menu
    void on_actionGameInformation_triggered();
    void on_actionDelete_triggered();

    // Edit menu -> Stone & Marker
    void on_actionAlternateMove_triggered();
    void on_actionAddBlackStone_triggered();
    void on_actionAddWhiteStone_triggered();
    void on_actionAddEmpty_triggered();
    void on_actionAddLabel_triggered();
    void on_actionAddCross_triggered();
    void on_actionAddCircle_triggered();
    void on_actionAddSquare_triggered();
    void on_actionAddTriangle_triggered();
    void on_actionDeleteMarker_triggered();

    // Edit menu -> Encoding
    void on_actionEncodingUTF8_triggered();
    void on_actionWindows_1252_triggered();
    void on_actionISO8859_1_triggered();
    void on_actionEncodingGB2312_triggered();
    void on_actionEncodingBig5_triggered();
    void on_actionEncodingKorean_triggered();
    void on_actionEncodingEucJP_triggered();
    void on_actionEncodingJIS_triggered();
    void on_actionEncodingShiftJIS_triggered();

    // View menu -> Move Number
    void on_actionNoMoveNumber_triggered();
    void on_actionLast1Move_triggered();
    void on_actionLast2Moves_triggered();
    void on_actionLast5Moves_triggered();
    void on_actionLast10Moves_triggered();
    void on_actionLast20Moves_triggered();
    void on_actionLast50Moves_triggered();
    void on_actionAllMoves_triggered();

    // View menu
    void on_actionBranchWindow_triggered();
    void on_actionCommentWindow_triggered();

    // Help menu
    void on_actionAbout_triggered();
    void on_actionAboutQT_triggered();

    // Board widget
    void on_boardWidget_nodeAdded(go::node* parent, go::node* node);
    void on_boardWidget_nodeDeleted(go::node* node);
    void on_boardWidget_nodeModified(go::node* node);
    void on_boardWidget_currentNodeChanged(go::node* node);

    // Branch widget
    void on_branchDockWidget_visibilityChanged(bool visible);
    void on_branchWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

    // Comment widget
    void on_commentDockWidget_visibilityChanged(bool visible);
    void on_commentWidget_textChanged();
};

#endif // MAINWINDOW_H
