/*
    mugo, sgf editor.
    Copyright (C) 2009-2010 nsase.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QDebug>
#include <QColorDialog>
#include <QSettings>
#include <QStyleFactory>
#include "appdef.h"
#include "mugoapp.h"
#include "setupdialog.h"
#include "ui_setupdialog.h"
#include "appdef.h"
#ifdef Q_WS_WIN
#  include "qtdotnetstyle.h"
#endif

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SetupDialog)
{
    m_ui->setupUi(this);

    QSettings settings;

    // board/board
    m_ui->boardTypeComboBox->setCurrentIndex( settings.value("board/boardType").toInt() );
    boardColor = settings.value("board/boardColor", BOARD_COLOR).value<QColor>();
    m_ui->boardColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(boardColor.red()).arg(boardColor.green()).arg(boardColor.blue()) );
    m_ui->boardPathEdit->setText( settings.value("board/boardPath").toString() );

    // board/coordinat ecolor
    coordinateColor = settings.value("board/coordinateColor", COORDINATE_COLOR).value<QColor>();
    m_ui->coordinateColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(coordinateColor.red()).arg(coordinateColor.green()).arg(coordinateColor.blue()) );

    // board/background
    bgColor = settings.value("board/bgColor", BG_COLOR).value<QColor>();
    m_ui->bgColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue()) );

    // board/bg in tutor
    tutorColor = settings.value("board/bgTutorColor", BG_TUTOR_COLOR).value<QColor>();
    m_ui->bgTutorColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(tutorColor.red()).arg(tutorColor.green()).arg(tutorColor.blue()) );

    // stones/white
    m_ui->whiteTypeComboBox->setCurrentIndex( settings.value("board/whiteType").toInt() );
    whiteColor = settings.value("board/whiteColor", WHITE_COLOR).value<QColor>();
    m_ui->whiteColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(whiteColor.red()).arg(whiteColor.green()).arg(whiteColor.blue()) );
    m_ui->whitePathEdit->setText( settings.value("board/whitePath").toString() );

    // stones/black
    m_ui->blackTypeComboBox->setCurrentIndex( settings.value("board/blackType").toInt() );
    blackColor = settings.value("board/blackColor", BLACK_COLOR).value<QColor>();
    m_ui->blackColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(blackColor.red()).arg(blackColor.green()).arg(blackColor.blue()) );
    m_ui->blackPathEdit->setText( settings.value("board/blackPath").toString() );

    // markers/focus
    m_ui->focusTypeComboBox->setCurrentIndex( settings.value("marker/focusType").toInt() );
    focusWhiteColor = settings.value("marker/focusWhiteColor", FOCUS_WHITE_COLOR).value<QColor>();
    m_ui->focusWhiteColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(focusWhiteColor.red()).arg(focusWhiteColor.green()).arg(focusWhiteColor.blue()) );
    focusBlackColor = settings.value("marker/focusBlackColor", FOCUS_BLACK_COLOR).value<QColor>();
    m_ui->focusBlackColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(focusBlackColor.red()).arg(focusBlackColor.green()).arg(focusBlackColor.blue()) );

    // markers/branch
    branchColor = settings.value("marker/branchColor", BRANCH_COLOR).value<QColor>();
    m_ui->branchColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(branchColor.red()).arg(branchColor.green()).arg(branchColor.blue()) );

    // markers/label
    m_ui->labelTypeComboBox->setCurrentIndex( settings.value("marker/labelType").toInt() );

    // navigation
    m_ui->stepsOfFastMoveSpinBox->setValue( settings.value("navigation/stepsOfFastMove", FAST_MOVE_STEPS).toInt() );
    m_ui->reproductionSpeedSpinBox->setValue( settings.value("navigation/autoReplayInterval", AUTO_REPLAY_INTERVAL).toInt() );

    // sound
    m_ui->soundTypeComboBox->setCurrentIndex( settings.value("sound/type").toInt() );
    m_ui->soundPathEdit->setText( settings.value("sound/path").toString() );

    // save name
    m_ui->saveNameEdit->setText( settings.value("saveName", SAVE_NAME).toString() );

    // encoding
    QString defaultCodec = settings.value("codec" ,"UTF-8").toString();
    for (int i=0; i<codecNames.size(); ++i){
        m_ui->defaultEncodingComboBox->addItem( codecActions[i]->text() );
        if (defaultCodec == codecNames[i])
            m_ui->defaultEncodingComboBox->setCurrentIndex( m_ui->defaultEncodingComboBox->count() - 1 );
    }

    // window style
    m_ui->windowStyleList->addItem( tr("Default") );
    m_ui->windowStyleList->addItems( QStyleFactory::keys() );
    QString style = settings.value("style").toString();
    if (style.isEmpty())
        m_ui->windowStyleList->setCurrentRow(0);
    else
        for (int i=0; i<m_ui->windowStyleList->count(); ++i)
            if (m_ui->windowStyleList->item(i)->text() == style){
                m_ui->windowStyleList->setCurrentRow(i);
                break;
            }
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
    settings.setValue("board/boardColor", boardColor);
    settings.setValue("board/boardPath", m_ui->boardPathEdit->text());
    settings.setValue("board/coordinateColor", coordinateColor);
    settings.setValue("board/bgColor", bgColor);
    settings.setValue("board/bgTutorColor", tutorColor);

    // stones
    settings.setValue("board/whiteType", m_ui->whiteTypeComboBox->currentIndex());
    settings.setValue("board/whiteColor", whiteColor);
    settings.setValue("board/whitePath", m_ui->whitePathEdit->text());

    settings.setValue("board/blackType", m_ui->blackTypeComboBox->currentIndex());
    settings.setValue("board/blackColor", blackColor);
    settings.setValue("board/blackPath", m_ui->blackPathEdit->text());

    // markers
    settings.setValue("marker/focusType", m_ui->focusTypeComboBox->currentIndex());
    settings.setValue("marker/focusWhiteColor", focusWhiteColor);
    settings.setValue("marker/focusBlackColor", focusBlackColor);
    settings.setValue("marker/branchColor", branchColor);
    settings.setValue("marker/labelType", m_ui->labelTypeComboBox->currentIndex());

    // navigation
    settings.setValue("navigation/stepsOfFastMove", m_ui->stepsOfFastMoveSpinBox->value());
    settings.setValue("navigation/autoReplayInterval", m_ui->reproductionSpeedSpinBox->value());

    // sound
    settings.setValue("sound/type", m_ui->soundTypeComboBox->currentIndex());
    settings.setValue("sound/path", m_ui->soundPathEdit->text());

    // save name
    settings.setValue("saveName", m_ui->saveNameEdit->text());

    // encoding
    int codec = m_ui->defaultEncodingComboBox->currentIndex();
    settings.setValue("codec", codecNames[codec]);

    // window style
    QString oldStyle = settings.value("style").toString();
    QString newStyle = m_ui->windowStyleList->currentItem()->text();
    int style = m_ui->windowStyleList->currentRow();
    if (style == 0)
        settings.remove("style");
    else
        settings.setValue("style", newStyle);
    qobject_cast<Application*>(qApp)->setWindowStyle(style ? newStyle : "");
}

/**
* slot
*/
void SetupDialog::on_categoryList_currentRowChanged(int currentRow){
    m_ui->stackedWidget->setCurrentIndex(currentRow);
}

/**
* slot
* board type is changed
*/
void SetupDialog::on_boardTypeComboBox_currentIndexChanged(int index){
    m_ui->boardPathEdit->setEnabled(index == 1);
    m_ui->boardPathButton->setEnabled(index == 1);
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
* board path borwse button is clicked
*/
void SetupDialog::on_boardPathButton_clicked(){
    QString fname = getOpenFileName(this, QString(), QString(), tr("All Image Files(*.bmp *.gif *.jpg *.jpeg *.png *.tif *.tiff);;All Files(*.*)"));
    if (!fname.isEmpty())
        m_ui->boardPathEdit->setText(fname);
}

/**
* slot
* coordinate color button clicked
*/
void SetupDialog::on_coordinateColorButton_clicked(){
    QColorDialog dlg(coordinateColor, this);
    dlg.setCurrentColor(coordinateColor);
    if (dlg.exec() != QDialog::Accepted)
        return;
    coordinateColor = dlg.selectedColor();
    m_ui->coordinateColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(coordinateColor.red()).arg(coordinateColor.green()).arg(coordinateColor.blue()) );
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
* white stone type is changed
*/
void SetupDialog::on_whiteTypeComboBox_currentIndexChanged(int index){
    m_ui->whitePathEdit->setEnabled(index == 1);
    m_ui->whitePathButton->setEnabled(index == 1);
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
* white stone type is changed
*/
void SetupDialog::on_whitePathButton_clicked(){
    QString fname = getOpenFileName(this, QString(), QString(), tr("All Image Files(*.bmp *.gif *.jpg *.jpeg *.png *.tif *.tiff);;All Files(*.*)"));
    if (!fname.isEmpty())
        m_ui->whitePathEdit->setText(fname);
}

/**
* slot
* black stone type is changed
*/
void SetupDialog::on_blackTypeComboBox_currentIndexChanged(int index){
    m_ui->blackPathEdit->setEnabled(index == 1);
    m_ui->blackPathButton->setEnabled(index == 1);
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
* black stone type is changed
*/
void SetupDialog::on_blackPathButton_clicked(){
    QString fname = getOpenFileName(this, QString(), QString(), tr("All Image Files(*.bmp *.gif *.jpg *.jpeg *.png *.tif *.tiff);;All Files(*.*)"));
    if (!fname.isEmpty())
        m_ui->blackPathEdit->setText(fname);
}

/**
* slot
* focus color (white stone) button clicked
*/
void SetupDialog::on_focusWhiteColorButton_clicked(){
    QColorDialog dlg(focusWhiteColor, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    focusWhiteColor = dlg.selectedColor();
    m_ui->focusWhiteColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(focusWhiteColor.red()).arg(focusWhiteColor.green()).arg(focusWhiteColor.blue()) );
}

/**
* slot
* focus color (black stone) button clicked
*/
void SetupDialog::on_focusBlackColorButton_clicked(){
    QColorDialog dlg(focusBlackColor, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    focusBlackColor = dlg.selectedColor();
    m_ui->focusBlackColorButton->setStyleSheet( QString("border:1px solid black; background-color:rgb(%1, %2, %3)").arg(focusBlackColor.red()).arg(focusBlackColor.green()).arg(focusBlackColor.blue()) );
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

/**
* slot
* sound type is changed
*/
void SetupDialog::on_soundTypeComboBox_currentIndexChanged(int index){
    m_ui->soundPathEdit->setEnabled(index == 1);
    m_ui->soundPathButton->setEnabled(index == 1);
}

/**
* slot
* sound path browse button
*/
void SetupDialog::on_soundPathButton_clicked(){
    QString fname = getOpenFileName(this, QString(), QString(), tr("Sound Files(*.wav *.mp3);;All Files(*.*)"));
    if (!fname.isEmpty())
        m_ui->soundPathEdit->setText(fname);
}
