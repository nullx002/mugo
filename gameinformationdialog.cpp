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
#include "gameinformationdialog.h"
#include "ui_gameinformationdialog.h"
#include "sgfcommand.h"

/**
  Constructs game information dialog
*/
GameInformationDialog::GameInformationDialog(QWidget *parent, GoDocument* doc, Go::NodePtr node)
    : QDialog(parent)
    , m_ui(new Ui::GameInformationDialog)
    , document_(doc)
    , node_(node)
{
    m_ui->setupUi(this);

    // white player
    m_ui->whitePlayer->setText( node_->information()->whitePlayer() );
    m_ui->whiteRank->setText( node_->information()->whiteRank() );
    m_ui->whiteTeam->setText( node_->information()->whiteTeam() );

    // black player
    m_ui->blackPlayer->setText( node_->information()->blackPlayer() );
    m_ui->blackRank->setText( node_->information()->blackRank() );
    m_ui->blackTeam->setText( node_->information()->blackTeam() );

    // rule
    m_ui->komi->setText( QString("%1").arg(node_->information()->komi()) );
    m_ui->handicap->setText( QString("%1").arg(node_->information()->handicap()) );
    m_ui->result->setText( node_->information()->result() );
    m_ui->time->setText( node_->information()->time() );
    m_ui->overtime->setText( node_->information()->overtime() );
    m_ui->rule->setText( node_->information()->rule() );

    // when / where
    m_ui->gameName->setText( node_->information()->gameName() );
    m_ui->date->setText( node_->information()->date() );
    m_ui->round->setText( node_->information()->round() );
    m_ui->place->setText( node_->information()->place() );
    m_ui->event->setText( node_->information()->event() );

    // other
    m_ui->annotation->setText( node_->information()->annotation() );
    m_ui->source->setText( node_->information()->source() );
    m_ui->gameComment->setPlainText( node_->information()->gameComment() );
    m_ui->copyright->setPlainText( node_->information()->copyright() );
    m_ui->user->setText( node_->information()->user() );
    m_ui->opening->setText( node_->information()->opening() );
}

/**
  Destructs dialog
*/
GameInformationDialog::~GameInformationDialog()
{
    delete m_ui;
}

void GameInformationDialog::changeEvent(QEvent *e)
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
  clicked OK button
*/
void GameInformationDialog::accept()
{
    QDialog::accept();

    Go::InformationPtr temp( new Go::Information );

    // white player
    temp->setWhitePlayer( m_ui->whitePlayer->text() );
    temp->setWhiteRank( m_ui->whiteRank->text() );
    temp->setWhiteTeam( m_ui->whiteTeam->text() );

    // black player
    temp->setBlackPlayer( m_ui->blackPlayer->text() );
    temp->setBlackRank( m_ui->blackRank->text() );
    temp->setBlackTeam( m_ui->blackTeam->text() );

    // rule
    temp->setKomi(m_ui->komi->text().toDouble() );
    temp->setHandicap( m_ui->handicap->text().toInt() );
    temp->setResult( m_ui->result->text() );
    temp->setTime( m_ui->time->text() );
    temp->setOvertime( m_ui->overtime->text() );
    temp->setRule( m_ui->rule->text() );

    // when / where
    temp->setGameName( m_ui->gameName->text() );
    temp->setDate( m_ui->date->text() );
    temp->setRound( m_ui->round->text() );
    temp->setPlace( m_ui->place->text() );
    temp->setEvent( m_ui->event->text() );

    // other
    temp->setAnnotation( m_ui->annotation->text() );
    temp->setSource( m_ui->source->text() );
    temp->setGameComment( m_ui->gameComment->toPlainText() );
    temp->setCopyright( m_ui->copyright->toPlainText() );
    temp->setUser( m_ui->user->text() );
    temp->setOpening( m_ui->opening->text() );

    // process command
    document_->undoStack()->push( new SetGameInformationCommand(document_, node_, temp) );
}
