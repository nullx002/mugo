#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class SetupDialog;
}

class SetupDialog : public QDialog {
    Q_OBJECT
public:
    SetupDialog(QWidget *parent = 0);
    ~SetupDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SetupDialog *m_ui;
    QColor boardColor;

private slots:
    void on_boardColorButton_clicked();
};

#endif // SETUPDIALOG_H
