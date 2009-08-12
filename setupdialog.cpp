#include <QColorDialog>
#include "setupdialog.h"
#include "ui_setupdialog.h"

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SetupDialog)
{
    m_ui->setupUi(this);

    // board
    m_ui->boardTypeComboBox->addItem( tr("Default Board") );
    m_ui->boardTypeComboBox->addItem( tr("Fill Color") );

    boardColor = QColor(255, 200, 100);
    m_ui->boardColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(boardColor.red()).arg(boardColor.green()).arg(boardColor.blue()) );

    // white
    whiteColor = QColor(255, 255, 255);
    m_ui->whiteColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(whiteColor.red()).arg(whiteColor.green()).arg(whiteColor.blue()) );

    // black
    blackColor = QColor(0, 0, 0);
    m_ui->blackColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(blackColor.red()).arg(blackColor.green()).arg(blackColor.blue()) );
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

/**
* slot
* board color button clicked
*/
void SetupDialog::on_boardColorButton_clicked(){
    QColorDialog dlg(boardColor, this);
    dlg.exec();
}

/**
* slot
* white color button clicked
*/
void SetupDialog::on_whiteColorButton_clicked(){
    QColorDialog dlg(whiteColor, this);
    dlg.exec();
}

/**
* slot
* black color button clicked
*/
void SetupDialog::on_blackColorButton_clicked(){
    QColorDialog dlg(blackColor, this);
    dlg.exec();
}
