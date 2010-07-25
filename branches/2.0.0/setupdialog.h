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
#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>

namespace Ui {
    class SetupDialog;
}

class SetupDialog : public QDialog {
    Q_OBJECT
public:
    SetupDialog(QWidget *parent = 0);
    ~SetupDialog();

protected:
    void accept();

private:
    Ui::SetupDialog *m_ui;

    // board
    QColor boardColor, whiteColor, blackColor, coordinateColor, bgColor, tutorColor;

    // markers
    QColor focusWhiteColor, focusBlackColor, branchColor;

private slots:
    void on_categoryList_currentRowChanged(int currentRow);

    // board
    void on_boardTypeComboBox_currentIndexChanged(int index);
    void on_boardColorButton_clicked();
    void on_boardPathButton_clicked();
    void on_coordinateColorButton_clicked();
    void on_bgColorButton_clicked();
    void on_bgTutorColorButton_clicked();

    // stones
    void on_whiteTypeComboBox_currentIndexChanged(int index);
    void on_whiteColorButton_clicked();
    void on_whitePathButton_clicked();
    void on_blackTypeComboBox_currentIndexChanged(int index);
    void on_blackColorButton_clicked();
    void on_blackPathButton_clicked();

    // marker
    void on_focusWhiteColorButton_clicked();
    void on_focusBlackColorButton_clicked();
    void on_branchColorButton_clicked();

    // sound
    void on_soundTypeComboBox_currentIndexChanged(int index);
    void on_soundPathButton_clicked();
};

#endif // SETUPDIALOG_H
