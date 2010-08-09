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
    QTreeWidgetItem* analysis_;

private slots:
    void on_getNameButton_clicked();
    void on_browseButton_clicked();
    void on_pathEdit_textChanged(QString);
    void on_parametersEdit_textChanged(QString);
    void on_nameEdit_textChanged(QString);
    void on_analysisCheckBox_toggled(bool checked);
    void on_upButton_clicked();
    void on_downButton_clicked();
    void on_newButton_clicked();
    void on_deleteButton_clicked();
    void on_engineList_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void on_gtp_ended();
    void on_gtp_name(const QString& name);
    void on_gtp_version(const QString& name);
};

#endif // ENGINELISTDIALOG_H
