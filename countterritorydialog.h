#ifndef COUNTTERRITORYDIALOG_H
#define COUNTTERRITORYDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class CountTerritoryDialog;
}

class CountTerritoryDialog : public QDialog {
    Q_OBJECT
public:
    CountTerritoryDialog(QWidget *parent = 0);
    ~CountTerritoryDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::CountTerritoryDialog *m_ui;
};

#endif // COUNTTERRITORYDIALOG_H
