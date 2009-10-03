#include <QDebug>
#include <QMessageBox>
#include "appdef.h"
#include "boardwidget.h"
#include "gtp.h"
#include "gtp.h"

gtp::gtp(BoardWidget* board, go::color color, QProcess& proc, QObject* parent) : PlayGame(board, color, parent), process(proc), index(0), moving_(false)
{
    connect(&process, SIGNAL(readyRead()), this, SLOT(gtpRead()));
}

void gtp::move(int x, int y){
    commandList.push_back( commandPtr(new moveCommand(x, y)) );

    QString xy;
    if (x < 0 || y < 0)
        xy = "PASS";
    else
        xy = boardWidget_->getXYString(x, y, false);

    if (yourColor_ == go::black)
        write( QString("play black %1\n").arg(xy) );
    else
        write( QString("play white %1\n").arg(xy) );

    moving_ = true;
}

void gtp::put(go::color color, int x, int y){
    commandList.push_back( commandPtr(new putCommand(x, y)) );

    QString xy;
    if (x < 0 || y < 0)
        xy = "PASS";
    else
        xy = boardWidget_->getXYString(x, y, false);

    if (color == go::black)
        write( QString("play black %1\n").arg(xy) );
    else
        write( QString("play white %1\n").arg(xy) );
}

void gtp::wait(){
    if (yourColor_ == go::black){
        commandList.push_back( commandPtr(new genmoveCommand(go::white)) );
        write( "genmove white\n" );
    }
    else{
        commandList.push_back( commandPtr(new genmoveCommand(go::black)) );
        write( "genmove black\n" );
    }
}

void gtp::quit(){
    commandList.push_back( commandPtr(new command(eQuit)) );
    write( "quit\n" );
}

void gtp::deadList(){
    commandList.push_back( commandPtr(new command(eDeadList)) );
    write( "final_status_list dead\n" );
}

void gtp::write(const QString& buf){
    QString msg = QString("%1 %2").arg(index++).arg(buf);
    qDebug() << msg;
    process.write( msg.toAscii() );
}

void gtp::gtpRead(){
    gtpBuf.append( process.readAll() );

    QStringList resList = gtpBuf.split("\n\n", QString::SkipEmptyParts);
    if (gtpBuf.size() < 4 || gtpBuf.right(2) != "\n\n"){
        gtpBuf = resList.back();
        resList.pop_back();
    }
    else
        gtpBuf.clear();

    foreach(QString s, resList){
        qDebug() << s;
        QChar status = s[0];
        int p = s.indexOf(' ', 2);
        QString msg = s.mid(p+1);
        int index = s.mid(1, p-1).toInt();

        if (commandList[index]->status == eMove)
            moving_ = false;

        if (status != '=')
            continue;

        if (commandList[index]->status == eMove){
            moveCommand* cmd = (moveCommand*)commandList[index].get();
            boardWidget_->addStoneNodeCommand(cmd->x, cmd->y);
            if (isGameEnd()){
                deadList();
                return;
            }
            wait();
        }
        else if (commandList[index]->status == eGen){
            if (msg == "resign"){
                QMessageBox::information(boardWidget_, APPNAME, tr("Computer resign."));
                isResign_ = true;
                quit();
                emit gameEnded();
                return;
            }

            int x, y;
            if (stringToPosition(msg, x, y))
                boardWidget_->addStoneNodeCommand(x, y);

            if (isGameEnd())
                deadList();
        }
        else if (commandList[index]->status == ePut){
            const go::nodePtr& root = boardWidget_->getData().root;
            putCommand* cmd = (putCommand*)commandList[index].get();
            boardWidget_->addStone(root, go::point(cmd->x, cmd->y), go::black);
        }
        else if (commandList[index]->status == eDeadList){
            const BoardWidget::BoardBuffer& buffer = boardWidget_->getBuffer();
            QStringList deadStones = msg.split(QRegExp("[ \n]"));
            foreach(QString stone, deadStones){
                int sx, sy;  // sgfX, sgfY
                if (stringToPosition(stone, sx, sy) == false)
                    continue;

                int bx, by;  // boardX, boardY
                boardWidget_->sgfToBoardCoordinate(sx, sy, bx, by);

                if (!buffer[by][bx].blackTerritory() && !buffer[by][bx].whiteTerritory())
                    boardWidget_->addTerritory(bx, by);
            }
            emit gameEnded();
        }
    }
}

bool gtp::stringToPosition(const QString& buf, int& x, int& y){
    if (buf.size() < 2)
        return false;

    if (buf == "PASS"){
        x = y = -1;
        return true;
    }

    int xsize = boardWidget_->getData().root->xsize;
    int ysize = boardWidget_->getData().root->ysize;

    x = buf[0].toAscii() - 'A';
    if (x > 7)
        --x;
    y = ysize - buf.mid(1).toInt();

    return x >= 0 && x < xsize && y >= 0 && y < ysize;
}
