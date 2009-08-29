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
    void accept();

private:
    Ui::SetupDialog *m_ui;
    QColor boardColor, whiteColor, blackColor;

private slots:
    void on_categoryList_currentRowChanged(int currentRow);
    void on_blackColorButton_clicked();
    void on_whiteColorButton_clicked();
    void on_boardColorButton_clicked();
};

#endif // SETUPDIALOG_H
