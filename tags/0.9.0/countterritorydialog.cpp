#include "countterritorydialog.h"
#include "ui_countterritorydialog.h"

CountTerritoryDialog::CountTerritoryDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::CountTerritoryDialog)
{
    m_ui->setupUi(this);
}

CountTerritoryDialog::~CountTerritoryDialog()
{
    delete m_ui;
}

void CountTerritoryDialog::changeEvent(QEvent *e)
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

void CountTerritoryDialog::done(int r){
    QDialog::done(r);
    emit dialogClosed();
}

void CountTerritoryDialog::setScoreText(const QString& text)
{
    m_ui->scoreTextEdit->setPlainText(text);
}
