#ifndef __ugf_h__
#define __ugf_h__

#include "godata.h"

namespace go{


class ugf : public fileBase{
public:
    class marker{
    public:
        marker(int x_, int y_, const QString& str_) : x(x_), y(y_), str(str_){}

        int x;
        int y;
        QString str;
    };
    typedef QList<marker> markerList;

    class stone{
    public:
        stone(int x_, int y_, int color_, int originalColor_ = 0) : x(x_), y(y_), color(color_), originalColor(originalColor_){}

        int x;
        int y;
        int color;
        int originalColor;
    };
    typedef QList<stone> stoneList;

    class data{
    public:
        typedef QList<data> dataList;
        typedef QList<dataList> branchList;

        data() : x(-1), y(-1), index(0), color(go::empty){}

        int x;
        int y;
        int index;
        int color;
        QString comment;
        markerList markers;
        stoneList  stones;
        branchList branches;

        bool operator ==(const data& d) const{
            return x == d.x && y == d.y && color == d.color;
        }
    };
    typedef data::dataList   dataList;
    typedef data::branchList branchList;


    virtual bool readStream(QString::iterator& first, QString::iterator last);
    virtual bool saveStream(QTextStream& stream);
    virtual QTextCodec* getCodec(const QByteArray&) const;

    virtual bool get(go::data& data) const;
    virtual bool set(const go::data& data);

private:
    QString readLine(QString::iterator& first, QString::iterator& last);
    bool    readHeader(QString::iterator& first, QString::iterator& last);
    bool    readRemote(QString::iterator& first, QString::iterator& last);
    bool    readFiles(QString::iterator& first, QString::iterator& last);
    bool    readData(QString::iterator& first, QString::iterator& last);
    bool    readFigure(QString::iterator& first, QString::iterator& last);
    bool    readComment(QString::iterator& first, QString::iterator& last);
    bool    readFig(QString::iterator& first, QString::iterator& last, int index);
    bool    readText(QString::iterator& first, QString::iterator& last, int index);
    void    replaceStone(stoneList& stones, const data& d);

    bool get(dataList::const_iterator first, dataList::const_iterator last, go::nodePtr parent) const;
    bool getData(const QStringList& list, data& d);
    data* getNthData(int index);

    dataList dataList_;
    QString title;
    QString place;
    QString date;
    QString rule;
    int     size;
    int     handicap;
    double  komi;
    QString winner;
    QString writer;
    QString copyright;
    QString comment;
    QString whitePlayer;
    QString whiteRank;
    QString blackPlayer;
    QString blackRank;
    QString coordinateType;
};


}



#endif
