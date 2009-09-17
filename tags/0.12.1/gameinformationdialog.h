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
    virtual void changeEvent(QEvent* e);
    virtual void accept();

private:
    Ui::GameInformationDialog *m_ui;
    go::informationNode* gameInfo;
};

#endif // GAMEINFORMATIONDIALOG_H
