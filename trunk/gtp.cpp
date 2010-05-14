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

    QString param;
    if (color == go::black)
        param = QString("black %1\n").arg(xy);
    else
        param = QString("white %1\n").arg(xy);

    execute("play", param);
}


/**
* Constructor
*/
gtp::gtp(BoardWidget* board, go::color color, bool newGame, int level, QProcess* proc, QObject* parent)
    : PlayGame(board, color, newGame, parent)
    , process(proc)
    , initialized_(false)
    , index(0)
    , commandProcessing(false)
    , level_(level)
    , isKilled_(false)
    , mode_(ePlayGame)
{
    connect(process, SIGNAL(readyRead()), this, SLOT(on_gtp_read()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_gtp_finished(int, QProcess::ExitStatus)));
    name();
    version();
    list_commands();
}

/**
* Constructor
*/
gtp::gtp(BoardWidget* board, QProcess* proc, QObject* parent)
    : PlayGame(board, parent)
    , process(proc)
    , initialized_(false)
    , index(0)
    , commandProcessing(false)
    , level_(0)
    , isKilled_(false)
    , mode_(eNoMode)
{
    connect(process, SIGNAL(readyRead()), this, SLOT(on_gtp_read()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_gtp_finished(int, QProcess::ExitStatus)));
    name();
    version();
    list_commands();
}

/**
* Constructor
*/
gtp::gtp(QProcess* proc, QObject* parent)
    : PlayGame(parent)
    , process(proc)
    , initialized_(false)
    , index(0)
    , commandProcessing(false)
    , level_(0)
    , isKilled_(false)
    , mode_(eNoMode)
{
    connect(process, SIGNAL(readyRead()), this, SLOT(on_gtp_read()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_gtp_finished(int, QProcess::ExitStatus)));
    name();
    version();
    list_commands();
}

/**
* Destructor
*/
gtp::~gtp(){
    if (process){
        process->disconnect();
        delete process;
    }
}

/**
* undo
* send undo command.
*/
bool gtp::undo(){
    if (moving())
        return false;

    if (supportedCommandList.indexOf("undo") != -1){
        commandList.push_back( commandPtr(new command(this, eUndo, "undo")) );
        commandList.push_back( commandPtr(new command(this, eUndo, "undo")) );
        write();
        return true;
    }
    return false;
}

/**
* move stone.
* send play black or play white command.
*/
bool gtp::move(int x, int y){
    if (moving())
        return false;

    commandList.push_back( commandPtr(new moveCommand(this, boardWidget_, color_, x, y)) );
    write();

    return true;
}

/**
* put stone.
*/
bool gtp::put(go::color color, int x, int y){
    return put(color, x, y, ePut);
}

/**
* put stone.
*/
bool gtp::put(go::color color, int x, int y, eKind kind){
//    if (moving())
//        return false;

    commandList.push_back( commandPtr(new putCommand(this, boardWidget_, color, x, y, kind)) );
    write();

    return true;
}

/**
* wait opponent's move.
* send genmove command.
*/
bool gtp::wait(){
//    if (moving())
//        return false;

    commandList.push_back( commandPtr(new genmoveCommand(this, color_ == go::white ? go::black : go::white)) );
    write();

    return true;
}

/**
* quit game.
* send quit command.
*/
bool gtp::quit(bool resign){
    isResign_ = resign;
    isAbort_  = false;
    commandList.push_back( commandPtr(new command(this, eQuit, "quit")) );
    write();

    return true;
}

/**
* abort game.
* send quit command.
*/
bool gtp::abort(){
    isResign_ = false;
    isAbort_  = true;
    commandList.push_back( commandPtr(new command(this, eQuit, "quit")) );
    write();

    return true;
}

/**
* kill porcess
* send quit command.
*/
void gtp::kill(){
    isKilled_ = true;
    process->close();
}

/**
* get dead stones.
* send final_status_list dead command.
*/
bool gtp::deadList(){
    if (supportedCommandList.indexOf("final_status_list") != -1){
        commandList.push_back( commandPtr(new command(this, eDeadList, "final_status_list", "dead")) );
        write();
        return true;
    }
    return false;
}

/**
* list_commands
*/
void gtp::list_commands(){
    commandList.push_back( commandPtr(new command(this, eListCommands, "list_commands")) );
    write();
}

/**
* get name
*/
void gtp::name(){
    commandList.push_back( commandPtr(new command(this, eName, "name")) );
    write();
}

/**
* get version
*/
void gtp::version(){
    commandList.push_back( commandPtr(new command(this, eVersion, "version")) );
    write();
}

/**
* set board size
*/
void gtp::boardSize(int size){
    commandList.push_back( commandPtr(new command(this, eBoardSize, "boardsize", QString::number(size))) );
    write();
}

/**
* set komi
*/
void gtp::komi(double komi){
    commandList.push_back( commandPtr(new command(this, eKomi, "komi", QString::number(komi, 'g', 1))) );
    write();
}

/**
* set level
*/
void gtp::level(int level){
    if (supportedCommandList.indexOf("level") != -1){
        commandList.push_back( commandPtr(new levelCommand(this, level)) );
        write();
    }
}

/**
* estimate score
*/
void gtp::estimateScore(){
    if (supportedCommandList.indexOf("initial_influence") != -1){
        commandList.push_back( commandPtr(new command(this, eInitialInfluence, "initial_influence", "black territory_value")) );
        commandList.push_back( commandPtr(new command(this, eInitialInfluence, "initial_influence", "white territory_value")) );
        write();
    }
}

/**
* send gtp command to game engine.
*/
void gtp::write(){
    if (commandProcessing == false && commandList.empty() == false){
        commandPtr& command = commandList.front();
        QString cmd = command->cmd;
//        if (supportedCommandList.indexOf(cmd) != -1){
            commandProcessing = true;
            if (command->param.isEmpty() == false)
                cmd += ' ' + command->param;
            qDebug() << cmd;
            cmd += '\n';
            process->write( cmd.toAscii() );
//        }
    }
}

/**
* received response message from game engine.
*/
void gtp::on_gtp_read(){
    // read received message.
    QByteArray bytes = process->readAll();
    qDebug() << bytes;
    gtpBuf.append( bytes );
    if (gtpBuf.size() < 4 || gtpBuf.right(2) != "\n\n")
        return;

    // process received message.
    QString s( gtpBuf.left(gtpBuf.size()-2) );
    gtpBuf.clear();
    processCommand(s);
    commandProcessing = false;

    // if command stack is not empty, send next command.
    if (process->isOpen())
        write();
}

/**
* game engine was finished
*/
void gtp::on_gtp_finished(int exitCode, QProcess::ExitStatus exitStatus){
    emit gameEnded();
    if (!isKilled_){  // crashed when delete process after kill process.
        process->disconnect();
        delete process;
    }
    process = NULL;
    delete this;
}

/**
* process received message.
*/
void gtp::processCommand(QString& s){
    QChar status = s[0];
//    int p = s.indexOf(' ', 2);
//    QString msg = s.mid(p+1);
    QString msg = s.mid(2);
//    int index = s.mid(1, p-1).toInt();

    if (commandList.isEmpty())
        return;

    commandPtr command = commandList.front();
    commandList.pop_front();

    if (status != '=')
        return;

    if (command->kind == eMove){
        moveCommand* cmd = (moveCommand*)command.get();
        boardWidget_->insertStoneNodeCommand(0, cmd->x, cmd->y);
        if (isGameEnd()){
            deadList();
            quit(false);
            return;
        }
        wait();
    }
    else if (command->kind == eGen){
        if (msg == "resign"){
            winner_ = color_;
            quit(true);
            return;
        }

        int x, y;
        if (stringToPosition(msg, x, y))
            boardWidget_->insertStoneNodeCommand(0, x, y);

        if (isGameEnd()){
            deadList();
            quit(false);
        }
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
                boardWidget_->reverseTerritory(bx, by);
        }
    }
    else if (command->kind == eUndo)
        boardWidget_->forward(-1);
    else if (command->kind == eListCommands){
        supportedCommandList = s.split("\n");
        initialize();
    }
    else if (command->kind == eName){
        name_ = msg;
        emit getName(msg);
    }
    else if (command->kind == eVersion){
        version_ = msg;
        emit getVersion(msg);
    }
    else if (command->kind == eLoadSgf)
        tempFile.remove();
    else if (command->kind == eInitialInfluence)
        initialInfluence(msg);
}

/**
* get position from gtp message.
*/
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

/**
*/
bool gtp::moving() const{
    return commandList.isEmpty() == false;
}

/**
* init game engine.
*/
void gtp::initialize(){
    initialized_ = true;
    emit initialized();

    if (boardWidget_ == NULL)
        return;

    boardSize( boardWidget_->getData().root->xsize );
    komi( boardWidget_->getData().root->komi );
    level(level_);

    if (isNewGame()){
        setHandicap();
        int handicap = boardWidget_->getData().root->handicap;
        if (mode_ == ePlayGame && ((color_ == go::white && handicap == 0) || (color_ == go::black && handicap > 0)))
            wait();
    }
    else{
        restoreSgf();
        if (mode_ == ePlayGame && boardWidget_->getColor() != color_)
            wait();
    }

    if (mode_ == eEstimateScore)
        estimateScore();
}

/**
* restore board if continue game
*/
void gtp::restoreSgf(){
    if (loadSgf())
        return;

    const go::nodeList& nodeList = boardWidget_->getCurrentNodeList();
    foreach (const go::nodePtr& node, nodeList){
        int x = node->getX();
        int y = node->getY();

        QString xy;
        if (x < 0 || y < 0)
            xy = "PASS";
        else
            xy = boardWidget_->getXYString(x, y, false);

        if (node->color == go::black)
            put(go::black, x, y, eNone);
        else if (node->color == go::white)
            put(go::white, x, y, eNone);

        foreach(const go::stone& stone, node->blackStones)
            put(go::black, stone.p.x, stone.p.y, eNone);

        foreach(const go::stone& stone, node->whiteStones)
            put(go::white, stone.p.x, stone.p.y, eNone);

        if (node == boardWidget_->getCurrentNode())
            break;
    }
}

/**
* load sgf file to gtp engine if supported.
*/
bool gtp::loadSgf(){
    if (name_.compare("fuego", Qt::CaseInsensitive) == 0)  // fuego returns loadsgf at list_commands, but fuego freeze in loadsgf.
        return false;
    if (supportedCommandList.indexOf("loadsgf") == -1)
        return false;

    if (tempFile.open() == false)
        return false;
    go::informationPtr& root = boardWidget_->getData().root;
    tempFile.write( QString().sprintf("(;GM[1]SZ[%d]KM[%.1f]", root->xsize, root->komi).toAscii() );

    const go::nodeList& nodeList = boardWidget_->getCurrentNodeList();
    foreach (const go::nodePtr& node, nodeList){
        int x = node->getX();
        int y = node->getY();

        if (node->color == go::black || node->color == go::white){
            tempFile.write( node->color == go::black ? ";B" : ";W" );
            if (x < 0 || y < 0)
                tempFile.write("[]");
            else
                tempFile.write( QString().sprintf("[%c%c]", char(x < 26 ? 'a' + x : 'A' + x), char(y < 26 ? 'a' + y : 'A' + y)).toAscii() );
        }

        foreach(const go::stone& stone, node->blackStones){
            tempFile.write( QString().sprintf("AB[%c%c]", char(stone.p.x < 26 ? 'a' + stone.p.x : 'A' + stone.p.x), char(stone.p.y < 26 ? 'a' + stone.p.y : 'A' + stone.p.y)).toAscii() );
        }

        foreach(const go::stone& stone, node->whiteStones){
            tempFile.write( QString().sprintf("AW[%c%c]", char(stone.p.x < 26 ? 'a' + stone.p.x : 'A' + stone.p.x), char(stone.p.y < 26 ? 'a' + stone.p.y : 'A' + stone.p.y)).toAscii() );
        }

        if (node == boardWidget_->getCurrentNode())
            break;
    }

    QDir tempPath = QDir::tempPath();
    QString str = "loadsgf " + tempPath.absoluteFilePath(tempFile.fileName()) + "\n";
    tempFile.write(")");
    tempFile.close();

    commandList.push_back( commandPtr(new command(this, eLoadSgf, str)) );
    write();

    return true;
}

void gtp::initialInfluence(const QString& msg){
    QVector< QVector<double> > territories;
    QStringList msgList = msg.split("\n");
    territories.resize(msgList.size());

    int i = 0;
    foreach(const QString& line, msgList){
        QStringList values = line.split(" ");
        territories[i].reserve(msgList.size());

        foreach(const QString& v, values){
            if (v.isEmpty() == false)
                territories[i].push_back(v.toDouble());
        }
        ++i;
    }
    emit scoreEstimated(boardWidget_, territories);
}
