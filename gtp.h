#ifndef GTP_H
#define GTP_H

#include <QProcess>
#include <QTemporaryFile>
#include <boost/shared_ptr.hpp>
#include "playgame.h"

/**
* play game with program using gtp protocol.
*/
class gtp : public PlayGame{
Q_OBJECT
public:
    enum eKind{ eNone, eListCommands, eName, eVersion, eLoadSgf, eBoardSize, eKomi, eLevel, eMove, eGen, ePut, eQuit, eDeadList, eUndo };
    enum eStatus{ eProcessing, eSuccess, eFailure };

    class command{
        public:
            command(gtp* parent_, eKind k) : parent(parent_), kind(k), status(eProcessing){}
            command(gtp* parent_, eKind k, const QString& msg) : parent(parent_), kind(k), status(eProcessing){ execute(msg); }

            void execute(const QString& msg){ parent->pushCommandList(msg); }

            gtp*  parent;
            eKind kind;
            eStatus status;
    };

    class levelCommand : public command{
        public:
            levelCommand(gtp* parent, int level_) : command(parent, eLevel, QString().sprintf("level %d\n", level_)), level(level_){}
            int level;
    };

    class moveBaseCommand : public command{
        public:
            moveBaseCommand(gtp* parent, eKind k, BoardWidget* boardWidget, go::color color, int x_, int y_);
            int x, y;
    };

    class moveCommand : public moveBaseCommand{
        public:
            moveCommand(gtp* parent, BoardWidget* boardWidget, go::color color, int x_, int y_) : moveBaseCommand(parent, eMove, boardWidget, color, x_, y_){}
    };

    class putCommand : public moveBaseCommand{
        public:
            putCommand(gtp* parent, BoardWidget* boardWidget, go::color color, int x_, int y_) : moveBaseCommand(parent, ePut, boardWidget, color, x_, y_){}
    };

    class genmoveCommand : public command{
        public:
            genmoveCommand(gtp* parent, go::color c) : command(parent, eGen, c == go::black ? "genmove black\n" : "genmove white\n"), color(c){}
            go::color color;
    };

    typedef boost::shared_ptr<command> commandPtr;

    gtp(BoardWidget* board, go::color color, int boardSize, const qreal& komi, int handicap, bool newGame, int level, QProcess* process, QObject* parent=0);
    gtp(QProcess* process, QObject* parent=0);
    ~gtp();

    virtual bool undo();
    virtual bool move(int x, int y);
    virtual bool put(go::color c, int x, int y);
    virtual bool wait();
    virtual bool quit(bool resign);
    virtual bool abort();
    virtual void kill();
    virtual bool deadList();
    virtual bool moving() const;

    bool name();
    bool version();
    bool boardSize(int size);
    bool komi(double komi);

    void pushCommandList(const QString& s);

    QProcess* getProcess(){ return process; }

signals:
    void getName(const QString&);
    void getVersion(const QString&);

private:
    void write();
    void initialize();
    bool stringToPosition(const QString& buf, int& x, int& y);
    void processCommand(QString& s);
    void restoreSgf();
    bool loadSgf();

    QProcess* process;
    QString   gtpBuf;

    bool initialized;
    int index;
    QList<commandPtr> commandList;
    QStringList commandStringList;
    bool commandProcessing;
    int level_;
    bool isKilled;
    QStringList supportedCommandList;
    QTemporaryFile tempFile;

private slots:
    void on_gtp_read();
    void on_gtp_finished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // GTP_H
