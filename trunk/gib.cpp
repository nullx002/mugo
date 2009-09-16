#include <QDebug>
#include <QRegExp>
#include "appdef.h"
#include "gib.h"

namespace go{



bool gib::readStream(QString::iterator& first, QString::iterator last){
//    dataList_.push_back( data() );
    while (first != last){
        QString s = readLine(first, last);
        if (s == "\\HS")
            readHeader(first, last);
        else if (s == "\\GS")
            readGame(first, last);
    }

    return true;
}

bool gib::saveStream(QTextStream& stream){
    return false;
}

bool gib::get(go::data& data) const{
    int hcapx1[] = {15, 3, 15, 3, 9};
    int hcapx2[] = {15, 3, 15, 3, 15, 3, 9};
    int hcapx3[] = {15, 3, 15, 3, 15, 3, 9, 9, 9};
    int hcapy1[] = {3, 15, 15, 3, 9};
    int hcapy2[] = {3, 15, 15, 3, 9, 9, 9};
    int hcapy3[] = {3, 15, 15, 3, 9, 9, 15, 3, 9};

    data.root->xsize = 19;
    data.root->ysize = 19;
    data.root->whitePlayer = whitePlayer;
    data.root->whiteRank   = whiteRank;
    data.root->blackPlayer = blackPlayer;
    data.root->blackRank   = blackRank;
    data.root->comment     = comment;
    data.root->place       = place;
    data.root->date        = gameDate;
    data.root->gameName    = gameName;
    data.root->result      = result;
    data.root->annotation  = condition;
    data.root->time        = gameTime;
    if (handicap)
        data.root->komi = 0;
    data.root->handicap = handicap;
    for (int i=0; i<handicap; ++i){
        if (handicap < 6)
            data.root->blackStones.push_back( stone(hcapx1[i], hcapy1[i], go::black) );
        else if (handicap < 8)
            data.root->blackStones.push_back( stone(hcapx2[i], hcapy2[i], go::black) );
        else
            data.root->blackStones.push_back( stone(hcapx3[i], hcapy3[i], go::black) );
    }
    get(dataList_.begin(), dataList_.end(), go::nodePtr(data.root));
    return true;
}

bool gib::set(const go::data& data){
    return false;
}

bool gib::get(dataList::const_iterator first, dataList::const_iterator last, go::nodePtr parent) const{
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

bool gib::readHeader(QString::iterator& first, QString::iterator& last){
    QString str;
    while (first != last){
        str.append( readLine(first, last) );
        if (str == "\\HE")
            return true;

        // \[%1=%2\]
        QRegExp rx("\\\\\\[(.+)=(.*)\\\\\\]");
        int pos = rx.indexIn(str);
        if (pos == -1){
            str.push_back(' ');
            continue;
        }
        str.clear();

        QStringList list = rx.capturedTexts();
        if (list.size() < 3)
            continue;

        if (list[1] == "GAMEWHITENICK")
            whitePlayer = list[2];
        else if (list[1] == "GAMEWHITELEVEL")
            whiteRank = list[2];
        else if (list[1] == "GAMEBLACKNICK")
            blackPlayer = list[2];
        else if (list[1] == "GAMEBLACKLEVEL")
            blackRank = list[2];
        else if (list[1] == "GAMECOMMENT")
            comment = list[2];
        else if (list[1] == "GAMEPLACE")
            place = list[2];
        else if (list[1] == "GAMEDATE")
            gameDate = list[2];
        else if (list[1] == "GAMENAME")
            gameName = list[2];
        else if (list[1] == "GAMERESULT")
            result = list[2];
        else if (list[1] == "GAMECONDITION")
            condition = list[2];
        else if (list[1] == "GAMETIME")
            gameTime = list[2];
    }

    return false;
}

bool gib::readGame(QString::iterator& first, QString::iterator& last){
    QString str;
    while (first != last){
        str = readLine(first, last);
        if (str == "\\GE")
            return true;
        QStringList list = str.split(' ');
        if (list.size() < 3)
            continue;
        else if (list[0].compare("INI", Qt::CaseInsensitive) == 0){
            readINI(list[3].toInt());
            continue;
        }

        if (list.size() < 6)
            continue;
        else if (list[0].compare("STO", Qt::CaseInsensitive) == 0)
            readSTO(list[4].toInt(), list[5].toInt(), list[3].toInt());
    }

    return false;
}

bool gib::readINI(int hcap){
    if (handicap < 0 || handicap > 9)
        return false;
    handicap = hcap;
    return true;
}

bool gib::readSTO(int x, int y, int color){
    data d;

    d.x = x;
    d.y = y;

    if (color == 1)
        d.color = go::black;
    else if (color == 2)
        d.color = go::white;
    else
        return false;

    dataList_.push_back(d);

    return true;
}



};
