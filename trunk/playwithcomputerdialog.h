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
