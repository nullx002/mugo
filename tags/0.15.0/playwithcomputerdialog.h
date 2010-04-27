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
    QString path;
    QString parameter;
    int     size;
    double  komi;
    int     handicap;
    int     level;

    // start position
    int     startPosition;

protected:
    virtual void changeEvent(QEvent *e);
    virtual void accept();

private:
    Ui::PlayWithComputerDialog *m_ui;

private slots:
    void on_computerPathBrowse_clicked();
    void on_newGame_toggled(bool checked);
    void on_resume_toggled(bool checked);
};

#endif // PLAYWITHCOMPUTERDIALOG_H
