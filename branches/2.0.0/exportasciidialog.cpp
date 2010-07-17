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
#include "boardwidget.h"

/**
  Constructor
*/
ExportAsciiDialog::ExportAsciiDialog(QWidget *parent, const BoardWidget::BoardBuffer& buf)
    : QDialog(parent)
    , m_ui(new Ui::ExportAsciiDialog)
    , boardBuffer(buf)
{
    m_ui->setupUi(this);

    QSettings setting;
    int language = setting.value("asciiboard/language").toInt();
    m_ui->typeComboBox->setCurrentIndex( language );
    createAscii( language );
}

/**
  Destructor
*/
ExportAsciiDialog::~ExportAsciiDialog()
{
    delete m_ui;
}

/**
  accept dialog
*/
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

/**
  create ascii board
*/
void ExportAsciiDialog::createAscii(int index){
    if (index == 0)
        createEnglishAscii();
    else
        createJapaneseAscii(index == 1);
}

/**
  create ascii board using english font
*/
void ExportAsciiDialog::createEnglishAscii(){
    QFont f("Consolas,Courier,monospace", 8);
    m_ui->asciiTextEdit->setFont(f);

    QByteArray s;
    s.reserve( (boardBuffer.size() + 2) * boardBuffer[0].size() * 3);

    s.append("  ");
    for (int i=0; i<boardBuffer[0].size(); ++i){
        int n = i % 25;
        if (n > 7)
            ++n;
        s.push_back(' ');
        s.push_back('A' + n);
    }
    s.push_back('\n');

    for (int y=0; y<boardBuffer.size(); ++y){
        s.append( QString("%1").arg(boardBuffer.size() - y, 2) );

        for (int x=0; x<boardBuffer[y].size(); ++x){
            if (boardBuffer[y][x].isBlack())
                s.append(" #");
            else if (boardBuffer[y][x].isWhite())
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
        int n = i % 25;
        if (n > 7)
            ++n;
        s.push_back(' ');
        s.push_back('A' + n);
    }

    m_ui->asciiTextEdit->setPlainText(s);
}

/**
  create ascii board using japanese font
*/
void ExportAsciiDialog::createJapaneseAscii(bool isMono){
#ifdef Q_WS_WIN
    const char* monospace   = "\xef\xbc\xad\xef\xbc\xb3\x20\xe3\x82\xb4\xe3\x82\xb7\xe3\x83\x83\xe3\x82\xaf"; // ms gothic
    const char* propotional = "\xef\xbc\xad\xef\xbc\xb3\x20\xef\xbc\xb0\xe3\x82\xb4\xe3\x82\xb7\xe3\x83\x83\xe3\x82\xaf"; // ms p-gothic
#elif defined(Q_WS_MAC)
    const char* monospace   = "Osaka\342\210\222\347\255\211\345\271\205"; // Osaka-tohaba
    const char* propotional = "Osaka"; // Osaka
#else
    const char* monospace   = "monospace";
    const char* propotional = "Takao Pゴシック";
#endif
    const char* fontname = isMono ? monospace : propotional;

    QFont f(fontname, 8);
    m_ui->asciiTextEdit->setFont(f);

    QByteArray s;
    s.reserve( (boardBuffer.size() + 2) * boardBuffer[0].size() * 3);

    const char* top[]    = {"\xe2\x94\x8f","\xe2\x94\xaf","\xe2\x94\x93"};
    const char* center[] = {"\xe2\x94\xa0","\xe2\x94\xbc","\xe2\x94\xa8"};
    const char* bottom[] = {"\xe2\x94\x97","\xe2\x94\xb7","\xe2\x94\x9b"};
    const char* A = "\xef\xbc\xa1";

    // 2byte space
    if (isMono)
        s.append( "\xe3\x80\x80" );
    else
        s.append( "\xef\xbc\xbf" );

    for (int i=0; i<boardBuffer[0].size(); ++i){
        int n = i % 25;
        if (n > 7)
            ++n;

        int a = *(int*)A;
        ((char*)&a)[2] += n;
        s.append((char*)&a);
        if (isMono == false && n != 13 && n != 17)
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
            if (boardBuffer[y][x].isBlack())
                s.append("\xe2\x97\x8f");
            else if (boardBuffer[y][x].isWhite())
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

    for (int i=0; i<boardBuffer[0].size(); ++i){
        int n = i % 25;
        if (n > 7)
            ++n;

        int a = *(int*)A;
        ((char*)&a)[2] += n;
        s.append((char*)&a);
        if (isMono == false && n != 13 && n != 17)
            s.push_back(' ');
    }

    s.push_back('\n');

    m_ui->asciiTextEdit->setPlainText(s);
}

/**
  @retval true  position(x, y) is star
  @retval false position(x, y) is not star
*/
bool ExportAsciiDialog::isStar(int x, int y){
    int  ysize = boardBuffer.size();
    int  xsize = boardBuffer[0].size();
    return isStar2(xsize, x) && isStar2(ysize, y);
}

/**
  @retval true  n is star
  @retval false n is not star
*/
bool ExportAsciiDialog::isStar2(int size, int n){
    bool isStar = false;
    if (size < 7)
        isStar = false;
    else if (size < 10)
        isStar = n == 2 || n == size - 3;
    else{
        isStar = n == 3 || n == size - 4;
        if (!isStar && size % 2)
            isStar = n == size / 2;
    }

    return isStar;
}
