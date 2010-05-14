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
#ifndef SAVEIMAGEDIALOG_H
#define SAVEIMAGEDIALOG_H

#include <QtGui/QDialog>
#include <QFileInfo>

namespace Ui {
    class SaveImageDialog;
}

/**
* export as image dialog
*/
class SaveImageDialog : public QDialog {
    Q_OBJECT
public:
    SaveImageDialog(QWidget *parent = 0);
    ~SaveImageDialog();

    virtual void accept();

    QFileInfo fileInfo;
    int       imageSize;
    bool      showCoordinate;
    bool      monochrome;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SaveImageDialog *m_ui;

private slots:
    void on_fileNameEdit_textChanged(QString );
    void on_fileBrowseButton_clicked();
};

#endif // SAVEIMAGEDIALOG_H
