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
#ifndef EXPORTASCIIDIALOG_H
#define EXPORTASCIIDIALOG_H

#include <QtGui/QDialog>
#include "boardwidget.h"

namespace Ui {
    class ExportAsciiDialog;
}

class ExportAsciiDialog : public QDialog {
    Q_OBJECT
public:
    ExportAsciiDialog(QWidget *parent, const BoardWidget::BoardBuffer& buffer);
    ~ExportAsciiDialog();

    void createAscii(int index);
    void createEnglishAscii();
    void createJapaneseAscii(bool isMono);
    bool isStar(int x, int y);

public slots:
    virtual void accept();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ExportAsciiDialog *m_ui;
    const BoardWidget::BoardBuffer& boardBuffer;

private slots:
    void on_typeComboBox_currentIndexChanged(int index);
};

#endif // EXPORTASCIIDIALOG_H
