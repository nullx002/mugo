#include <QDebug>
#include "boardsizedialog.h"
#include "ui_boardsizedialog.h"

BoardSizeDialog::BoardSizeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BoardSizeDialog)
{
    ui->setupUi(this);
}

BoardSizeDialog::~BoardSizeDialog()
{
    delete ui;
}

void BoardSizeDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

/**
* slot
* ok button was clicked.
*/
void BoardSizeDialog::accept()
{
    QDialog::accept();
    if (ui->radio19Button->isChecked())
        size = 19;
    else if (ui->radio13Button->isChecked())
        size = 13;
    else if (ui->radio9Button->isChecked())
        size = 9;
    else
        size = ui->customSpinBox->value();
}

/**
* slot
* custom radio button was toggled.
*/
void BoardSizeDialog::on_radioCustomButton_toggled(bool checked)
{
    // if custom radio button is checked, enable custom spin button.
    ui->customSpinBox->setEnabled(checked);
}
