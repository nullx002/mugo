#include <QDebug>
#include <QSettings>
#include "appdef.h"
#include "playwithcomputerdialog.h"
#include "ui_playwithcomputerdialog.h"

/**
* constructor
*/
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

    // set default value from initial file
    QSettings settings;
    m_ui->colorComboBox->setCurrentIndex( settings.value("playWithComputer/color").toInt() );
    m_ui->boardSizeComboBox->setCurrentIndex( settings.value("playWithComputer/size").toInt() );
    m_ui->computerPathEdit->setText( settings.value("playWithComputer/path").toString() );
    m_ui->komiSpinBox->setValue( settings.value("playWithComputer/komi", 6.5).toDouble() );
    m_ui->handicapSpinBox->setValue( settings.value("playWithComputer/handicap").toInt() );
    m_ui->levelSpinBox->setValue( settings.value("playWithComputer/level", 10).toInt() );
}

/**
* destructor
*/
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

/**
* event handler
* ok button was pushed.
*/
void PlayWithComputerDialog::accept(){
    QDialog::accept();

    // path of go program
    path = m_ui->computerPathEdit->text();

    // game settings
    isBlack = m_ui->colorComboBox->currentIndex() == 0;
    int boardSize = m_ui->boardSizeComboBox->currentIndex();
    size = boardSize == 0 ? 19 : (boardSize == 1 ? 13 : 9);
    komi = m_ui->komiSpinBox->value();
    handicap = m_ui->handicapSpinBox->value();
    level = m_ui->levelSpinBox->value();
    parameter = m_ui->parameterEdit->text();

    // start position
    startPosition = m_ui->newGame->isChecked() ? eNewGame : eContinueGame;

    // save dialog settings for ini file(or registry)
    QSettings settings;
    settings.setValue("playWithComputer/color", m_ui->colorComboBox->currentIndex());
    settings.setValue("playWithComputer/size", m_ui->boardSizeComboBox->currentIndex());
    settings.setValue("playWithComputer/path", path);
    settings.setValue("playWithComputer/komi", komi);
    settings.setValue("playWithComputer/handicap", handicap);
    settings.setValue("playWithComputer/level", level);
}

/**
* browse go program.
*/
void PlayWithComputerDialog::on_computerPathBrowse_clicked(){
    QString fname = getOpenFileName(this);
    if (fname.isEmpty())
        return;
    m_ui->computerPathEdit->setText(fname);
}

void PlayWithComputerDialog::on_newGame_toggled(bool checked)
{
    m_ui->boardSizeComboBox->setEnabled(true);
    m_ui->komiSpinBox->setEnabled(true);
    m_ui->handicapSpinBox->setEnabled(true);
}

void PlayWithComputerDialog::on_resume_toggled(bool checked)
{
    m_ui->boardSizeComboBox->setEnabled(false);
    m_ui->komiSpinBox->setEnabled(false);
    m_ui->handicapSpinBox->setEnabled(false);
}
