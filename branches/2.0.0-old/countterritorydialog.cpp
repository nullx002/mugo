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
#include <QUndoStack>
#include "countterritorydialog.h"
#include "ui_countterritorydialog.h"
#include "sgfdocument.h"
#include "command.h"


/**
  Constructor
*/
CountTerritoryDialog::CountTerritoryDialog(SgfDocument* doc, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::CountTerritoryDialog)
    , document(doc)
    , japanese_score(0)
    , chinese_score(0)
{
    m_ui->setupUi(this);
}

/**
  Destructor
*/
CountTerritoryDialog::~CountTerritoryDialog(){
    delete m_ui;
}

/**
  done
*/
void CountTerritoryDialog::done(int r){
    QDialog::done(r);
    m_ui->scoreTextEdit->clear();
}

/**
  accept
*/
void CountTerritoryDialog::accept(){
    if (gameInformation){
        double score = m_ui->ruleComboBox->currentIndex() == 0 ? japanese_score : chinese_score;

        Go::GameInformationPtr info(new Go::GameInformation);
        *info = *gameInformation;

        if (score > 0)
            info->result = QString("W+%1").arg(score);
        else if (japanese_score < 0)
            info->result = QString("B+%1").arg(score * -1);
        else
            info->result = "Draw";

        document->getUndoStack()->push( new SetGameInformationCommand(document, gameInformation, info) );
    }

    QDialog::accept();
}

/**
  current index of rule combobox chagned
*/
void CountTerritoryDialog::on_ruleComboBox_currentIndexChanged(int index){
    if (index == 0)
        m_ui->scoreTextEdit->setPlainText(japanese_text);
    else
        m_ui->scoreTextEdit->setPlainText(chinese_text);
}

/**
  set score
*/
void CountTerritoryDialog::setScore(int total, int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory, double komi){
    // japanese rule
    qreal bscorej = blackTerritory + dead_w + capturedWhite;
    qreal wscorej = whiteTerritory + dead_b + capturedBlack + komi;
    japanese_score = wscorej - bscorej;

    // chinese rule
    double half = double(total) / 2.0;
    double dame = total - alive_b - alive_w - blackTerritory - whiteTerritory;
    double bscorec = blackTerritory + alive_b - komi / 2.0 + dame / 2.0;
    double wscorec = whiteTerritory + alive_w + komi / 2.0 + dame / 2.0;
    chinese_score = wscorec - half;

    if (wscorej > bscorej)
        japanese_text = tr("White: %1 = %2(territories) + %3(captured) + %4(komi)\nBlack: %5 = %6(territories) + %7(captured)\nResult: W+%8")
                        .arg(wscorej).arg(whiteTerritory).arg(dead_b + capturedBlack).arg(komi)
                        .arg(bscorej).arg(blackTerritory).arg(dead_w + capturedWhite).arg(japanese_score);
    else if (wscorej < bscorej)
        japanese_text = tr("White: %1 = %2(territories) + %3(captured) + %4(komi)\nBlack: %5 = %6(territories) + %7(captured)\nResult: B+%8")
                        .arg(wscorej).arg(whiteTerritory).arg(dead_b + capturedBlack).arg(komi)
                        .arg(bscorej).arg(blackTerritory).arg(dead_w + capturedWhite).arg(japanese_score * -1.0);
    else
        japanese_text = tr("White: %1 = %2(territories) + %3(captured) + %4(komi)\nBlack: %5 = %6(territories) + %7(captured)\nResult: Draw")
                        .arg(wscorej).arg(whiteTerritory).arg(dead_b + capturedBlack).arg(komi)
                        .arg(bscorej).arg(blackTerritory).arg(dead_w + capturedWhite);

    if (wscorec > bscorec)
        chinese_text = tr("White: %1 = %2(points) - %3(komi) / 2 + %4(dame) / 2\nBlack: %5 = %6(points) - %7(komi) / 2 + %8(dame) / 2\nResult: W+%9")
                        .arg(wscorec).arg(whiteTerritory + alive_w).arg(komi).arg(dame)
                        .arg(bscorec).arg(blackTerritory + alive_b).arg(komi).arg(dame).arg(chinese_score);
    else if (wscorec < bscorec)
        chinese_text = tr("White: %1 = %2(points) - %3(komi) / 2 + %4(dame) / 2\nBlack: %5 = %6(points) - %7(komi) / 2 + %8(dame) / 2\nResult: B+%9")
                        .arg(wscorec).arg(whiteTerritory + alive_w).arg(komi).arg(dame)
                        .arg(bscorec).arg(blackTerritory + alive_b).arg(komi).arg(dame).arg(chinese_score * -1.0);
    else
        chinese_text = tr("White: %1 = %2(points) - %3(komi) / 2 + %4(dame) / 2\nBlack: %5 = %6(points) - %7(komi) / 2 + %8(dame) / 2\nResult: Draw")
                        .arg(wscorec).arg(whiteTerritory + alive_w).arg(komi).arg(dame)
                        .arg(bscorec).arg(blackTerritory + alive_b).arg(komi).arg(dame);

    if (m_ui->ruleComboBox->currentIndex() == 0)
        m_ui->scoreTextEdit->setPlainText(japanese_text);
    else
        m_ui->scoreTextEdit->setPlainText(chinese_text);
}
