#include <QDebug>
#include "gameinformationdialog.h"
#include "ui_gameinformationdialog.h"

GameInformationDialog::GameInformationDialog(QWidget *parent, go::informationNode* infoNode) :
    QDialog(parent),
    m_ui(new Ui::GameInformationDialog),
    gameInfo(infoNode)
{
    m_ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));

    // white player
    m_ui->whitePlayer->setText( gameInfo->whitePlayer );
    m_ui->whiteRank->setText( gameInfo->whiteRank);
    m_ui->whiteTeam->setText( gameInfo->whiteTeam );

    // black player
    m_ui->blackPlayer->setText( gameInfo->blackPlayer );
    m_ui->blackRank->setText( gameInfo->blackRank);
    m_ui->blackTeam->setText( gameInfo->blackTeam );

    // rule
    m_ui->komi->setText( QString("%1").arg(gameInfo->komi) );
    m_ui->handicap->setText( QString("%1").arg(gameInfo->handicap) );
    m_ui->result->setText( gameInfo->result );
    m_ui->time->setText( gameInfo->time );
    m_ui->overTime->setText( gameInfo->overTime );
//    QString rule;

    // when / where
    m_ui->gameName->setText( gameInfo->gameName );
    m_ui->date->setText( gameInfo->date );
    m_ui->round->setText( gameInfo->round );
    m_ui->place->setText( gameInfo->place );
    m_ui->event->setText( gameInfo->event );

    // other
    m_ui->annotation->setText( gameInfo->annotation );
    m_ui->source->setText( gameInfo->source );
    m_ui->gameComment->setPlainText( gameInfo->gameComment );
    m_ui->copyright->setPlainText( gameInfo->copyright);
    m_ui->user->setText( gameInfo->user );
    m_ui->opening->setText( gameInfo->opening );
}

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

void GameInformationDialog::onAccepted()
{
    // white player
    gameInfo->whitePlayer = m_ui->whitePlayer->text();
    gameInfo->whiteRank   = m_ui->whiteRank->text();
    gameInfo->whiteTeam   = m_ui->whiteTeam->text();

    // black player
    gameInfo->blackPlayer = m_ui->blackPlayer->text();
    gameInfo->blackRank   = m_ui->blackRank->text();
    gameInfo->blackTeam   = m_ui->blackTeam->text();

    // rule
    gameInfo->komi = m_ui->komi->text().toDouble();
    gameInfo->handicap = m_ui->handicap->text().toInt();
    gameInfo->result   = m_ui->result->text();
    gameInfo->time     = m_ui->time->text();
    gameInfo->overTime = m_ui->overTime->text();

    // when / where
    gameInfo->gameName = m_ui->gameName->text();
    gameInfo->date = m_ui->date->text();
    gameInfo->round = m_ui->round->text();
    gameInfo->place = m_ui->place->text();
    gameInfo->event = m_ui->event->text();

    // other
    gameInfo->annotation  = m_ui->annotation->text();
    gameInfo->source      = m_ui->source->text();
    gameInfo->gameComment = m_ui->gameComment->toPlainText();
    gameInfo->copyright   = m_ui->copyright->toPlainText();
    gameInfo->user        = m_ui->user->text();
    gameInfo->opening     = m_ui->opening->text();
}
