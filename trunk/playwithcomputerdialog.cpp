#include <QDebug>
#include <QFileDialog>
#include "playwithcomputerdialog.h"
#include "ui_playwithcomputerdialog.h"

PlayWithComputerDialog::PlayWithComputerDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::PlayWithComputerDialog)
{
    m_ui->setupUi(this);

    m_ui->colorComboBox->addItem( tr("Black") );
    m_ui->colorComboBox->addItem( tr("White") );

    m_ui->boardSizeComboBox->addItem( "19 x 19" );
    m_ui->boardSizeComboBox->addItem( "13 x 13" );
    m_ui->boardSizeComboBox->addItem( "9 x 9" );

    QDoubleValidator* dv = new QDoubleValidator(-20.0, 20.0, 2, this);
    dv->setNotation(QDoubleValidator::StandardNotation);
    m_ui->komiEdit->setValidator( dv );

    m_ui->handicapEdit->setValidator( new QIntValidator(0, 9, this) );

    m_ui->levelEdit->setValidator( new QIntValidator(1, 12, this) );
}

PlayWithComputerDialog::~PlayWithComputerDialog()
{
    delete m_ui;
}

void PlayWithComputerDialog::changeEvent(QEvent *e)
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

void PlayWithComputerDialog::accept(){
    QDialog::accept();

    isBlack = m_ui->colorComboBox->currentIndex() == 0;

    int boardSize = m_ui->boardSizeComboBox->currentIndex();
    size = boardSize == 0 ? 19 : (boardSize == 1 ? 13 : 9);

    path = m_ui->computerPathEdit->text();
    komi = m_ui->komiEdit->text().toDouble();
    handicap = m_ui->handicapEdit->text().toInt();
    level = m_ui->levelEdit->text().toInt();
}

/**
* browse computer go
*/
void PlayWithComputerDialog::on_computerPathBrowse_clicked(){
    QString fname = QFileDialog::getOpenFileName(this);
    if (fname.isEmpty())
        return;
    m_ui->computerPathEdit->setText(fname);
}
