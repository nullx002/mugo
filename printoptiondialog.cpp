#include "printoptiondialog.h"
#include "ui_printoptiondialog.h"

PrintOptionDialog::PrintOptionDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::PrintOptionDialog)
{
    m_ui->setupUi(this);
}

PrintOptionDialog::~PrintOptionDialog()
{
    delete m_ui;
}

void PrintOptionDialog::changeEvent(QEvent *e)
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
