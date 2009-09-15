#ifndef __gib_h__
#define __gib_h__

#include "godata.h"

namespace go{


class gib : public fileBase{
public:
    class data{
    public:
        typedef QList<data> dataList;

        data() : x(-1), y(-1), color(go::empty){}

        int x;
        int y;
        int color;

        bool operator ==(const data& d) const{
            return x == d.x && y == d.y && color == d.color;
        }
    };
    typedef data::dataList   dataList;

    virtual bool readStream(QString::iterator& first, QString::iterator last);
    virtual bool saveStream(QTextStream& stream);

    virtual bool get(go::data& data) const;
    virtual bool set(const go::data& data);

private:
    bool get(dataList::const_iterator first, dataList::const_iterator last, go::nodePtr parent) const;

    bool readHeader(QString::iterator& first, QString::iterator& last);
    bool readGame(QString::iterator& first, QString::iterator& last);

    bool readINI(int handicap);
    bool readSTO(int x, int y, int color);

    dataList dataList_;
    int handicap;
};


}



#endif
