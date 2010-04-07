#include <QDebug>
#include <QMessageBox>
#include "appdef.h"
#include "boardwidget.h"
#include "gtp.h"
#include "gtp.h"

gtp::moveBaseCommand::moveBaseCommand(gtp* parent, eKind k, BoardWidget* boardWidget, go::color color, int x_, int y_) : command(parent, k), x(x_), y(y_)
{
    QString xy;
    if (x < 0 || y < 0)
        xy = "PASS";
    else
        xy = boardWidget->getXYString(x, y, false);

    QString msg;
    if (color == go::black)
        msg = QString("play black %1\n").arg(xy);
    else
        msg = QString("play white %1\n").arg(xy);

    execute(msg);
}


/**
* Constructor
*/
gtp::gtp(BoardWidget* board, go::color color, bool newGame, int boardSize, const qreal& komi, int handicap, int level, QProcess& proc, QObject* parent)
    : PlayGame(board, color, newGame, parent), process(proc), index(0)
    , commandProcessing(false)
{
    connect(&process, SIGNAL(readyRead()), this, SLOT(gtpRead()));

    // initialize
    commandList.push_back( commandPtr(new boardSizeCommand(this, boardSize)) );
    commandList.push_back( commandPtr(new komiCommand(this, komi)) );
    commandList.push_back( commandPtr(new levelCommand(this, level)) );

    // add stone if game is continued.
    const go::nodeList& nodeList = board->getCurrentNodeList();
    foreach (const go::nodePtr& node, nodeList){
        int x = node->getX();
        int y = node->getY();

        QString xy;
        if (x < 0 || y < 0)
            xy = "PASS";
        else
            xy = boardWidget_->getXYString(x, y, false);

        if (node->color == go::black)
            commandList.push_back( commandPtr(new command(this, eNone, QString("play black %1\n").arg(xy))) );
        else if (node->color == go::white)
            commandList.push_back( commandPtr(new command(this, eNone, QString("play white %1\n").arg(xy))) );

        if (node == board->getCurrentNode())
            break;
    }
}

bool gtp::undo(){
    if (moving())
        return false;

    commandList.push_back( commandPtr(new undoCommand(this)) );
    commandList.push_back( commandPtr(new undoCommand(this)) );

    return true;
}

bool gtp::move(int x, int y){
    if (moving())
        return false;

    commandList.push_back( commandPtr(new moveCommand(this, boardWidget_, yourColor_, x, y)) );

    return true;
}

bool gtp::put(go::color color, int x, int y){
//    if (moving())
//        return false;

    commandList.push_back( commandPtr(new putCommand(this, boardWidget_, color, x, y)) );

    return true;
}

bool gtp::wait(){
//    if (moving())
//        return false;

    commandList.push_back( commandPtr(new genmoveCommand(this, yourColor_ == go::white ? go::black : go::white)) );

    return true;
}

void gtp::quit(){
    commandList.push_back( commandPtr(new command(this, eQuit, "quit\n")) );
}

void gtp::deadList(){
    commandList.push_back( commandPtr(new command(this, eDeadList, "final_status_list dead\n")) );
}

void gtp::pushCommandList(const QString& buf){
//    QString msg = QString("%1 %2").arg(index++).arg(buf);
//    commandStringList.push_back(msg);
    commandStringList.push_back(buf);
    write();
}

void gtp::write(){
    if (commandProcessing == false && commandStringList.empty() == false){
        qDebug() << commandStringList.front();

        commandProcessing = true;
        process.write( commandStringList.front().toAscii() );
        commandStringList.pop_front();
    }
}

void gtp::gtpRead(){
    QByteArray bytes = process.readAll();
    qDebug() << bytes;
    gtpBuf.append( bytes );

    QStringList resList = gtpBuf.split("\n\n", QString::SkipEmptyParts);
    if (gtpBuf.size() < 4 || gtpBuf.right(2) != "\n\n")
        return;

    QString s( gtpBuf.left(gtpBuf.size()-2) );
    gtpBuf.clear();
    processCommand(s);
    commandProcessing = false;
    write();
}

void gtp::processCommand(QString& s){
    QChar status = s[0];
    int p = s.indexOf(' ', 2);
    QString msg = s.mid(p+1);
//    int index = s.mid(1, p-1).toInt();

    if (status != '=')
        return;

    commandPtr command = commandList.front();
    commandList.pop_front();

    if (command->kind == eMove){
        moveCommand* cmd = (moveCommand*)command.get();
        boardWidget_->addStoneNodeCommand(cmd->x, cmd->y);
        if (isGameEnd()){
            deadList();
            return;
        }
        wait();
    }
    else if (command->kind == eGen){
        if (msg == "resign"){
            QMessageBox::information(boardWidget_, APPNAME, tr("Computer resigns."));
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
    else if (command->kind == ePut){
        const go::nodePtr& root = boardWidget_->getData().root;
        putCommand* cmd = (putCommand*)command.get();
        boardWidget_->addStone(root, go::point(cmd->x, cmd->y), go::black);
    }
    else if (command->kind == eDeadList){
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
    else if (command->kind == eUndo)
        boardWidget_->forward(-1);
    else if (command->kind == eBoardSize){
        boardSizeCommand* cmd = (boardSizeCommand*)command.get();
        if (isNewGame())
            boardWidget_->setBoardSize( cmd->boardSize, cmd->boardSize );
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

bool gtp::moving() const{
    return commandList.isEmpty() == false;
}
