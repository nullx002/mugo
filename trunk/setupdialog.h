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

    // board
    QColor boardColor, whiteColor, blackColor, bgColor, tutorColor;

    // markers
    QColor focusColor, branchColor;

private slots:
    void on_categoryList_currentRowChanged(int currentRow);

    // board
    void on_boardColorButton_clicked();
    void on_whiteColorButton_clicked();
    void on_blackColorButton_clicked();
    void on_bgColorButton_clicked();
    void on_bgTutorColorButton_clicked();

    // marker
    void on_focusColorButton_clicked();
    void on_branchColorButton_clicked();
};

#endif // SETUPDIALOG_H
