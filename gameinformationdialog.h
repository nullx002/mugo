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
#ifndef GAMEINFORMATIONDIALOG_H
#define GAMEINFORMATIONDIALOG_H

#include <QtGui/QDialog>
#include "godata.h"

namespace Ui {
    class GameInformationDialog;
}

/**
  Edit Game Information
*/
class GameInformationDialog : public QDialog {
    Q_OBJECT
public:
    GameInformationDialog(QWidget* parent, Go::InformationPtr& info);
    ~GameInformationDialog();

protected:
    virtual void changeEvent(QEvent* e);
    virtual void accept();

private:
    Ui::GameInformationDialog* m_ui;
    Go::InformationPtr gameInfo;
};

#endif // GAMEINFORMATIONDIALOG_H
