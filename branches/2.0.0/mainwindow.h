/*
    mugo, sgf editor.
    Copyright (C) 2009-2011 nsase.

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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUndoGroup>
#include "boardwidget.h"
#include "document.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // constructor, destructor
    explicit MainWindow(const QString& fname, QWidget *parent = 0);
    ~MainWindow();

public slots:
    // new, open, save, close
    bool fileNew(QTextCodec* codec=NULL, int xsize=19, int ysize=19, double komi=6.5, int handicap=0);
    bool fileOpen(const QString& fname, QTextCodec* codec=NULL, bool newTab=true);

protected:
    void changeEvent(QEvent* e);
    void closeEvent(QCloseEvent* e);

    // initialize
    void initializeMenu();

    // create new tab
    bool createNewTab(Document* doc);

private slots:
    void on_boardTabWidget_tabCloseRequested(int index);
    void on_boardTabWidget_currentChanged(QWidget*);

private:
    Ui::MainWindow *ui;

    QUndoGroup undoGroup;
    QAction* undoAction;
    QAction* redoAction;
};

#endif // MAINWINDOW_H
