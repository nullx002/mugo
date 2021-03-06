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
