#include <QDebug>
#include <QMessageBox>
#include "appdef.h"
#include "gtp.h"
#include "enginelistdialog.h"
#include "enginelist.h"
#include "ui_enginelistdialog.h"

/**
* Constructor
*/
EngineListDialog::EngineListDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::EngineListDialog)
    , process_(NULL)
    , gtp_(NULL)
{
    m_ui->setupUi(this);

    EngineList enginelist;
    enginelist.load();

    foreach (const Engine& e, enginelist.engines){
        QTreeWidgetItem* item = new QTreeWidgetItem(
            QStringList() << e.name << e.path << e.parameters
        );
        m_ui->engineList->addTopLevelItem(item);
    }
}

/**
* Destructor
*/
EngineListDialog::~EngineListDialog()
{
    delete m_ui;
    if(gtp_ != NULL){
        disconnect(gtp_);
        gtp_->kill();
    }
}

/**
* ok button clicked.
*/
void EngineListDialog::accept(){
    QDialog::accept();

    EngineList enginelist;

    for (int i=0; i<m_ui->engineList->topLevelItemCount(); ++i){
        QTreeWidgetItem* item = m_ui->engineList->topLevelItem(i);
        Engine e;
        e.name = item->text(0);
        e.path = item->text(1);
        e.parameters = item->text(2);
        enginelist.engines.push_back(e);
    }

    enginelist.save();
}

/**
* on browse
* select engine path.
*/
void EngineListDialog::on_browseButton_clicked(){
    QString fname = getOpenFileName(this);
    if (fname.isEmpty())
        return;

    m_ui->pathEdit->setText(fname);
}

/**
* on browse
* select engine path.
*/
void EngineListDialog::on_getNameButton_clicked(){
    // set default parameters.
    QFileInfo finfo( m_ui->pathEdit->text() );
    if (finfo.baseName().indexOf("gnugo", 0, Qt::CaseInsensitive) == 0 || finfo.baseName().indexOf("aya", 0, Qt::CaseInsensitive) == 0){
        const QString& param = m_ui->parametersEdit->text();
        if (param.indexOf("--mode") == -1){
            if (param.isEmpty())
                m_ui->parametersEdit->setText("--mode gtp");
            else
                m_ui->parametersEdit->setText("--mode gtp " + param);
        }
    }

    if (gtp_){
        disconnect(gtp_);
        gtp_->kill();
    }

    QString path = '"' + m_ui->pathEdit->text() + "\" " + m_ui->parametersEdit->text();
    process_ = new QProcess(this); // auto delete
    process_->start(path, QIODevice::ReadWrite|QIODevice::Text);
    if (process_->state() == QProcess::NotRunning){
        QMessageBox::critical(this, APPNAME, tr("Can not launch computer go program."));
        return;
    }
    gtp_ = new gtp(process_); // auto delete
    connect(gtp_, SIGNAL(initialized()), SLOT(on_gtp_initialized()));
    connect(gtp_, SIGNAL(gameEnded()), SLOT(on_gtp_ended()));
    connect(gtp_, SIGNAL(getName(const QString&)), SLOT(on_gtp_name(const QString&)));
    connect(gtp_, SIGNAL(getVersion(const QString&)), SLOT(on_gtp_version(const QString&)));
}

/**
* path edit changed
* get name button can be enabled if path is not empty.
*/
void EngineListDialog::on_pathEdit_textChanged(QString s){
    m_ui->getNameButton->setEnabled( s.isEmpty() == false );

    QTreeWidgetItem* item = m_ui->engineList->currentItem();
    if (item == NULL)
        return;
    item->setText(1, s);
}

/**
* parameters edit changed
*/
void EngineListDialog::on_parametersEdit_textChanged(QString s){
    QTreeWidgetItem* item = m_ui->engineList->currentItem();
    if (item == NULL)
        return;
    item->setText(2, s);
}

/**
* name canged
*/
void EngineListDialog::on_nameEdit_textChanged(QString s){
    QTreeWidgetItem* item = m_ui->engineList->currentItem();
    if (item == NULL)
        return;
    item->setText(0, s);
}

/**
* up button clicked
*/
void EngineListDialog::on_upButton_clicked(){
    QTreeWidgetItem* item = m_ui->engineList->currentItem();
    if (item == NULL)
        return;

    QModelIndex mi = m_ui->engineList->currentIndex();
    if (mi.row() == 0)
        return;

    m_ui->engineList->invisibleRootItem()->removeChild(item);
    m_ui->engineList->insertTopLevelItem(mi.row() - 1, item);
    m_ui->engineList->setCurrentItem(item);
}

/**
* down button clicked
*/
void EngineListDialog::on_downButton_clicked(){
    QTreeWidgetItem* item = m_ui->engineList->currentItem();
    if (item == NULL)
        return;

    QModelIndex mi = m_ui->engineList->currentIndex();
    if (mi.row() == m_ui->engineList->topLevelItemCount() - 1)
        return;

    m_ui->engineList->invisibleRootItem()->removeChild(item);
    m_ui->engineList->insertTopLevelItem(mi.row() + 1, item);
    m_ui->engineList->setCurrentItem(item);
}

/**
* nwe button clicked.
*/
void EngineListDialog::on_newButton_clicked(){
    QTreeWidgetItem* item = new QTreeWidgetItem( QStringList("(New Engine)") );
    m_ui->engineList->addTopLevelItem(item);
    m_ui->engineList->setCurrentItem(item);
}

/**
* delete button clicked
*/
void EngineListDialog::on_deleteButton_clicked(){
    QTreeWidgetItem* item = m_ui->engineList->currentItem();
    if (item == NULL)
        return;
    delete item;
}

void EngineListDialog::on_engineList_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous){
    m_ui->nameEdit->setEnabled(current != NULL);
    m_ui->browseButton->setEnabled(current != NULL);
    m_ui->pathEdit->setEnabled(current != NULL);
    m_ui->parametersEdit->setEnabled(current != NULL);

    if (current == NULL)
        return;

    m_ui->nameEdit->setText( current->text(0) );
    m_ui->pathEdit->setText( current->text(1) );
    m_ui->parametersEdit->setText( current->text(2) );
}

/**
* gtp class was initialized
*/
void EngineListDialog::on_gtp_initialized(){
    gtp_->name();
    gtp_->version();
    gtp_->abort();
}

/**
* engine was ended
*/
void EngineListDialog::on_gtp_ended(){
    disconnect(gtp_);
    gtp_ = NULL;
}

/**
* name received from gtp
*/
void EngineListDialog::on_gtp_name(const QString& name){
    m_ui->nameEdit->setText(name);
}

/**
* version received from gtp
*/
void EngineListDialog::on_gtp_version(const QString& version){
    m_ui->nameEdit->setText( m_ui->nameEdit->text() + ' ' + version );
}
