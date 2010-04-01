#ifndef PLAYWITHCOMPUTERDIALOG_H
#define PLAYWITHCOMPUTERDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class PlayWithComputerDialog;
}

class PlayWithComputerDialog : public QDialog {
    Q_OBJECT
public:
    PlayWithComputerDialog(QWidget *parent = 0);
    ~PlayWithComputerDialog();

    bool    isBlack;
    QString path;
    QString parameter;
    int     size;
    double  komi;
    int     handicap;
    int     level;

protected:
    virtual void changeEvent(QEvent *e);
    virtual void accept();

private:
    Ui::PlayWithComputerDialog *m_ui;

private slots:
    void on_computerPathBrowse_clicked();
};

#endif // PLAYWITHCOMPUTERDIALOG_H
