#ifndef GTP_H
#define GTP_H

#include <QProcess>
#include <boost/shared_ptr.hpp>
#include "playgame.h"

class gtp : public PlayGame{
Q_OBJECT
public:
    enum eStatus{ eMove, eGen, ePut, eQuit, eDeadList };

    class command{
        public:
            command(eStatus s) : status(s){}

            eStatus status;
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

    typedef boost::shared_ptr<command> commandPtr;

    gtp(BoardWidget* board, go::color color, QProcess& process, QObject* parent=0);

    virtual void move(int x, int y);
    virtual void put(go::color c, int x, int y);
    virtual void wait();
    virtual void quit();
    virtual void deadList();

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
