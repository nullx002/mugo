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
    enum eKind{ eNone, eListCommands, eName, eVersion, eLoadSgf, eInitialInfluence, eBoardSize, eKomi, eLevel, eMove, eGen, ePut, eQuit, eDeadList, eUndo };
    enum eStatus{ eProcessing, eSuccess, eFailure };
    enum eMode{ eNoMode, ePlayGame, eEstimateScore };

    class command{
        public:
            command(gtp* parent_, eKind k) : parent(parent_), kind(k), status(eProcessing){}
            command(gtp* parent_, eKind k, const QString& cmd, const QString& param = QString()) : parent(parent_), kind(k), status(eProcessing){ execute(cmd, param); }

            void execute(const QString& cmd_, const QString& param_ = QString()){ cmd = cmd_; param = param_; }

            gtp*  parent;
            eKind kind;
            eStatus status;
            QString cmd;
            QString param;
    };

    class levelCommand : public command{
        public:
            levelCommand(gtp* parent, int level_) : command(parent, eLevel, "level", QString().number(level_)), level(level_){}
            int level;
    };

    class moveBaseCommand : public command{
        public:
            moveBaseCommand(gtp* parent, eKind k, BoardWidget* boardWidget, go::color color, int x, int y);
            int x, y;
    };

    class moveCommand : public moveBaseCommand{
        public:
            moveCommand(gtp* parent, BoardWidget* boardWidget, go::color color, int x, int y) : moveBaseCommand(parent, eMove, boardWidget, color, x, y){}
    };

    class putCommand : public moveBaseCommand{
        public:
            putCommand(gtp* parent, BoardWidget* boardWidget, go::color color, int x, int y, eKind kind=ePut) : moveBaseCommand(parent, kind, boardWidget, color, x, y){}
    };

    class genmoveCommand : public command{
        public:
            genmoveCommand(gtp* parent, go::color c) : command(parent, eGen, "genmove", c == go::black ? "black" : "white"), color(c){}
            go::color color;
    };

    typedef boost::shared_ptr<command> commandPtr;

    gtp(BoardWidget* board, go::color color, bool newGame, int level, QProcess* process, QObject* parent=0);
    gtp(BoardWidget* board, QProcess* process, QObject* parent=0);
    gtp(QProcess* process, QObject* parent=0);
    ~gtp();

    virtual bool isInitialized() const{ return initialized_; }
    virtual void kill();
    virtual bool moving() const;
    virtual bool undo();
    virtual bool move(int x, int y);
    virtual bool put(go::color c, int x, int y);
    virtual bool put(go::color c, int x, int y, eKind kind);
    virtual bool wait();
    virtual bool quit(bool resign);
    virtual bool abort();

    void list_commands();
    void name();
    void version();
    void boardSize(int size);
    void komi(double komi);
    void level(int level);
    void estimateScore();
    bool deadList();

    QProcess* getProcess(){ return process; }
    void  setMode(eMode mode){ mode_ = mode; }
    eMode getMode() const{ return mode_; }
    const QStringList& getCommandList() const{ return supportedCommandList; }
    const QString& getName() const{ return name_; }
    const QString& getVersion() const{ return version_; }

signals:
    void initialized();
    void getName(const QString&);
    void getVersion(const QString&);
    void invalidResponseReceived(const QString&);
    void scoreEstimated(BoardWidget*, const QVector< QVector<double> >&);

private:
    void write();
    void initialize();
    bool stringToPosition(const QString& buf, int& x, int& y);
    void processCommand(QString& s);
    void restoreSgf();
    bool loadSgf();
    void initialInfluence(const QString& msg);

    QProcess* process;
    QString   gtpBuf;

    bool initialized_;
    QString name_;
    QString version_;

    int index;
    QList<commandPtr> commandList;
    bool commandProcessing;
    int level_;
    bool isKilled_;
    eMode mode_;
    QStringList supportedCommandList;
    QTemporaryFile tempFile;

private slots:
    void on_gtp_read();
    void on_gtp_finished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // GTP_H
