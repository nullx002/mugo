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
gtp::gtp(BoardWidget* board, go::color color, int boardSize, const qreal& komi, int handicap, bool newGame, int level, QProcess* proc, QObject* parent)
    : PlayGame(board, color, boardSize, komi, handicap, newGame, parent)
    , process(proc)
    , initialized(false)
    , index(0)
    , commandProcessing(false)
    , level_(level)
    , isKilled(false)
{
    connect(process, SIGNAL(readyRead()), this, SLOT(on_gtp_read()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_gtp_finished(int, QProcess::ExitStatus)));

    commandList.push_back( commandPtr(new command(this, eListCommands, "list_commands\n")) );
}

/**
* Constructor
*/
gtp::gtp(QProcess* proc, QObject* parent)
    : PlayGame(parent)
    , process(proc)
    , initialized(false)
    , index(0)
    , commandProcessing(false)
    , level_(0)
    , isKilled(false)
{
    connect(process, SIGNAL(readyRead()), this, SLOT(on_gtp_read()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_gtp_finished(int, QProcess::ExitStatus)));

    commandList.push_back( commandPtr(new command(this, eListCommands, "list_commands\n")) );
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
        commandList.push_back( commandPtr(new command(this, eUndo, "undo\n")) );
        commandList.push_back( commandPtr(new command(this, eUndo, "undo\n")) );
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

    return true;
}

/**
* put stone.
*/
bool gtp::put(go::color color, int x, int y){
//    if (moving())
//        return false;

    commandList.push_back( commandPtr(new putCommand(this, boardWidget_, color, x, y)) );

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

    return true;
}

/**
* quit game.
* send quit command.
*/
bool gtp::quit(bool resign){
    isResign_ = resign;
    isAbort_  = false;
    if (!initialized || supportedCommandList.indexOf("quit") != -1){
        commandList.push_back( commandPtr(new command(this, eQuit, "quit\n")) );
        return true;
    }
    return false;
}

/**
* abort game.
* send quit command.
*/
bool gtp::abort(){
    isResign_ = false;
    isAbort_  = true;
    if (!initialized || supportedCommandList.indexOf("quit") != -1){
        commandList.push_back( commandPtr(new command(this, eQuit, "quit\n")) );
        return true;
    }
    return false;
}

/**
* kill porcess
* send quit command.
*/
void gtp::kill(){
    isKilled = true;
    process->close();
}

/**
* get dead stones.
* send final_status_list dead command.
*/
bool gtp::deadList(){
    if (!initialized || supportedCommandList.indexOf("final_status_list") != -1){
        commandList.push_back( commandPtr(new command(this, eDeadList, "final_status_list dead\n")) );
        return true;
    }
    return false;
}

/**
* get name
*/
bool gtp::name(){
    if (!initialized || supportedCommandList.indexOf("name") != -1){
        commandList.push_back( commandPtr(new command(this, eName, "name\n")) );
        return true;
    }
    return false;
}

/**
* get version
*/
bool gtp::version(){
    if (!initialized || supportedCommandList.indexOf("version") != -1){
        commandList.push_back( commandPtr(new command(this, eVersion, "version\n")) );
        return true;
    }
    return false;
}

/**
* set board size
*/
bool gtp::boardSize(int size){
// mogo can process boardsize command, but mogo doesn't list this command.
//    if (!initialized || supportedCommandList.indexOf("boardsize") != -1){
        commandList.push_back( commandPtr(new command(this, eBoardSize, QString().sprintf("boardsize %d\n", size))) );
        return true;
//    }
//    return false;
}

/**
* set komi
*/
bool gtp::komi(double komi){
    if (!initialized || supportedCommandList.indexOf("komi") != -1){
        commandList.push_back( commandPtr(new command(this, eKomi, QString().sprintf("komi %f\n", komi))) );
        return true;
    }
    return false;
}

/**
*
*/
void gtp::pushCommandList(const QString& buf){
//    QString msg = QString("%1 %2").arg(index++).arg(buf);
//    commandStringList.push_back(msg);
    commandStringList.push_back(buf);
    write();
}

/**
* send gtp command to game engine.
*/
void gtp::write(){
    if (commandProcessing == false && commandStringList.empty() == false){
        qDebug() << commandStringList.front();

        commandProcessing = true;
        process->write( commandStringList.front().toAscii() );
        commandStringList.pop_front();
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
    if (!isKilled){  // crashed when delete process after kill process.
        emit gameEnded();
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
                boardWidget_->addTerritory(bx, by);
        }
    }
    else if (command->kind == eUndo)
        boardWidget_->forward(-1);
    else if (command->kind == eListCommands){
        supportedCommandList = s.split("\n");
        initialize();
    }
    else if (command->kind == eName)
        emit getName(msg);
    else if (command->kind == eVersion)
        emit getVersion(msg);
    else if (command->kind == eLoadSgf)
        tempFile.remove();
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
    initialized = true;
    if (boardWidget_ == NULL)
        return;

    boardSize(boardSize_);
    komi(komi_);

    if (supportedCommandList.indexOf("level") != -1)
        commandList.push_back( commandPtr(new levelCommand(this, level_)) );

    if (isNewGame()){
        setHandicap();
        if ((color_ == go::white && handicap_ == 0) || (color_ == go::black && handicap_ > 0))
            wait();
    }
    else{
        restoreSgf();
        if (boardWidget_->getColor() != color_)
            wait();
    }
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
            commandList.push_back( commandPtr(new command(this, eNone, QString("play black %1\n").arg(xy))) );
        else if (node->color == go::white)
            commandList.push_back( commandPtr(new command(this, eNone, QString("play white %1\n").arg(xy))) );

        foreach(const go::stone& stone, node->blackStones){
            xy = boardWidget_->getXYString(stone.p.x, stone.p.y, false);
            commandList.push_back( commandPtr(new command(this, eNone, QString("play black %1\n").arg(xy))) );
        }

        foreach(const go::stone& stone, node->whiteStones){
            xy = boardWidget_->getXYString(stone.p.x, stone.p.y, false);
            commandList.push_back( commandPtr(new command(this, eNone, QString("play white %1\n").arg(xy))) );
        }

        if (node == boardWidget_->getCurrentNode())
            break;
    }
}

/**
* load sgf file to gtp engine if supported.
*/
bool gtp::loadSgf(){
    if (supportedCommandList.indexOf("loadsgf") == -1)
        return false;

    if (tempFile.open() == false)
        return false;
    tempFile.write( QString().sprintf("(;GM[1]SZ[%d]KM[%.1f]", boardSize_, komi_).toAscii() );

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

    return true;
}
