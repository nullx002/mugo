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
#ifndef BOARDSIZEDIALOG_H
#define BOARDSIZEDIALOG_H

#include <QDialog>

namespace Ui {
    class BoardSizeDialog;
}

/**
* Select board size.
*
* select board size using by radio buttons
*  19 x 19
*  13 x 13
*   9 x  9
* or input custom board size using by spin box
*  2 to 52
*/
class BoardSizeDialog : public QDialog {
    Q_OBJECT
public:
    BoardSizeDialog(QWidget *parent = 0);
    ~BoardSizeDialog();

    int size;

protected:
    virtual void changeEvent(QEvent *e);
    virtual void accept();

private:
    Ui::BoardSizeDialog *ui;

private slots:
    void on_radioCustomButton_toggled(bool checked);
};

#endif // BOARDSIZEDIALOG_H
