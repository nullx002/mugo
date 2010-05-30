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
#include <QSettings>
#include "appdef.h"
#include "playwithcomputerdialog.h"
#include "enginelistdialog.h"
#include "enginelist.h"
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
    m_ui->komiSpinBox->setValue( settings.value("playWithComputer/komi", 6.5).toDouble() );
    m_ui->handicapSpinBox->setValue( settings.value("playWithComputer/handicap").toInt() );
    m_ui->levelSpinBox->setValue( settings.value("playWithComputer/level", 10).toInt() );

    updateEngineList();
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

    // game settings
    isBlack = m_ui->colorComboBox->currentIndex() == 0;
    int boardSize = m_ui->boardSizeComboBox->currentIndex();
    size = boardSize == 0 ? 19 : (boardSize == 1 ? 13 : 9);
    komi = m_ui->komiSpinBox->value();
    handicap = m_ui->handicapSpinBox->value();
    level = m_ui->levelSpinBox->value();

    // engine settings
    int engine = m_ui->engineComboBox->currentIndex();
    EngineList engineList;
    engineList.load();
    if (engine >= 0 && engine < engineList.engines.size()){
        name = engineList.engines[engine].name;
        path = engineList.engines[engine].path;
        parameters = engineList.engines[engine].parameters;
    }

    // start position
    startPosition = m_ui->newGame->isChecked() ? eNewGame : eContinueGame;

    // save dialog settings for ini file(or registry)
    QSettings settings;
    settings.setValue("playWithComputer/color", m_ui->colorComboBox->currentIndex());
    settings.setValue("playWithComputer/size", m_ui->boardSizeComboBox->currentIndex());
    settings.setValue("playWithComputer/komi", komi);
    settings.setValue("playWithComputer/handicap", handicap);
    settings.setValue("playWithComputer/level", level);
    settings.setValue("playWithComputer/defaultEngine", engine);
}

/**
* edit engines button is clicked.
* show manage engine dialog.
*/
void PlayWithComputerDialog::on_editEngineButton_clicked(){
    EngineListDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    updateEngineList();
}

/**
* new game radio button is toggled.
*/
void PlayWithComputerDialog::on_newGame_toggled(bool checked){
    m_ui->boardSizeComboBox->setEnabled(checked);
    m_ui->komiSpinBox->setEnabled(checked);
    m_ui->handicapSpinBox->setEnabled(checked);
}

/**
* resume radio button is toggled.
*/
void PlayWithComputerDialog::on_resume_toggled(bool checked){
    m_ui->boardSizeComboBox->setEnabled(checked == false);
    m_ui->komiSpinBox->setEnabled(checked == false);
    m_ui->handicapSpinBox->setEnabled(checked == false);
}

/**
*/
void PlayWithComputerDialog::updateEngineList(){
    m_ui->engineComboBox->clear();

    EngineList engineList;
    engineList.load();
    foreach(const Engine& e, engineList.engines){
        m_ui->engineComboBox->addItem( e.name );
    }

    int index = QSettings().value("playWithComputer/defaultEngine", 0).toInt();
    if (index < 0 || index >= m_ui->engineComboBox->count())
        index = 0;
    if (m_ui->engineComboBox->count() > 0)
        m_ui->engineComboBox->setCurrentIndex(index);
}
