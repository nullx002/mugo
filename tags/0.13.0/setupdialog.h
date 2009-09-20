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
    QColor focusWhiteColor, focusBlackColor, branchColor;

private slots:
    void on_categoryList_currentRowChanged(int currentRow);

    // board
    void on_boardTypeComboBox_currentIndexChanged(int index);
    void on_boardColorButton_clicked();
    void on_boardPathButton_clicked();
    void on_bgColorButton_clicked();
    void on_bgTutorColorButton_clicked();

    // stones
    void on_whiteTypeComboBox_currentIndexChanged(int index);
    void on_whiteColorButton_clicked();
    void on_whitePathButton_clicked();
    void on_blackTypeComboBox_currentIndexChanged(int index);
    void on_blackColorButton_clicked();
    void on_blackPathButton_clicked();

    // marker
    void on_focusWhiteColorButton_clicked();
    void on_focusBlackColorButton_clicked();
    void on_branchColorButton_clicked();

    // sound
    void on_soundTypeComboBox_currentIndexChanged(int index);
    void on_soundPathButton_clicked();
};

#endif // SETUPDIALOG_H
