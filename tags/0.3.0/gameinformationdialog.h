#ifndef GAMEINFORMATIONDIALOG_H
#define GAMEINFORMATIONDIALOG_H

#include <QtGui/QDialog>
#include "godata.h"

namespace Ui {
    class GameInformationDialog;
}

class GameInformationDialog : public QDialog {
    Q_OBJECT
public:
    GameInformationDialog(QWidget *parent, go::informationNode* infoNode);
    ~GameInformationDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::GameInformationDialog *m_ui;
    go::informationNode* gameInfo;

private slots:
    void onAccepted();
};

#endif // GAMEINFORMATIONDIALOG_H
