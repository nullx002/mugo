#ifndef GTP_H
#define GTP_H

#include <QProcess>
#include <boost/shared_ptr.hpp>
#include "playgame.h"

class gtp : public PlayGame{
Q_OBJECT
public:
    enum eKind{ eNone, eBoardSize, eKomi, eLevel, eMove, eGen, ePut, eQuit, eDeadList, eUndo };
    enum eStatus{ eProcessing, eSuccess, eFailure };

    class command{
        public:
            command(eKind k) : kind(k), status(eProcessing){}

            eKind kind;
            eStatus status;
    };

    class boardSizeCommand : public command{
        public:
            boardSizeCommand(int boardSize_) : command(eBoardSize), boardSize(boardSize_){}
            int boardSize;
    };

    class komiCommand : public command{
        public:
            komiCommand(const qreal& komi_) : command(eKomi), komi(komi_){}
            qreal komi;
    };

    class levelCommand : public command{
        public:
            levelCommand(int level_) : command(eLevel), level(level_){}
            int level;
    };

    class moveCommand : public command{
        public:
            moveCommand(int x_, int y_) : command(eMove), x(x_), y(y_){}
            int x, y;
    };

    class putCommand : public command{
        public:
            putCommand(int x_, int y_) : command(ePut), x(x_), y(y_){}
            int x, y;
    };

    class genmoveCommand : public command{
        public:
            genmoveCommand(go::color c) : command(eGen), color(c){}
            go::color color;
    };

    class undoCommand : public command{
        public:
            undoCommand() : command(eUndo){}
    };

    typedef boost::shared_ptr<command> commandPtr;

    gtp(BoardWidget* board, go::color color, bool newGame, int boardSize, const qreal& komi, int handicap, int level, QProcess& process, QObject* parent=0);

    virtual bool undo();

    virtual bool move(int x, int y);
    virtual bool put(go::color c, int x, int y);
    virtual bool wait();
    virtual void quit();
    virtual void deadList();
    virtual bool moving() const;

private:
    void write(const QString& s);
    bool stringToPosition(const QString& buf, int& x, int& y);

    QProcess& process;
    QString   gtpBuf;

    int index;
    QList<commandPtr> commandList;

private slots:
    void gtpRead();
};

#endif // GTP_H
