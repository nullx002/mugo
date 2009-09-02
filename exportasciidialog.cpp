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

    m_ui->typeComboBox->addItem( tr("English") );
    m_ui->typeComboBox->addItem( tr("Japanese") );

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
        createJapaneseAscii();
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

void ExportAsciiDialog::createJapaneseAscii(){
#ifdef Q_WS_WIN
    QFont f(QString::fromUtf8("ＭＳ ゴシック"), 8);
    m_ui->asciiTextEdit->setFont(f);
#elif defined(Q_WS_MAC)
    QFont f(QString::fromUtf8("Osaka−等幅"), 8);
    m_ui->asciiTextEdit->setFont(f);
#endif

    QByteArray s;
    s.reserve(boardBuffer.size() + 2 * boardBuffer[0].size() * 3);

    const char* top[]    = {"┏", "┯", "┓"};
    const char* center[] = {"┠", "┼", "┨"};
    const char* bottom[] = {"┗", "┷", "┛"};
    const char* header[] = {"Ａ", "Ｂ", "Ｃ", "Ｄ", "Ｅ", "Ｆ", "Ｇ", "Ｈ", "Ｊ", "Ｋ", "Ｌ", "Ｍ", "Ｎ", "Ｏ", "Ｐ", "Ｑ", "Ｒ", "Ｓ", "Ｔ", "Ｕ", "Ｖ", "Ｗ", "Ｘ", "Ｙ", "Ｚ"};
    int headerNum = sizeof(header)/sizeof(header[0]);

    s.append("　");
    for (int i=0, j=0; i<boardBuffer[0].size(); ++i, ++j){
        if (j >= headerNum)
            j = 0;
        s.push_back( header[j] );
    }

    s.push_back('\n');

    for (int y=0; y<boardBuffer.size(); ++y){
        s.append( QString("%1").arg(boardBuffer.size() - y, 2) );

        const char** b = y == 0 ? top : y == boardBuffer.size() - 1 ? bottom : center;

        for (int x=0; x<boardBuffer[y].size(); ++x){
            if (boardBuffer[y][x].black())
                s.append("●");
            else if (boardBuffer[y][x].white())
                s.append("○");
            else{
                if (x == 0)
                    s.append(b[0]);
                else if (x == boardBuffer[y].size() - 1)
                    s.append(b[2]);
                else if (isStar(x, y))
                    s.append("╋");
                else
                    s.append(b[1]);
            }
        }

        s.append( QString("%1\n").arg(boardBuffer.size() - y, 2) );
    }

    s.append("　");
    for (int i=0, j=0; i<boardBuffer[0].size(); ++i, ++j){
        if (j >= headerNum)
            j = 0;
        s.push_back( header[j] );
    }

    m_ui->asciiTextEdit->setPlainText( QString::fromUtf8(s) );
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

