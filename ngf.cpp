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
#include <QRegExp>
#include "appdef.h"
#include "ngf.h"

namespace go{



bool ngf::readStream(QString::iterator& first, QString::iterator last){
//    dataList_.push_back( data() );
    gameName = readLine(first, last);
    size = readLine(first, last).toInt();
    whitePlayer = readLine(first, last);
    blackPlayer = readLine(first, last);
    readLine(first, last); // url?
    handicap = readLine(first, last).toInt(); //
    readLine(first, last); // 0?
    readLine(first, last); // 6?
    gameDate = readLine(first, last);
    gameTime = readLine(first, last);
    result   = readLine(first, last);
    readLine(first, last); // last move number

    while (first != last){
        QString s = readLine(first, last);
        if (s.size() < 7)
            continue;

        data d;
        if (s[4] == 'B')
            d.color = go::black;
        else if (s[4] == 'W')
            d.color = go::white;
        else
            continue;

        d.x = s[5].toAscii() - 'A' - 1;
        d.y = s[6].toAscii() - 'A' - 1;

        dataList_.push_back(d);
    }

    return true;
}

bool ngf::saveStream(QTextStream& /*stream*/){
    return false;
}

QTextCodec* ngf::getCodec(const QByteArray&) const{
    return QTextCodec::codecForName("Shift_JIS");
}

bool ngf::get(go::data& data) const{
    data.clear();

    int left = size > 9 ? 3 : 2;
    int right = size > 9 ? size-4 : size-3;
    int center = size / 2;
    int hcapx1[] = {right, left, right, left, center};
    int hcapx2[] = {right, left, right, left, right, left, center};
    int hcapx3[] = {right, left, right, left, right, left, center, center, center};
    int hcapy1[] = {left, right, right, left, center};
    int hcapy2[] = {left, right, right, left, center, center, center};
    int hcapy3[] = {left, right, right, left, center, center, right, left, center};

    data.root->xsize = size;
    data.root->ysize = size;
    data.root->whitePlayer = whitePlayer;
    data.root->blackPlayer = blackPlayer;
    data.root->date        = gameDate;
    data.root->gameName    = gameName;
    data.root->result      = result;
    data.root->time        = gameTime;
    if (handicap)
        data.root->komi = 0;
    data.root->handicap = handicap;
    int hcap = (size % 2 == 0 && handicap > 4) ? 4 : handicap;
    for (int i=0; i<hcap; ++i){
        if (hcap < 6)
            data.root->blackStones.push_back( stone(hcapx1[i], hcapy1[i], go::black) );
        else if (hcap < 8)
            data.root->blackStones.push_back( stone(hcapx2[i], hcapy2[i], go::black) );
        else
            data.root->blackStones.push_back( stone(hcapx3[i], hcapy3[i], go::black) );
    }
    get(dataList_.begin(), dataList_.end(), go::nodePtr(data.root));
    return true;
}

bool ngf::set(const go::data& /*data*/){
    return false;
}

bool ngf::get(dataList::const_iterator first, dataList::const_iterator last, go::nodePtr parent) const{
    if (first == last)
        return true;

    nodePtr node;
    if (first->color == go::black)
        node = go::createBlackNode(parent, first->x, first->y);
    else if (first->color == go::white)
        node = go::createWhiteNode(parent, first->x, first->y);
    else
        node.reset(new go::node(parent));

    parent->childNodes.push_back(node);

    get(++first, last, node);

    return true;
}



};
