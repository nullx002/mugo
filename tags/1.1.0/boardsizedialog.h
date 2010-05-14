#ifndef BOARDSIZEDIALOG_H
#define BOARDSIZEDIALOG_H

#include <QDialog>

namespace Ui {
    class BoardSizeDialog;
}

/**
* Select board size.
*
* select board size using by radio buttons
*  19 x 19
*  13 x 13
*   9 x  9
* or input custom board size using by spin box
*  2 to 52
*/
class BoardSizeDialog : public QDialog {
    Q_OBJECT
public:
    BoardSizeDialog(QWidget *parent = 0);
    ~BoardSizeDialog();

    int size;

protected:
    virtual void changeEvent(QEvent *e);
    virtual void accept();

private:
    Ui::BoardSizeDialog *ui;

private slots:
    void on_radioCustomButton_toggled(bool checked);
};

#endif // BOARDSIZEDIALOG_H
