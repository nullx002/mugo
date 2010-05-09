#ifndef ENGINELISTDIALOG_H
#define ENGINELISTDIALOG_H

#include <QtGui/QDialog>
#include <QProcess>
#include <QTreeWidgetItem>
#include "gtp.h"

namespace Ui {
    class EngineListDialog;
}


class EngineListDialog : public QDialog {
    Q_OBJECT
public:
    EngineListDialog(QWidget *parent = 0);
    ~EngineListDialog();

    virtual void accept();

private:
    Ui::EngineListDialog *m_ui;
    QProcess* process_;
    gtp* gtp_;

private slots:
    void on_getNameButton_clicked();
    void on_browseButton_clicked();
    void on_pathEdit_textChanged(QString);
    void on_parametersEdit_textChanged(QString);
    void on_nameEdit_textChanged(QString);
    void on_upButton_clicked();
    void on_downButton_clicked();
    void on_newButton_clicked();
    void on_deleteButton_clicked();
    void on_engineList_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void on_gtp_initialized();
    void on_gtp_ended();
    void on_gtp_name(const QString& name);
    void on_gtp_version(const QString& name);
};

#endif // ENGINELISTDIALOG_H
