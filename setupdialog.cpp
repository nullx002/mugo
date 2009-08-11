#include <QColorDialog>
#include "setupdialog.h"
#include "ui_setupdialog.h"

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SetupDialog)
{
    m_ui->setupUi(this);

    boardColor = QColor(255, 200, 100);
    m_ui->boardColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(boardColor.red()).arg(boardColor.green()).arg(boardColor.blue()) );
}

SetupDialog::~SetupDialog()
{
    delete m_ui;
}

void SetupDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

// slot
// board color button clicked
void SetupDialog::on_boardColorButton_clicked(){
    QColorDialog dlg(boardColor, this);
    dlg.exec();
}
