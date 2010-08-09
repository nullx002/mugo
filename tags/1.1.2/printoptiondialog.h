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
#ifndef PRINTOPTIONDIALOG_H
#define PRINTOPTIONDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class PrintOptionDialog;
}

class PrintOptionDialog : public QDialog {
    Q_OBJECT
public:
    PrintOptionDialog(QWidget *parent = 0);
    ~PrintOptionDialog();

    int printOption() const{ return printOption_; }
    int movesPerPage() const{ return movesPerPage_; }
    bool showCoordinate() const{ return showCoordinate_; }
    bool includeComments() const{ return includeComments_; }
    const QFont& font() const{ return font_; }
    const QString& headerLeftFormat() const{ return headerLeftFormat_; }
    const QString& headerCenterFormat() const{ return headerCenterFormat_; }
    const QString& headerRightFormat() const{ return headerRightFormat_; }
    const QString& footerLeftFormat() const{ return footerLeftFormat_; }
    const QString& footerCenterFormat() const{ return footerCenterFormat_; }
    const QString& footerRightFormat() const{ return footerRightFormat_; }

protected:
    void changeEvent(QEvent *e);

public slots:
    virtual void accept();

private:
    Ui::PrintOptionDialog *m_ui;
    int printOption_;
    int movesPerPage_;
    bool  showCoordinate_;
    bool  includeComments_;
    QFont font_;
    QString headerLeftFormat_;
    QString headerCenterFormat_;
    QString headerRightFormat_;
    QString footerLeftFormat_;
    QString footerCenterFormat_;
    QString footerRightFormat_;
};

#endif // PRINTOPTIONDIALOG_H
