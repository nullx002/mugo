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
#ifndef COUNTTERRITORYDIALOG_H
#define COUNTTERRITORYDIALOG_H

#include <QtGui/QDialog>
#include <QDebug>
#include "godata.h"

namespace Ui {
    class CountTerritoryDialog;
}

class CountTerritoryDialog : public QDialog {
    Q_OBJECT
public:
    CountTerritoryDialog(QWidget *parent = 0);
    ~CountTerritoryDialog();

    void setInformationNode(go::informationNode* infoNode){ informationNode = infoNode; }
    void setScore(int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory, double komi);

protected:
    virtual void changeEvent(QEvent *e);
    virtual void done(int r);
    virtual void accept();

private:
    Ui::CountTerritoryDialog *m_ui;
    go::informationNode* informationNode;
    double scorej;
};

#endif // COUNTTERRITORYDIALOG_H
