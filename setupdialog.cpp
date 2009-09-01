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

    QSettings settings;

    // category list
    m_ui->categoryList->addItem( tr("Board") );
    m_ui->categoryList->addItem( tr("Markers") );

    // board
    m_ui->boardTypeComboBox->addItem( tr("Bitmap") );
    m_ui->boardTypeComboBox->addItem( tr("Fill Color") );
    m_ui->boardTypeComboBox->setCurrentIndex( settings.value("board/boardType").toInt() );
    boardColor = settings.value("board/boardColor", BOARD_COLOR).value<QColor>();
    m_ui->boardColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(boardColor.red()).arg(boardColor.green()).arg(boardColor.blue()) );

    // white
    m_ui->whiteTypeComboBox->addItem( tr("Bitmap") );
    m_ui->whiteTypeComboBox->addItem( tr("Fill Color") );
    m_ui->whiteTypeComboBox->setCurrentIndex( settings.value("board/whiteType").toInt() );
    whiteColor = settings.value("board/whiteColor", WHITE_COLOR).value<QColor>();
    m_ui->whiteColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(whiteColor.red()).arg(whiteColor.green()).arg(whiteColor.blue()) );

    // black
    m_ui->blackTypeComboBox->addItem( tr("Bitmap") );
    m_ui->blackTypeComboBox->addItem( tr("Fill Color") );
    m_ui->blackTypeComboBox->setCurrentIndex( settings.value("board/blackType").toInt() );
    blackColor = settings.value("board/blackColor", BLACK_COLOR).value<QColor>();
    m_ui->blackColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(blackColor.red()).arg(blackColor.green()).arg(blackColor.blue()) );

    // bg
    bgColor = settings.value("board/bgColor", BG_COLOR).value<QColor>();
    m_ui->bgColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue()) );

    // tutor
    tutorColor = settings.value("board/bgTutorColor", BG_TUTOR_COLOR).value<QColor>();
    m_ui->bgTutorColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(tutorColor.red()).arg(tutorColor.green()).arg(tutorColor.blue()) );

    // last move
    focusColor = settings.value("board/focusColor", FOCUS_COLOR).value<QColor>();
    m_ui->focusColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(focusColor.red()).arg(focusColor.green()).arg(focusColor.blue()) );

    // branch
    branchColor = settings.value("board/branchColor", BRANCH_COLOR).value<QColor>();
    m_ui->branchColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(branchColor.red()).arg(branchColor.green()).arg(branchColor.blue()) );
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

    QSettings settings;

    // board
    settings.setValue("board/boardType", m_ui->boardTypeComboBox->currentIndex());
    settings.setValue("board/whiteType", m_ui->whiteTypeComboBox->currentIndex());
    settings.setValue("board/blackType", m_ui->blackTypeComboBox->currentIndex());
    settings.setValue("board/boardColor", boardColor);
    settings.setValue("board/whiteColor", whiteColor);
    settings.setValue("board/blackColor", blackColor);
    settings.setValue("board/bgColor", bgColor);
    settings.setValue("board/bgTutorColor", tutorColor);

    // last move
    settings.setValue("board/focusColor", focusColor);
    settings.setValue("board/branchColor", branchColor);
}

/**
* slot
*/
void SetupDialog::on_categoryList_currentRowChanged(int currentRow){
    m_ui->stackedWidget->setCurrentIndex(currentRow);
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
    if (dlg.exec() != QDialog::Accepted)
        return;
    blackColor = dlg.selectedColor();
    m_ui->blackColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(blackColor.red()).arg(blackColor.green()).arg(blackColor.blue()) );
}

/**
* slot
* background color button clicked
*/
void SetupDialog::on_bgColorButton_clicked(){
    QColorDialog dlg(bgColor, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    bgColor = dlg.selectedColor();
    m_ui->bgColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue()) );
}

/**
* slot
* background in tutor color button clicked
*/
void SetupDialog::on_bgTutorColorButton_clicked(){
    QColorDialog dlg(tutorColor, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    tutorColor = dlg.selectedColor();
    m_ui->bgTutorColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(tutorColor.red()).arg(tutorColor.green()).arg(tutorColor.blue()) );
}

/**
* slot
* focus color button clicked
*/
void SetupDialog::on_focusColorButton_clicked(){
    QColorDialog dlg(focusColor, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    focusColor = dlg.selectedColor();
    m_ui->focusColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(focusColor.red()).arg(focusColor.green()).arg(focusColor.blue()) );
}

/**
* slot
* branch color button clicked
*/
void SetupDialog::on_branchColorButton_clicked(){
    QColorDialog dlg(branchColor, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    branchColor = dlg.selectedColor();
    m_ui->branchColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(branchColor.red()).arg(branchColor.green()).arg(branchColor.blue()) );
}
