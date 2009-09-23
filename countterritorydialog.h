#ifndef COUNTTERRITORYDIALOG_H
#define COUNTTERRITORYDIALOG_H

#include <QtGui/QDialog>
#include <QDebug>
#include "godata.h"

namespace Ui {
    class CountTerritoryDialog;
}

class CountTerritoryDialog : public QDialog {
    Q_OBJECT
public:
    CountTerritoryDialog(QWidget *parent = 0);
    ~CountTerritoryDialog();

    void setInformationNode(go::informationNode* infoNode){ informationNode = infoNode; }
    void setScore(int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory, double komi);

protected:
    virtual void changeEvent(QEvent *e);
    virtual void accept();

private:
    Ui::CountTerritoryDialog *m_ui;
    go::informationNode* informationNode;
    double scorej;
};

#endif // COUNTTERRITORYDIALOG_H
