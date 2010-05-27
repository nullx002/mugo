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
#include <QClipboard>
#include "exportasciidialog.h"
#include "ui_exportasciidialog.h"

ExportAsciiDialog::ExportAsciiDialog(QWidget *parent, const BoardWidget::BoardBuffer& buffer) :
    QDialog(parent),
    m_ui(new Ui::ExportAsciiDialog),
    boardBuffer(buffer)
{
    m_ui->setupUi(this);

    QSettings setting;
    int language = setting.value("asciiboard/language").toInt();
    m_ui->typeComboBox->setCurrentIndex( language );
    createAscii( language );
}

ExportAsciiDialog::~ExportAsciiDialog()
{
    delete m_ui;
}

void ExportAsciiDialog::changeEvent(QEvent *e)
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

void ExportAsciiDialog::accept(){
    QDialog::accept();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText( m_ui->asciiTextEdit->toPlainText() );

    QSettings setting;
    setting.setValue("asciiboard/language", m_ui->typeComboBox->currentIndex());
}

/** slot
*/
void ExportAsciiDialog::on_typeComboBox_currentIndexChanged(int index){
    createAscii(index);
}

void ExportAsciiDialog::createAscii(int index){
    if (index == 0)
        createEnglishAscii();
    else
        createJapaneseAscii(index == 1);
}

void ExportAsciiDialog::createEnglishAscii(){
#ifndef Q_WS_X11
    QFont f("Consolas,Courier", 8);
    m_ui->asciiTextEdit->setFont(f);
#endif

    QByteArray s;
    s.reserve(boardBuffer.size() + 2 * boardBuffer[0].size() * 3);

    s.append("  ");
    for (int i=0; i<boardBuffer[0].size(); ++i){
        s.push_back(' ');
        s.push_back( 'A' + (i > 7 ? i+1 : i) );
    }
    s.push_back('\n');

    for (int y=0; y<boardBuffer.size(); ++y){
        s.append( QString("%1").arg(boardBuffer.size() - y, 2) );

        for (int x=0; x<boardBuffer[y].size(); ++x){
            if (boardBuffer[y][x].black())
                s.append(" #");
            else if (boardBuffer[y][x].white())
                s.append(" O");
            else if (isStar(x, y))
                s.append(" +");
            else
                s.append(" .");
        }

        s.append( QString(" %1\n").arg(boardBuffer.size() - y, 2) );
    }

    s.append("  ");
    for (int i=0; i<boardBuffer[0].size(); ++i){
        s.push_back(' ');
        s.push_back( 'A' + (i > 7 ? i+1 : i) );
    }

    m_ui->asciiTextEdit->setPlainText(s);
}

void ExportAsciiDialog::createJapaneseAscii(bool isMono){
#ifdef Q_WS_WIN
    const char* monospace   = "\xef\xbc\xad\xef\xbc\xb3\x20\xe3\x82\xb4\xe3\x82\xb7\xe3\x83\x83\xe3\x82\xaf"; // ms gothic
    const char* propotional = "\xef\xbc\xad\xef\xbc\xb3\x20\xef\xbc\xb0\xe3\x82\xb4\xe3\x82\xb7\xe3\x83\x83\xe3\x82\xaf"; // ms p-gothic
#elif defined(Q_WS_MAC)
    const char* monospace   = "Osaka\xe2\xe7\xad\x89\xe5\xb9\x85"; // Osaka-tohaba
    const char* propotional = "Osaka"; // Osaka
#else
    const char* monospace   = "Osaka\xe2\xe7\xad\x89\xe5\xb9\x85"; // Osaka-tohaba
    const char* propotional = "Osaka\xe2\xe7\xad\x89\xe5\xb9\x85"; //
#endif

    QFont f(isMono ? monospace : propotional, 8);
    m_ui->asciiTextEdit->setFont(f);

    QByteArray s;
    s.reserve(boardBuffer.size() + 2 * boardBuffer[0].size() * 3);

    const char* top[]    = {"\xe2\x94\x8f","\xe2\x94\xaf","\xe2\x94\x93"};
    const char* center[] = {"\xe2\x94\xa0","\xe2\x94\xbc","\xe2\x94\xa8"};
    const char* bottom[] = {"\xe2\x94\x97","\xe2\x94\xb7","\xe2\x94\x9b"};
    const char* header[] = {"\xef\xbc\xa1","\xef\xbc\xa2","\xef\xbc\xa3","\xef\xbc\xa4","\xef\xbc\xa5","\xef\xbc\xa6","\xef\xbc\xa7","\xef\xbc\xa8","\xef\xbc\xaa","\xef\xbc\xab","\xef\xbc\xac","\xef\xbc\xad","\xef\xbc\xae","\xef\xbc\xaf","\xef\xbc\xb0","\xef\xbc\xb1","\xef\xbc\xb2","\xef\xbc\xb3","\xef\xbc\xb4","\xef\xbc\xb5","\xef\xbc\xb6","\xef\xbc\xb7","\xef\xbc\xb8","\xef\xbc\xb9","\xef\xbc\xba"};
    int headerNum = sizeof(header)/sizeof(header[0]);

    // 2byte space
    if (isMono)
        s.append("\xe3\x80\x80");
    else
        s.append("\xef\xbc\xbf");

    for (int i=0, j=0; i<boardBuffer[0].size(); ++i, ++j){
        if (j >= headerNum)
            j = 0;
        s.push_back( header[j] );
        if (isMono == false)
            s.push_back(' ');
    }

    s.push_back('\n');

    for (int y=0; y<boardBuffer.size(); ++y){
        if (isMono)
            s.append( QString().sprintf("%2d", boardBuffer.size() - y) );
        else
            s.append( QString().sprintf("%02d", boardBuffer.size() - y) );

        const char** b = y == 0 ? top : y == boardBuffer.size() - 1 ? bottom : center;

        for (int x=0; x<boardBuffer[y].size(); ++x){
            if (boardBuffer[y][x].black())
                s.append("\xe2\x97\x8f");
            else if (boardBuffer[y][x].white())
                s.append("\xe2\x97\x8b");
            else{
                if (x == 0)
                    s.append(b[0]);
                else if (x == boardBuffer[y].size() - 1)
                    s.append(b[2]);
                else if (isStar(x, y))
                    s.append("\xe2\x95\x8b");
                else
                    s.append(b[1]);
            }
        }

        if (isMono)
            s.append( QString().sprintf("%2d\n", boardBuffer.size() - y) );
        else
            s.append( QString().sprintf("%02d\n", boardBuffer.size() - y) );
    }

    // 2byte space
    if (isMono)
        s.append("\xe3\x80\x80");
    else
        s.append("\xef\xbc\xbf");

    for (int i=0, j=0; i<boardBuffer[0].size(); ++i, ++j){
        if (j >= headerNum)
            j = 0;
        s.push_back( header[j] );
        if (isMono == false)
            s.push_back(' ');
    }

    m_ui->asciiTextEdit->setPlainText(s);
}

bool ExportAsciiDialog::isStar(int x, int y){
    int  xsize = boardBuffer.size();
    bool isxstar = false;
    if (xsize < 7)
        isxstar = false;
    else if (xsize < 10)
        isxstar = x == 2 || x == xsize - 3;
    else{
        isxstar = x == 3 || x == xsize - 4;
        if (!isxstar && xsize % 2)
            isxstar = x == xsize / 2;
    }

    int  ysize = boardBuffer[0].size();
    bool isystar = false;
    if (ysize < 7)
        isystar = false;
    else if (ysize < 10)
        isystar = y == 2 || y == ysize - 3;
    else{
        isystar = y == 3 || y == ysize - 4;
        if (!isystar && ysize % 2)
            isystar = y == ysize / 2;
    }

    return isxstar && isystar;
}

