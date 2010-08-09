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
#include "countterritorydialog.h"
#include "ui_countterritorydialog.h"

CountTerritoryDialog::CountTerritoryDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::CountTerritoryDialog),
    scorej(0)
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
    m_ui->scoreTextEdit->clear();
}

void CountTerritoryDialog::accept(){
    if (informationNode){
        if (scorej > 0)
            informationNode->result = QString("W+%1").arg(scorej);
        else if (scorej < 0)
            informationNode->result = QString("B+%1").arg(scorej * -1);
        else
            informationNode->result = "Draw";
    }

    QDialog::accept();
}

void CountTerritoryDialog::setScore(int alive_b, int alive_w, int dead_b, int dead_w, int capturedBlack, int capturedWhite, int blackTerritory, int whiteTerritory, double komi){
    qreal bscorej = blackTerritory + dead_w + capturedWhite;
    qreal wscorej = whiteTerritory + dead_b + capturedBlack + komi;
    scorej = wscorej - bscorej;

    // japanese rule
    QString bj( tr("Black: %1 = %2(territories) + %3(captured)").arg(bscorej).arg(blackTerritory).arg(dead_w + capturedWhite) );
    QString wj( tr("White: %1 = %2(territories) + %3(captured) + %4(komi)").arg(wscorej).arg(whiteTerritory).arg(dead_b + capturedBlack).arg(komi) );

    QString resultj;
    if (wscorej > bscorej)
        resultj = QString(tr("W+%1")).arg(wscorej - bscorej);
    else if (bscorej > wscorej)
        resultj = QString(tr("B+%1")).arg(bscorej - wscorej);
    else
        resultj = tr("Draw");

    QString s = tr("Japanese Rule") + ":\n" + wj + "\n" + bj + "\n" + resultj + "\n\n";


    // chinese rule
    double half = (blackTerritory + alive_b + whiteTerritory + alive_w) / 2.0;
    double bscorec = blackTerritory + alive_b - komi / 2.0;
    double wscorec = whiteTerritory + alive_w + komi / 2.0;

    QString bc, wc;
    if (komi > 0){
        bc = tr("Black: %1 = %2(point) - %3(komi) / 2").arg(bscorec).arg(blackTerritory + alive_b).arg(komi);
        wc = tr("White: %1 = %2(point) + %3(komi) / 2").arg(wscorec).arg(whiteTerritory + alive_w).arg(komi);
    }
    else{
        bc = tr("Black: %1 = %2(point) + %3(komi) / 2").arg(bscorec).arg(blackTerritory + alive_b).arg(komi);
        wc = tr("White: %1 = %2(point) - %3(komi) / 2").arg(wscorec).arg(whiteTerritory + alive_w).arg(komi);
    }

    QString resultc;
    if (wscorec > bscorec)
        resultc = QString(tr("W+%1")).arg(wscorec - half);
    else if (bscorec > wscorec)
        resultc = QString(tr("B+%1")).arg(bscorec - half);
    else
        resultc = tr("Draw");

    s += tr("Chinese Rule") + ":\n" + wc + "\n" + bc + "\n" + resultc;

    m_ui->scoreTextEdit->setPlainText(s);
}
