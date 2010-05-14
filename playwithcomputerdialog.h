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
#ifndef PLAYWITHCOMPUTERDIALOG_H
#define PLAYWITHCOMPUTERDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class PlayWithComputerDialog;
}

/**
    setting dialog for play with go program.
*/
class PlayWithComputerDialog : public QDialog {
    Q_OBJECT
public:
    enum StartPosition{
            eNewGame,       ///< start new game on new tab.
            eContinueGame,  ///< resume game on current tab.
    };

    PlayWithComputerDialog(QWidget *parent = 0);
    ~PlayWithComputerDialog();

    // game settings
    bool    isBlack;
    int     size;
    double  komi;
    int     handicap;
    int     level;

    // engine settings
    QString name;
    QString path;
    QString parameters;

    // start position
    int     startPosition;

protected:
    virtual void changeEvent(QEvent *e);
    virtual void accept();

private:
    void updateEngineList();

    Ui::PlayWithComputerDialog *m_ui;

private slots:
    void on_editEngineButton_clicked();
    void on_newGame_toggled(bool checked);
    void on_resume_toggled(bool checked);
};

#endif // PLAYWITHCOMPUTERDIALOG_H
