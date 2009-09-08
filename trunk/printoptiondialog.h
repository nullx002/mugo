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

protected:
    void changeEvent(QEvent *e);

private:
    Ui::PrintOptionDialog *m_ui;
};

#endif // PRINTOPTIONDIALOG_H
