#ifndef __ngf_h__
#define __ngf_h__

#include "godata.h"

namespace go{


class ngf : public fileBase{
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

    ngf() : handicap(0){}

    virtual bool readStream(QString::iterator& first, QString::iterator last);
    virtual bool saveStream(QTextStream& stream);
    virtual QTextCodec* getCodec(const QByteArray&) const;

    virtual bool get(go::data& data) const;
    virtual bool set(const go::data& data);

private:
    bool get(dataList::const_iterator first, dataList::const_iterator last, go::nodePtr parent) const;

    dataList dataList_;
    QString whitePlayer;
    QString blackPlayer;
    QString gameDate;
    QString gameName;
    QString result;
    QString gameTime;
    int size;
    int handicap;
};


}



#endif
