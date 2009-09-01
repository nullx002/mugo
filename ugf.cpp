#include <QDebug>
#include "appdef.h"
#include "ugf.h"

namespace go{



bool ugf::readStream(QString::iterator& first, QString::iterator last){
    dataList_.push_back( data() );
    while (first != last){
        QString s = readLine(first, last);
        if (s == "[Header]")
            readHeader(first, last);
        else if (s == "[Remote]")
            readRemote(first, last);
        else if (s == "[Files]")
            readFiles(first, last);
        else if (s == "[Data]")
            readData(first, last);
        else if (s == "[Figure]")
            readFigure(first, last);
        else if (s == "[Comment]")
            readComment(first, last);
    }

    return false;
}

bool ugf::saveStream(QTextStream& /*stream*/){
    return false;
}

QTextCodec* ugf::getCodec(const QByteArray& a) const{
    int s = a.indexOf("\nLang=");
    if (s == -1)
        return NULL;
    s += 6;

    int e = a.indexOf('\n', s);
    if (e == -1)
        return NULL;

    return QTextCodec::codecForName( a.mid(s, e-s) );
}

bool ugf::get(go::data& data) const{
    data.clear();

    data.root->gameName = title;
    data.root->place = place;
    data.root->date = date;
    data.root->rule = rule;
    data.root->xsize = size;
    data.root->ysize = size;
    data.root->handicap = handicap;
    data.root->komi = komi;
    data.root->result = winner;
    data.root->user = writer;
    data.root->copyright = copyright;
    data.root->whitePlayer = whitePlayer;
    data.root->whiteRank = whiteRank;
    data.root->blackPlayer = blackPlayer;
    data.root->blackRank = blackRank;

    dataList::const_iterator first = dataList_.begin();
    data.root->comment = first->comment;
    markerList::const_iterator marker = first->markers.begin();
    while (marker != first->markers.end()){
        data.root->characters.push_back( go::mark(marker->x, marker->y, marker->str) );
        ++marker;
    }

    stoneList::const_iterator stone = first->stones.begin();
    while (stone != first->stones.end()){
        data.root->stones.push_back( go::stone(stone->x, stone->y, (go::color)stone->color) );
        ++stone;
    }

    return get(++first, dataList_.end(), data.root);
}

bool ugf::set(const go::data& /*data*/){
    return false;
}

QString ugf::readLine(QString::iterator& first, QString::iterator& last){
    QString str;
    while (first != last){
        QChar c = *first++;
        if (c == '\r')
            continue;
        else if (c == '\n')
            break;
        str.push_back(c);
    }
    return str;
}

bool ugf::readHeader(QString::iterator& first, QString::iterator& last){
    while (first != last && *first != '['){
        QString str = readLine(first, last);

        int pos = str.indexOf('=');
        if (pos == -1)
            continue;

        QString key   = str.left(pos);
        QString value = str.mid(pos + 1);

        if (key == "Title")
            title = value;
        else if (key == "Place")
            place = value;
        else if (key == "Date")
            date = value;
        else if (key == "Rule"){
            if (value == "JPN")
                rule = "Japanese";
            else
                rule = value;
        }
        else if (key == "Size")
            size = value.toInt();
        else if (key == "Hdcp"){
            QStringList list = value.split(',');
            handicap = list[0].toInt();
            if (list.size() > 1)
                komi = list[1].toDouble();
        }
        else if (key == "Winner"){
            QStringList list = value.split(',');
            winner = list[0];
            if (list.size() > 1){
                if (list[1] == "C")
                    winner.append("+R");
                else
                    winner += "+" + list[1];
            }
        }
        else if (key == "Writer")
            writer = value;
        else if (key == "Copyright")
            copyright = value;
        else if (key == "Comment")
            comment = value;
        else if (key == "WMemb1" || key == "PlayerW"){
            QStringList list = value.split(',');
            whitePlayer = list[0];
            if (list.size() > 1)
                whiteRank = list[1];
        }
        else if (key == "BMemb1" || key == "PlayerB"){
            QStringList list = value.split(',');
            blackPlayer = list[0];
            if (list.size() > 1)
                blackRank = list[1];
        }
    }

    return true;
}

bool ugf::readRemote(QString::iterator& first, QString::iterator& last){
    while (first != last && *first != '['){
        QString str = readLine(first, last);
    }

    return false;
}

bool ugf::readFiles(QString::iterator& first, QString::iterator& last){
    while (first != last && *first != '['){
        QString str = readLine(first, last);
    }

    return false;
}

bool ugf::readData(QString::iterator& first, QString::iterator& last){
    while (first != last && *first != '['){
        QString str = readLine(first, last);

        QStringList list = str.split(',');
        if (list.size() != 4)
            continue;

        if (list[0].size() < 2)
            continue;

        data d;
        if (getData(list, d)){
            data* d2 = getNthData(list[2].toInt());
            if (d2)
                d2->stones.push_back( stone(d.x, d.y, d.color) );
            else
                dataList_.push_back(d);
        }
    }

    return true;
}

bool ugf::readFigure(QString::iterator& first, QString::iterator& last){
    while (first != last && *first != '['){
        QString str = readLine(first, last);

        QStringList list =  str.split(',');
        if (list.size() < 2)
            continue;

        if (list[0] == ".Text")
            readFigureText(first, last, list[1].toInt());
    }

    return false;
}

bool ugf::readFigureText(QString::iterator& first, QString::iterator& last, int index){
    data* d = getNthData(index);
    if (d == NULL)
        return false;

    while (first != last && *first != '['){
        QString str = readLine(first, last);
        if (!str.isEmpty() && str[0] == '.'){
            if (str == ".EndText")
                break;

            QStringList list =  str.split(',');
            if (list[0] == ".#" && list.size() >= 4)
                d->markers.push_back( marker(list[1].toInt()-1, list[2].toInt()-1, list[3]) );
        }
        else{
            if (!d->comment.isEmpty())
                d->comment.push_back('\n');
            d->comment.append(str);
        }
    }

    return true;
}

bool ugf::readComment(QString::iterator& first, QString::iterator& last){
    while (first != last && *first != '['){
        QString str = readLine(first, last);

        QStringList list =  str.split(',');
        if (list.size() < 2)
            continue;

        if (list[0] == ".Fig")
            readBranch(first, last, list[1].toInt());
    }

    return true;
}

bool ugf::readBranch(QString::iterator& first, QString::iterator& last, int index){
    data* d = getNthData(index);
    if (d == NULL)
        return false;

    dataList branch;
    while (first != last && *first != '['){
        QString str = readLine(first, last);
        if (str == ".EndFig")
            break;

        QStringList list =  str.split(',');
        if (list.size() < 2)
            continue;

        dataList::iterator b_last = dataList_.begin() + index + 2;
        data d2;
        if (getData(list, d2))
            if (qFind(dataList_.begin(), b_last, d2) == b_last)
                branch.push_back(d2);
    }

    d->branches.push_back(branch);

    return true;
}

bool ugf::get(dataList::const_iterator first, dataList::const_iterator last, go::nodePtr parent) const{
    if (first == last)
        return true;

    nodePtr node;
    if (first->color == go::black)
        node = go::createBlackNode(parent, first->x, first->y);
    else
        node = go::createWhiteNode(parent, first->x, first->y);

    node->comment = first->comment;

    markerList::const_iterator marker = first->markers.begin();
    while (marker != first->markers.end()){
        node->characters.push_back( go::mark(marker->x, marker->y, marker->str) );
        ++marker;
    }

    stoneList::const_iterator stone = first->stones.begin();
    while (stone != first->stones.end()){
        node->stones.push_back( go::stone(stone->x, stone->y, (go::color)stone->color) );
        ++stone;
    }

    parent->childNodes.push_back(node);

    get(first+1, last, node);

    branchList::const_iterator b_first = first->branches.begin();
    branchList::const_iterator b_last  = first->branches.end();
    while (b_first != b_last){
        get(b_first->begin(), b_first->end(), node);
        ++b_first;
    }

    return true;
}

bool ugf::getData(const QStringList& list, data& d){
    if (list[1].size() < 2)
        return false;

    if (list[1][0] != 'B' && list[1][0] != 'W')
        return false;

    d.x = list[0][0].toAscii() - 'A';
    d.y = list[0][1].toAscii() - 'A';
    d.index = list[2].toInt();
    d.color = list[1][0] == 'B' ? go::black : go::white;

    return true;
}

ugf::data* ugf::getNthData(int index){
    for (int i=0; i<dataList_.size(); ++i)
        if (dataList_[i].index == index)
            return &dataList_[i];
    return NULL;
}



};
