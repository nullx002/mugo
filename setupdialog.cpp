#include <QDebug>
#include <QColorDialog>
#include <QSettings>
#include "setupdialog.h"
#include "ui_setupdialog.h"
#include "appdef.h"

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SetupDialog)
{
    m_ui->setupUi(this);

    QSettings settings(AUTHOR, APPNAME);

    // board
    m_ui->boardTypeComboBox->addItem( tr("Bitmap") );
    m_ui->boardTypeComboBox->addItem( tr("Fill Color") );
    m_ui->boardTypeComboBox->setCurrentIndex( settings.value("boardType").toInt() );
    boardColor = settings.value("boardColor").value<QColor>();
    m_ui->boardColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(boardColor.red()).arg(boardColor.green()).arg(boardColor.blue()) );

    // white
    m_ui->whiteTypeComboBox->addItem( tr("Bitmap") );
    m_ui->whiteTypeComboBox->addItem( tr("Fill Color") );
    m_ui->whiteTypeComboBox->setCurrentIndex( settings.value("whiteType").toInt() );
    whiteColor = settings.value("whiteColor").value<QColor>();
    m_ui->whiteColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(whiteColor.red()).arg(whiteColor.green()).arg(whiteColor.blue()) );

    // black
    m_ui->blackTypeComboBox->addItem( tr("Bitmap") );
    m_ui->blackTypeComboBox->addItem( tr("Fill Color") );
    m_ui->blackTypeComboBox->setCurrentIndex( settings.value("blackType").toInt() );
    blackColor = settings.value("blackColor").value<QColor>();
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

void SetupDialog::accept(){
    QDialog::accept();

    QSettings settings(AUTHOR, APPNAME);
    settings.setValue("boardType", m_ui->boardTypeComboBox->currentIndex());
    settings.setValue("whiteType", m_ui->whiteTypeComboBox->currentIndex());
    settings.setValue("blackType", m_ui->blackTypeComboBox->currentIndex());
    settings.setValue("boardColor", boardColor);
    settings.setValue("whiteColor", whiteColor);
    settings.setValue("blackColor", blackColor);
}

/**
* slot
* board color button clicked
*/
void SetupDialog::on_boardColorButton_clicked(){
    QColorDialog dlg(boardColor, this);
    dlg.setCurrentColor(boardColor);
    if (dlg.exec() != QDialog::Accepted)
        return;
    boardColor = dlg.selectedColor();
    m_ui->boardColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(boardColor.red()).arg(boardColor.green()).arg(boardColor.blue()) );
}

/**
* slot
* white color button clicked
*/
void SetupDialog::on_whiteColorButton_clicked(){
    QColorDialog dlg(whiteColor, this);
    dlg.setCurrentColor(whiteColor);
    if (dlg.exec() != QDialog::Accepted)
        return;
    whiteColor = dlg.selectedColor();
    m_ui->whiteColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(whiteColor.red()).arg(whiteColor.green()).arg(whiteColor.blue()) );
}

/**
* slot
* black color button clicked
*/
void SetupDialog::on_blackColorButton_clicked(){
    QColorDialog dlg(blackColor, this);
    dlg.setCurrentColor(blackColor);
    if (dlg.exec() != QDialog::Accepted)
        return;
    blackColor = dlg.selectedColor();
    m_ui->blackColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(blackColor.red()).arg(blackColor.green()).arg(blackColor.blue()) );
}
