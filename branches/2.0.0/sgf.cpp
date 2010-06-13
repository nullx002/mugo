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
#include "mugoapp.h"
#include "sgf.h"

namespace Go{



/**
  read sgf file
*/
bool Sgf::readStream(QString::iterator& first, QString::iterator last){
    while (first != last){
        wchar_t c = first->unicode();
        ++first;
        if (c == L'('){
            NodePtr root(new Node());
            if (readBranch(first, last, root) == false)
                continue;

            if (root->childNodes.empty())
                continue;

            gameList.push_back(root);
        }
    }

    return gameList.isEmpty() == false;
}

/**
  get codec
*/
QTextCodec* Sgf::guessCodec(const QByteArray& a) const{
    int s = a.indexOf("CA[");
    if (s == -1)
        return NULL;
    s += 3;

    int e = a.indexOf(']', s);
    if (e == -1)
        return NULL;

    QString name = a.mid(s, e-s);
    if (name.compare("windows-31j", Qt::CaseInsensitive) == 0)
        return QTextCodec::codecForName("Shift_JIS");
    else
        return QTextCodec::codecForName(name.toAscii());
}


/**
  read branch
*/
bool Sgf::readBranch(QString::iterator& first, QString::iterator last, NodePtr& node){
    int pt = 1;

    while (first != last){
        wchar_t c = first->unicode();
        ++first;

        if (c == L')')
            return true;
        // only ';' or '(' appears hear, but ';' does not exist in cyber oro sgf.
        else if (iswspace(c))
            continue;

        NodePtr newNode( new Node );
        if (c == L'('){
            node->childNodes.push_back(newNode);

            readBranch(first, last, newNode);
        }
        else{
            if (c != ';')
                --first;

            if (pt == 1)
                node->childNodes.push_back(newNode);
            else
                --pt;
            readNode(first, last, newNode, pt);
        }
    }

    return true;
}

/**
  read node
*/
bool Sgf::readNode(QString::iterator& first, QString::iterator last, NodePtr& node, int& pt){
    while (first != last){
        wchar_t c = first->unicode();
        if (iswspace(c)){
            ++first;
            continue;
        }
        else if (c == L';' || c == L'(' || c == L')')
            break;

        QString key;
        key.reserve(5);
        if (readNodeKey(first, last, key) == false)
            return false;

        QStringList values;
        if (readNodeValues(first, last, values) == false)
            return false;

        node->properties[key] += values;

        // for cyber oro?
        if (key == "PT" && values.empty() == false)
            pt = values[0].toInt();
    }

    return true;
}

bool Sgf::readNodeKey(QString::iterator& first, QString::iterator last, QString& key){
    while (first != last){
        wchar_t c = first->unicode();
        if (c == L'[')
            return true;
        else if (c == L';' || c == L'(' || c == L')')
            return false;
        else{
            key.push_back(c);
            ++first;
        }
    }

    return false;
}

bool Sgf::readNodeValues(QString::iterator& first, QString::iterator last, QStringList& values){
    while (first != last){
        wchar_t c = first->unicode();
        if (c == L'['){
            QString v;
            v.reserve(100);
            if (readNodeValue(++first, last, v))
                values.push_back(v);
        }
        else if (iswspace(c))
            ++first;
        else
            return true;
    }

    return false;
}

bool Sgf::readNodeValue(QString::iterator& first, QString::iterator last, QString& value){
    while (first != last){
        wchar_t c = first->unicode();
        ++first;

        if (c == ']')
            return true;

        if (c == '\\'){
            c = first->unicode();
            ++first;
        }
        value.push_back(c);
    }

    return false;
}

bool Sgf::saveStream(QTextStream& stream){
    foreach (NodePtr game, gameList){
        if (codec)
            if (game->childNodes.empty() == false)
                game->childNodes[0]->properties["CA"] = QStringList(codec->name());
        QString s;
        writeNode(stream, s, game);
        stream << "\n";
    }

    return true;
}

bool Sgf::get(Go::NodeList& gameList) const{
    gameList.clear();

    foreach(NodePtr sgfGame, this->gameList){
        Go::NodePtr goGame;
        get(sgfGame, goGame);

        if (goGame){
            if (!goGame->gameInformation)
                goGame->gameInformation.reset(new Go::GameInformation);
            if (goGame->color == empty && goGame->nextColor == empty)
                goGame->nextColor = black;
            gameList.push_back(goGame);
        }
    }

    return gameList.empty() == false;
}

Go::NodePtr Sgf::get(NodePtr& sgfNode, Go::NodePtr& goNode) const{
    if (sgfNode->childNodes.empty() == false){
        Go::NodePtr newNode(goNode);
        foreach(NodePtr child, sgfNode->childNodes){
            newNode = get(child, goNode ? newNode : goNode);
        }

        return goNode;
    }
    else{
        Go::NodePtr newNode( new Go::Node(goNode) );
        sgfNode->get(newNode);
        if (goNode)
            goNode->childNodes.push_back(newNode);
        else
            goNode = newNode;

        return newNode;
    }
}

bool Sgf::set(Go::NodeList& gameList){
    this->gameList.clear();

    foreach(Go::NodePtr game, gameList){
        NodePtr sgfGame(new Node());
        set(sgfGame, game);
        this->gameList.push_back(sgfGame);
    }

    return true;
}

bool Sgf::set(NodePtr& sgfNode, Go::NodePtr goNode){
    NodePtr newNode(new Node);
    newNode->set( goNode );
    sgfNode->childNodes.push_back(newNode);

    if (goNode->childNodes.size() > 1){
        foreach(const Go::NodePtr& childNode, goNode->childNodes){
            NodePtr branchNode(new Node);
            sgfNode->childNodes.push_back(branchNode);
            set(branchNode, childNode);
        }
    }
    else if (goNode->childNodes.size() == 1)
        set(sgfNode, goNode->childNodes.front());

    return true;
}

void Sgf::writeNode(QTextStream& stream, QString& s, NodePtr node){
    if (node->childNodes.empty() == false){
        s.push_back('(');
        stream << '(';
        foreach(NodePtr childNode, node->childNodes)
            writeNode(stream, s, childNode);
        s.push_back(')');
        stream << ')';
    }
    else{
        QString s2 = node->toString();
        if (s.isEmpty() == false && s.size() + s2.size() > lineWidth){
            stream << '\n';
            s.clear();
        }
        stream << s2;
        s.append(s2);
    }
}

bool Sgf::positionToInt(const QString& pos, int& x, int& y, QString* str){
    if (pos.size() < 2)
        return false;
    x = pos[0].unicode() - (pos[0].isLower() ? L'a' : L'A' - 27);
    y = pos[1].unicode() - (pos[1].isLower() ? L'a' : L'A' - 27);

    if (str && pos.size() > 3 && pos[2] == ':')
        *str = pos.mid(3);

    return true;
}

bool Sgf::positionToIntList(const QString& pos, QList<int>& xList, QList<int>& yList){
    QRegExp exp("(..):?(..)?");
    exp.indexIn(pos);
    QStringList list = exp.capturedTexts();

    int x1, y1;
    if (positionToInt(list[1], x1, y1) == false)
        return false;

    int x2, y2;
    if (list[2].isEmpty() || positionToInt(list[2], x2, y2) == false){
        xList.push_back(x1);
        yList.push_back(y1);
        return true;
    }

    for (int y=y1; y<=y2; ++y){
        for (int x=x1; x<=x2; ++x){
            xList.push_back( x );
            yList.push_back( y );
        }
    }

    return true;
}

QString Sgf::positionToString(int x, int y, const QString* s){
    QString buf;
    if (s)
        buf.sprintf("%c%c:%s", x + (x < 27 ? 'a' : 'A' - 27), y + (y < 27 ? 'a' : 'A' - 27), (const char*)s->toAscii());
    else
        buf.sprintf("%c%c", x + (x < 27 ? 'a' : 'A' - 27), y + (y < 27 ? 'a' : 'A' - 27));
    return buf;
}

QString Sgf::positionToString(const Go::Point& p, const QString* s){
    return positionToString(p.x, p.y, s);
}


bool Sgf::Node::get(Go::NodePtr& node) const{
    PropertyType::const_iterator iter = properties.begin();
    while (iter != properties.end()){
        get(iter.key(), iter.value(), node);
        ++iter;
    }

    return true;
}

bool Sgf::Node::get(const QString& key, const QStringList& values, Go::NodePtr& node) const{
    // game information
    if (key == "PW")
        getInformation(node)->whitePlayer = values[0];
    else if (key == "WR")
        getInformation(node)->whiteRank = values[0];
    else if (key == "WT")
        getInformation(node)->whiteTeam = values[0];
    if (key == "PB")
        getInformation(node)->blackPlayer = values[0];
    else if (key == "BR")
        getInformation(node)->blackRank = values[0];
    else if (key == "BT")
        getInformation(node)->blackTeam = values[0];
    else if (key == "RE")
        getInformation(node)->result = values[0];
    else if (key == "SZ"){
        int p = values[0].indexOf(":");
        if (p == -1)
            getInformation(node)->xsize = getInformation(node)->ysize = values[0].toInt();
        else{
            getInformation(node)->xsize = values[0].left(p).toInt();
            getInformation(node)->ysize = values[0].mid(p+1).toInt();
        }
    }
    else if (key == "KM" || key == "KO") // KM is correct, KO is used by cyber oro.
        getInformation(node)->komi = values[0].toDouble();
    else if (key == "HA")
        getInformation(node)->handicap = values[0].toInt();
    else if (key == "TM")
        getInformation(node)->time = values[0];
    else if (key == "OT")
        getInformation(node)->overTime = values[0];
    else if (key == "DT" || key == "RD")  // DT is correct, RD is cyber oro
        getInformation(node)->date = values[0];
    else if (key == "GN" || (node->gameInformation && key == "TE"))  // GN is correct, TE is cyber oro
        getInformation(node)->gameName = values[0];
    else if (key == "RO")
        getInformation(node)->round = values[0];
    else if (key == "PC")
        getInformation(node)->place = values[0];
    else if (key == "EV")
        getInformation(node)->event = values[0];
    else if (key == "GC" || key == "TC") // GC is correct, TC is cyber oro.
        getInformation(node)->gameComment = values[0];
    else if (key == "AN")
        getInformation(node)->annotation = values[0];
    else if (key == "CP")
        getInformation(node)->copyright = values[0];
    else if (key == "ON")
        getInformation(node)->opening = values[0];
    else if (key == "RU")
        getInformation(node)->rule = values[0];
    else if (key == "SO")
        getInformation(node)->source = values[0];
    else if (key == "US")
        getInformation(node)->user = values[0];

    // comment
    else if (key == "C")
        node->comment = values.join("\n");
    else if (key == "N" || key == "RN")  // N is correct, RN is cyber oro version?
        node->name = values[0];
    else if (key == "MN")
        node->moveNumber = values[0].toInt();

    // move
    else if (key == "B" || key == "W"){
        node->color = key == "B" ? Go::black : Go::white;
        int x, y;
        if (positionToInt(values[0], x, y))
            node->position.set(x, y);
    }
    else if (key == "PL")
        node->nextColor = values[0] == "B" ? Go::black : Go::white;

    // mark
    else if (key == "MA" || key == "M")
        addMark(node->marks, values, Mark::cross);
    else if (key == "TR")
        addMark(node->marks, values, Mark::triangle);
    else if (key == "CR")
        addMark(node->marks, values, Mark::circle);
    else if (key == "SQ")
        addMark(node->marks, values, Mark::square);
    else if (key == "SL")
        addMark(node->marks, values, Mark::select);
    else if (key == "LB")
        addMark(node->marks, values);
    else if (key == "TB")
        addMark(node->blackTerritories, values, Mark::blackTerritory);
    else if (key == "TW")
        addMark(node->whiteTerritories, values, Mark::whiteTerritory);
    else if (key == "DD")
        addMark(node->dims, values, Mark::dim);
    else if (key == "AB")
        addStone(node->blackStones, key, values);
    else if (key == "AW")
        addStone(node->whiteStones, key, values);
    else if (key == "AE")
        addStone(node->emptyStones, key, values);

    // move annotation
    else if (key == "TE")
        node->moveAnnotation = values[0].toInt() > 1 ? Go::Node::veryGoodMove : Go::Node::goodMove;
    else if (key == "BM")
        node->moveAnnotation = values[0].toInt() > 1 ? Go::Node::veryBadMove : Go::Node::badMove;
    else if (key == "DO")
        node->moveAnnotation = Go::Node::doubtfulMove;
    else if (key == "IT")
        node->moveAnnotation = Go::Node::interestingMove;

    // node annotation
    else if (key == "DM")
        node->nodeAnnotation = Go::Node::even;
    else if (key == "GB")
        node->nodeAnnotation = values[0].toInt() > 1 ? Go::Node::veryGoodForBlack : Go::Node::goodForBlack;
    else if (key == "GW")
        node->nodeAnnotation = values[0].toInt() > 1 ? Go::Node::veryGoodForWhite : Go::Node::goodForWhite;
    else if (key == "UC")
        node->nodeAnnotation = Go::Node::unclear;

    // annotation
    else if (key == "HO")
        node->annotation = Go::Node::hotspot;

    return true;
}

Go::GameInformationPtr Sgf::Node::getInformation(Go::NodePtr& node) const{
    if (node->gameInformation)
        return node->gameInformation;
    node->gameInformation.reset( new Go::GameInformation() );
    return node->gameInformation;
}

void Sgf::Node::addMark(Go::MarkList& markList, const QStringList& values) const{
    foreach(const QString& s, values){
        int x, y;
        QString str;
        if (positionToInt(s, x, y, &str))
            markList.push_back( Go::Mark(x, y, str) );
    }
}

void Sgf::Node::addMark(Go::MarkList& markList, const QStringList& values, Go::Mark::Type type) const{
    foreach(const QString& s, values){
        QList<int> x, y;
        if (positionToIntList(s, x, y))
            for(int i=0; i<x.size(); ++i)
                markList.push_back( Go::Mark(x[i], y[i], type) );
    }
}

void Sgf::Node::addStone(Go::StoneList& stoneList, const QString& key, const QStringList& values) const{
    Go::Color c = key == "AB" ? Go::black : key == "AW" ? Go::white : Go::empty;

    foreach(const QString& s, values){
        QList<int> x, y;
        if (positionToIntList(s, x, y))
            for (int i=0; i<x.size(); ++i)
                stoneList.push_back( Go::Stone(x[i], y[i], c) );
    }
}

bool Sgf::Node::set(const Go::NodePtr& n){
    if (n->gameInformation){
        properties["GM"].push_back("1");
        properties["FF"].push_back("4");
        properties["AP"].push_back(APP_NAME ":" APP_VERSION);
        if (n->gameInformation->xsize == n->gameInformation->ysize)
            properties["SZ"].push_back( QString("%1").arg(n->gameInformation->xsize) );
        else
            properties["SZ"].push_back( QString("%1:%2").arg(n->gameInformation->xsize).arg(n->gameInformation->ysize) );
        properties["KM"].push_back( QString("%1").arg(n->gameInformation->komi) );

        if (n->gameInformation->handicap != 0)   properties["HA"].push_back( QString("%1").arg(n->gameInformation->handicap) );
        if (!n->gameInformation->rule.isEmpty()) properties["RU"].push_back( n->gameInformation->rule );

        if (!n->gameInformation->whitePlayer.isEmpty()) properties["PW"].push_back( n->gameInformation->whitePlayer );
        if (!n->gameInformation->whiteRank.isEmpty())   properties["WR"].push_back( n->gameInformation->whiteRank );
        if (!n->gameInformation->whiteTeam.isEmpty())   properties["WT"].push_back( n->gameInformation->whiteTeam );
        if (!n->gameInformation->blackPlayer.isEmpty()) properties["PB"].push_back( n->gameInformation->blackPlayer );
        if (!n->gameInformation->blackRank.isEmpty())   properties["BR"].push_back( n->gameInformation->blackRank );
        if (!n->gameInformation->blackTeam.isEmpty())   properties["BT"].push_back( n->gameInformation->blackTeam );
        if (!n->gameInformation->result.isEmpty())      properties["RE"].push_back( n->gameInformation->result );
        if (!n->gameInformation->time.isEmpty())        properties["TM"].push_back( n->gameInformation->time );
        if (!n->gameInformation->overTime.isEmpty())    properties["OT"].push_back( n->gameInformation->overTime );

        if (!n->gameInformation->date.isEmpty())        properties["DT"].push_back( n->gameInformation->date );
        if (!n->gameInformation->gameName.isEmpty())    properties["GN"].push_back( n->gameInformation->gameName );
        if (!n->gameInformation->round.isEmpty())       properties["RO"].push_back( n->gameInformation->round );
        if (!n->gameInformation->place.isEmpty())       properties["PC"].push_back( n->gameInformation->place );
        if (!n->gameInformation->event.isEmpty())       properties["EV"].push_back( n->gameInformation->event );

        if (!n->gameInformation->gameComment.isEmpty()) properties["GC"].push_back( n->gameInformation->gameComment );
        if (!n->gameInformation->annotation.isEmpty())  properties["AN"].push_back( n->gameInformation->annotation );
        if (!n->gameInformation->copyright.isEmpty())   properties["CP"].push_back( n->gameInformation->copyright );
        if (!n->gameInformation->opening.isEmpty())     properties["ON"].push_back( n->gameInformation->opening );
        if (!n->gameInformation->source.isEmpty())      properties["SO"].push_back( n->gameInformation->source );
        if (!n->gameInformation->user.isEmpty())        properties["US"].push_back( n->gameInformation->user );
    }

    if (n->isStone()){
        QString str;
        if (!n->isPass())
            str = positionToString(n->position.x, n->position.y);
        PropertyType::key_type key = n->isBlack() ? "B" : "W";
        properties[key].push_back(str);

        if (n->moveNumber > 0)
            properties["MN"].push_back( QString("%1").arg(n->moveNumber) );
    }

    if (!n->name.isEmpty())
        properties["N"].push_back(n->name);
    if (!n->comment.isEmpty())
        properties["C"].push_back(n->comment);

/*
    if (n->nextColor == Go::black)
        properties["PL"].push_back("B");
    else if (n->nextColor == Go::white)
        properties["PL"].push_back("W");
*/

    // marker
    set(n->marks);
    set(n->blackTerritories);
    set(n->whiteTerritories);
    set(n->emptyStones);
    set(n->blackStones);
    set(n->whiteStones);
    set(n->dims);

    if (n->moveAnnotation == Go::Node::goodMove)
        properties["TE"].push_back("1");
    if (n->moveAnnotation == Go::Node::veryGoodMove)
        properties["TE"].push_back("2");
    else if (n->moveAnnotation == Go::Node::badMove)
        properties["BM"].push_back("1");
    else if (n->moveAnnotation == Go::Node::veryBadMove)
        properties["BM"].push_back("2");
    else if (n->moveAnnotation == Go::Node::doubtfulMove)
        properties["DO"].push_back("");
    else if (n->moveAnnotation == Go::Node::interestingMove)
        properties["IT"].push_back("");

    if (n->nodeAnnotation == Go::Node::even)
        properties["DM"].push_back("1");
    else if (n->nodeAnnotation == Go::Node::goodForBlack)
        properties["GB"].push_back("1");
    else if (n->nodeAnnotation == Go::Node::veryGoodForBlack)
        properties["GB"].push_back("2");
    else if (n->nodeAnnotation == Go::Node::goodForWhite)
        properties["GW"].push_back("1");
    else if (n->nodeAnnotation == Go::Node::veryGoodForWhite)
        properties["GW"].push_back("2");
    else if (n->nodeAnnotation == Go::Node::unclear)
        properties["UC"].push_back("1");

    if (n->annotation == Go::Node::hotspot)
        properties["HO"].push_back("1");

    return false;
}

bool Sgf::Node::set(const Go::MarkList& markList){
    foreach (const Go::Mark& mark, markList){
        switch (mark.type){
            case Go::Mark::cross:
                properties["MA"].push_back( positionToString(mark.position) );
                break;
            case Go::Mark::circle:
                properties["CR"].push_back( positionToString(mark.position) );
                break;
            case Go::Mark::square:
                properties["SQ"].push_back( positionToString(mark.position) );
                break;
            case Go::Mark::triangle:
                properties["TR"].push_back( positionToString(mark.position) );
                break;
            case Go::Mark::select:
                properties["SL"].push_back( positionToString(mark.position) );
                break;
            case Go::Mark::character:
                properties["LB"].push_back( positionToString(mark.position, &mark.text) );
                break;
            case Go::Mark::blackTerritory:
                properties["TB"].push_back( positionToString(mark.position) );
                break;
            case Go::Mark::whiteTerritory:
                properties["TW"].push_back( positionToString(mark.position) );
                break;
            case Go::Mark::dim:
                properties["DD"].push_back( positionToString(mark.position) );
                break;
        };
    }

    return true;
}

bool Sgf::Node::set(const Go::StoneList& stoneList){
    foreach (const Go::Stone& stone, stoneList){
        if (stone.isBlack())
            properties["AB"].push_back( positionToString(stone.position) );
        else if (stone.isWhite())
            properties["AW"].push_back( positionToString(stone.position) );
        else if (stone.isEmpty())
            properties["AE"].push_back( positionToString(stone.position) );
    }
    return true;
}

QString Sgf::Node::toString() const{
    QString str(";");

    PropertyType::const_iterator iter = properties.begin();
    while (iter != properties.end()){
        str += iter.key();
        foreach(const QString& v, iter.value()){
            str += "[";
            str += v;
            str += "]";
        }

        ++iter;
    }

    return str;
}




#if 0
QString Sgf::Node::toString() const{
    static const QString keys[] = {
        // game information
        "GM", "FF", "CA", "AP", "SZ", "KM", "HA", "RU",
        "PW", "WR", "WT", "PB", "BR", "BT", "RE", "TM", "OT",
        "DT", "GN", "RO", "PC", "EV",
        "GC", "ON", "AN", "CP", "SO", "US",

        // stone
        "B", "BL", "OB", "W", "WL","OW", "MN", "PL",

        // marker
        "AE", "AB", "AW", "MA", "CR", "SQ", "TR", "SL", "LB", "TB", "TW", "DD",

        // move annotation
        "TE", "BM", "DO", "IT",

        // node annotation
        "C", "DM", "GB", "GW", "UC", "HO", "N",

        // unsupported property
        // "ST", "KO", "V", "AR", "LN", "FG", "PM", "VW"
    };
    static const int N = sizeof(keys) / sizeof(keys[0]);

    QString str;
    str.reserve(100);
    str.push_back(';');

    QString str2;
    str2.reserve(100);

    for (int i=0; i<N; ++i){
        propertyType::const_iterator iter = property.find(keys[i]);
        if (iter == property.end())
            continue;

        const propertyType::key_type& key = iter.key();
        const propertyType::mapped_type& values = iter.value();
        str2.push_back(key);

        propertyType::mapped_type::const_iterator iter2 = values.begin();
        while (iter2 != values.end()){
            str2.push_back('[');
            QString tmp(*iter2);
            tmp.replace('\\', "\\\\");
            tmp.replace(']', "\\]");
            str2.push_back(tmp);
            str2.push_back(']');

            if (str2.length() > SGF_LINEWIDTH){
                str.append(str2);
                str.push_back('\n');
                str2.clear();
            }
            ++iter2;
        }
    }

    str.append(str2);

    return str;
}

bool Sgf::Node::setProperty(const QString& key, const QStringList& values){
    property[key] += values;
    return true;
}

bool Sgf::Node::get(Go::NodePtr n) const{
    propertyType::const_iterator iter = property.begin();
    while (iter != property.end()){
        get(n, iter.key(), iter.value());
        ++iter;
    }

    return true;
}

bool Sgf::Node::get(Go::NodePtr n, const QString& key, const QStringList& values) const{
    Go::informationNode* infoNode = dynamic_cast<Go::informationNode*>(n.get());

    // game information
    if (infoNode && key == "PW")
        infoNode->whitePlayer = values[0];
    else if (infoNode && key == "WR")
        infoNode->whiteRank = values[0];
    else if (infoNode && key == "WT")
        infoNode->whiteTeam = values[0];
    if (infoNode && key == "PB")
        infoNode->blackPlayer = values[0];
    else if (infoNode && key == "BR")
        infoNode->blackRank = values[0];
    else if (infoNode && key == "BT")
        infoNode->blackTeam = values[0];
    else if (infoNode && key == "RE")
        infoNode->result = values[0];
    else if (infoNode && key == "SZ"){
        int p = values[0].indexOf(":");
        if (p == -1)
            infoNode->xsize = infoNode->ysize = values[0].toInt();
        else{
            infoNode->xsize = values[0].left(p).toInt();
            infoNode->ysize = values[0].mid(p+1).toInt();
        }
    }
    else if (infoNode && (key == "KM" || key == "KO")) // KM is correct, KO is used by cyber oro.
        infoNode->komi = values[0].toDouble();
    else if (infoNode && key == "HA")
        infoNode->handicap = values[0].toInt();
    else if (infoNode && key == "TM")
        infoNode->time = values[0];
    else if (infoNode && key == "OT")
        infoNode->overTime = values[0];
    else if (infoNode && (key == "DT" || key == "RD"))  // DT is correct, RD is cyber oro
        infoNode->date = values[0];
    else if (infoNode && (key == "GN" || key == "TE"))  // GN is correct, TE is cyber oro
        infoNode->gameName = values[0];
    else if (infoNode && key == "RO")
        infoNode->round = values[0];
    else if (infoNode && key == "PC")
        infoNode->place = values[0];
    else if (infoNode && key == "EV")
        infoNode->event = values[0];
    else if (infoNode && (key == "GC" || key == "TC")) // GC is correct, TC is cyber oro.
        infoNode->gameComment = values[0];
    else if (infoNode && key == "AN")
        infoNode->annotation = values[0];
    else if (infoNode && key == "CP")
        infoNode->copyright = values[0];
    else if (infoNode && key == "ON")
        infoNode->opening = values[0];
    else if (infoNode && key == "RU")
        infoNode->rule = values[0];
    else if (infoNode && key == "SO")
        infoNode->source = values[0];
    else if (infoNode && key == "US")
        infoNode->user = values[0];

    // comment
    else if (key == "C")
        n->comment = values.join("\n");
    else if (key == "N" || key == "RN")  // N is correct, RN is cyber oro version?
        n->name = values[0];
    else if (key == "MN")
        n->moveNumber = values[0].toInt();

    // move
    else if (key == "B" || key == "W"){
        n->setColor( key == "B" ? Go::black : Go::white);
        int x, y;
        if (pointToInt(values[0], x, y)){
            n->setX(x);
            n->setY(y);
        }
    }
    else if (key == "PL")
        n->nextColor = values[0] == "B" ? Go::black : Go::white;

    // mark
    else if (key == "MA" || key == "M")
        addMark(n->marks, values, mark::eCross);
    else if (key == "TR")
        addMark(n->marks, values, mark::eTriangle);
    else if (key == "CR")
        addMark(n->marks, values, mark::eCircle);
    else if (key == "SQ")
        addMark(n->marks, values, mark::eSquare);
    else if (key == "SL")
        addMark(n->marks, values, mark::eSelect);
    else if (key == "LB")
        addMark(n->marks, values);
    else if (key == "TB")
        addMark(n->blackTerritories, values, mark::eBlackTerritory);
    else if (key == "TW")
        addMark(n->whiteTerritories, values, mark::eWhiteTerritory);
    else if (key == "DD")
        addMark(n->dims, values, mark::eDim);
    else if (key == "AB")
        addStone(n->blackStones, key, values);
    else if (key == "AW")
        addStone(n->whiteStones, key, values);
    else if (key == "AE")
        addStone(n->emptyStones, key, values);

    // move annotation
    else if (key == "TE")
        n->moveAnnotation = values[0].toInt() > 1 ? Go::Node::eVeryGoodMove : Go::Node::eGoodMove;
    else if (key == "BM")
        n->moveAnnotation = values[0].toInt() > 1 ? Go::Node::eVeryBadMove : Go::Node::eBadMove;
    else if (key == "DO")
        n->moveAnnotation = Go::Node::eDoubtfulMove;
    else if (key == "IT")
        n->moveAnnotation = Go::Node::eInterestingMove;

    // node annotation
    else if (key == "DM")
        n->nodeAnnotation = Go::Node::eEven;
    else if (key == "GB")
        n->nodeAnnotation = values[0].toInt() > 1 ? Go::Node::eVeryGoodForBlack : Go::Node::eGoodForBlack;
    else if (key == "GW")
        n->nodeAnnotation = values[0].toInt() > 1 ? Go::Node::eVeryGoodForWhite : Go::Node::eGoodForWhite;
    else if (key == "UC")
        n->nodeAnnotation = Go::Node::eUnclear;

    // annotation
    else if (key == "HO")
        n->annotation = Go::Node::eHotspot;

    return true;
}

bool Sgf::Node::set(const Go::NodePtr n){
    const Go::informationNode* infoNode = dynamic_cast<const Go::informationNode*>(n.get());
    if (infoNode){
        property["GM"].push_back("1");
        property["FF"].push_back("4");
        property["AP"].push_back(APPNAME ":" VERSION);
        if (infoNode->xsize == infoNode->ysize)
            property["SZ"].push_back( QString("%1").arg(infoNode->xsize) );
        else
            property["SZ"].push_back( QString("%1:%2").arg(infoNode->xsize).arg(infoNode->ysize) );
        property["KM"].push_back( QString("%1").arg(infoNode->komi) );

        if (infoNode->handicap != 0)   property["HA"].push_back( QString("%1").arg(infoNode->handicap) );
        if (!infoNode->rule.isEmpty()) property["RU"].push_back( infoNode->rule );

        if (!infoNode->whitePlayer.isEmpty()) property["PW"].push_back( infoNode->whitePlayer );
        if (!infoNode->whiteRank.isEmpty())   property["WR"].push_back( infoNode->whiteRank );
        if (!infoNode->whiteTeam.isEmpty())   property["WT"].push_back( infoNode->whiteTeam );
        if (!infoNode->blackPlayer.isEmpty()) property["PB"].push_back( infoNode->blackPlayer );
        if (!infoNode->blackRank.isEmpty())   property["BR"].push_back( infoNode->blackRank );
        if (!infoNode->blackTeam.isEmpty())   property["BT"].push_back( infoNode->blackTeam );
        if (!infoNode->result.isEmpty())      property["RE"].push_back( infoNode->result );
        if (!infoNode->time.isEmpty())        property["TM"].push_back( infoNode->time );
        if (!infoNode->overTime.isEmpty())    property["OT"].push_back( infoNode->overTime );

        if (!infoNode->date.isEmpty())        property["DT"].push_back( infoNode->date );
        if (!infoNode->gameName.isEmpty())    property["GN"].push_back( infoNode->gameName );
        if (!infoNode->round.isEmpty())       property["RO"].push_back( infoNode->round );
        if (!infoNode->place.isEmpty())       property["PC"].push_back( infoNode->place );
        if (!infoNode->event.isEmpty())       property["EV"].push_back( infoNode->event );

        if (!infoNode->gameComment.isEmpty()) property["GC"].push_back( infoNode->gameComment );
        if (!infoNode->annotation.isEmpty())  property["AN"].push_back( infoNode->annotation );
        if (!infoNode->copyright.isEmpty())   property["CP"].push_back( infoNode->copyright );
        if (!infoNode->opening.isEmpty())     property["ON"].push_back( infoNode->opening );
        if (!infoNode->source.isEmpty())      property["SO"].push_back( infoNode->source );
        if (!infoNode->user.isEmpty())        property["US"].push_back( infoNode->user );
    }
    else if (n->isStone()){
        QString str;
        if (!n->isPass())
            str = pointToString(n->position.x, n->position.y);
        propertyType::key_type key = n->isBlack() ? "B" : "W";
        property[key].push_back(str);

        if (n->moveNumber > 0)
            property["MN"].push_back( QString("%1").arg(n->moveNumber) );
    }

    if (!n->name.isEmpty())
        property["N"].push_back(n->name);
    if (!n->comment.isEmpty())
        property["C"].push_back(n->comment);

/*
    if (n->nextColor == Go::black)
        property["PL"].push_back("B");
    else if (n->nextColor == Go::white)
        property["PL"].push_back("W");
*/

    // marker
    set(n->marks);
    set(n->blackTerritories);
    set(n->whiteTerritories);
    set(n->emptyStones);
    set(n->blackStones);
    set(n->whiteStones);
    set(n->dims);

    if (n->moveAnnotation == Go::Node::eGoodMove)
        property["TE"].push_back("1");
    if (n->moveAnnotation == Go::Node::eVeryGoodMove)
        property["TE"].push_back("2");
    else if (n->moveAnnotation == Go::Node::eBadMove)
        property["BM"].push_back("1");
    else if (n->moveAnnotation == Go::Node::eVeryBadMove)
        property["BM"].push_back("2");
    else if (n->moveAnnotation == Go::Node::eDoubtfulMove)
        property["DO"].push_back("");
    else if (n->moveAnnotation == Go::Node::eInterestingMove)
        property["IT"].push_back("");

    if (n->nodeAnnotation == Go::Node::eEven)
        property["DM"].push_back("1");
    else if (n->nodeAnnotation == Go::Node::eGoodForBlack)
        property["GB"].push_back("1");
    else if (n->nodeAnnotation == Go::Node::eVeryGoodForBlack)
        property["GB"].push_back("2");
    else if (n->nodeAnnotation == Go::Node::eGoodForWhite)
        property["GW"].push_back("1");
    else if (n->nodeAnnotation == Go::Node::eVeryGoodForWhite)
        property["GW"].push_back("2");
    else if (n->nodeAnnotation == Go::Node::eUnclear)
        property["UC"].push_back("1");

    if (n->annotation == Go::Node::eHotspot)
        property["HO"].push_back("1");

    return false;
}

bool Sgf::Node::set(const Go::markList& markList){
    markList::const_iterator iter = markList.begin();
    while (iter != markList.end()){
        switch (iter->t){
            case Go::mark::eCross:
                property["MA"].push_back( pointToString(iter->p) );
                break;
            case Go::mark::eCircle:
                property["CR"].push_back( pointToString(iter->p) );
                break;
            case Go::mark::eSquare:
                property["SQ"].push_back( pointToString(iter->p) );
                break;
            case Go::mark::eTriangle:
                property["TR"].push_back( pointToString(iter->p) );
                break;
            case Go::mark::eSelect:
                property["SL"].push_back( pointToString(iter->p) );
                break;
            case Go::mark::eCharacter:
                property["LB"].push_back( pointToString(iter->p, &iter->s) );
                break;
            case Go::mark::eBlackTerritory:
                property["TB"].push_back( pointToString(iter->p) );
                break;
            case Go::mark::eWhiteTerritory:
                property["TW"].push_back( pointToString(iter->p) );
                break;
            case Go::mark::eDim:
                property["DD"].push_back( pointToString(iter->p) );
                break;
        };
        ++iter;
    }

    return true;
}

bool Sgf::Node::set(const Go::stoneList& stoneList){
    Go::stoneList::const_iterator iter = stoneList.begin();
    while (iter != stoneList.end()){
        if (iter->isBlack())
            property["AB"].push_back( pointToString(iter->p) );
        else if (iter->isWhite())
            property["AW"].push_back( pointToString(iter->p) );
        else if (iter->isEmpty())
            property["AE"].push_back( pointToString(iter->p) );
        ++iter;
    }

    return true;
}

void Sgf::Node::addMark(Go::markList& markList, const QStringList& values, const char* str) const{
    QStringList::const_iterator iter = values.begin();
    while (iter != values.end()){
        int x, y;
        QString str2;
        if (pointToInt(*iter, x, y, &str2))
            markList.push_back( Go::mark(x, y, str2.isEmpty() ? str : str2) );
        ++iter;
    }
}

void Sgf::Node::addMark(Go::markList& markList, const QStringList& values, mark::eType type) const{
    QStringList::const_iterator iter = values.begin();
    while (iter != values.end()){
        QList<int> x, y;
        if (pointToIntList(*iter, x, y))
            for(int i=0; i<x.size(); ++i)
                markList.push_back( Go::mark(x[i], y[i], type) );
        ++iter;
    }
}

void Sgf::Node::addStone(Go::stoneList& stoneList, const QString& key, const QStringList& values) const{
    Go::color c = key == "AB" ? Go::black : key == "AW" ? Go::white : Go::empty;

    QStringList::const_iterator iter = values.begin();
    while (iter != values.end()){
        QList<int> x, y;
        if (pointToIntList(*iter, x, y))
            for (int i=0; i<x.size(); ++i)
                stoneList.push_back( Go::stone(x[i], y[i], c) );
        ++iter;
    }
}

bool Sgf::readStream(QString::iterator& first, QString::iterator last){
    while (first != last){
        wchar_t c = first->unicode();
        ++first;
        if (c == L'('){
            NodePtr root(new node());
            if (readBranch(first, last, root) == false)
                continue;

            if (root->getChildNodes().empty())
                continue;

            NodePtr info = root->getChildNodes().front();
            if (root->getChildNodes().size() == 1 && info->getProperty().size() == 0)
                continue;

            root->setNodeType(eRoot);
            rootList.push_back(root);
            info->setNodeType(eGameInformation);
        }
    }

    return true;
}

bool Sgf::saveStream(QTextStream& stream){
    foreach (NodePtr root, rootList){
        if (codec){
            foreach (NodePtr info, root->getChildNodes()){
                info->setProperty("CA", QStringList(codec->name()));
            }
        }

        QString s;
        writeNode(stream, s, root);
        if (!s.isEmpty())
            stream << s;
        stream << '\n';
    }

    return true;
}

QTextCodec* Sgf::getCodec(const QByteArray& a) const{
    int s = a.indexOf("CA[");
    if (s == -1)
        return NULL;
    s += 3;

    int e = a.indexOf(']', s);
    if (e == -1)
        return NULL;

    QString name = a.mid(s, e-s);
    if (name.compare("windows-31j", Qt::CaseInsensitive) == 0)
        return QTextCodec::codecForName("Shift_JIS");
    else
        return QTextCodec::codecForName(name.toAscii());
}

bool Sgf::readBranch(QString::iterator& first, QString::iterator last, NodePtr n){
    n->setNodeType(Sgf::eBranch);
    NodePtr branchParent;
    nodeList::iterator branchNode = n->getChildNodes().end();
    int pt = 1;

    while (first != last){
        wchar_t c = first->unicode();
        ++first;

        if (c == L')')
            return true;
        // only ';' or '(' appears hear, but ';' does not exist in korean sgf.
        else if (c != L';' && c != L'(' && iswspace(c))
            continue;

        if (c == L'('){
            NodePtr newNode( new node );
            nodeList::iterator iter = n->getChildNodes().insert(n->getChildNodes().end(), newNode);

            readBranch(first, last, newNode);

            if (branchNode == n->getChildNodes().end()){
                branchNode = iter;
                branchParent = n;
            }
        }
        else{
            if (c != ';')
                --first;

            NodePtr newNode( new node );

            if (pt == 1){
                if ( branchNode == n->getChildNodes().end() )
                    n->getChildNodes().push_back(newNode);
                else{
                    // cyber oro? original format.
                    newNode->setNodeType(Sgf::eBranch);
                    branchParent->getChildNodes().insert(branchNode, newNode);
                    n = newNode;
                    branchNode = n->getChildNodes().end();
                    branchParent.reset();
                    newNode.reset( new node );
                    n->getChildNodes().push_back(newNode);
                }
            }
            else
                --pt;
            readNode(first, last, newNode, pt);
        }
    }

    return true;
}

bool Sgf::readNode(QString::iterator& first, QString::iterator last, NodePtr& n, int& pt){
    while (first != last){
        wchar_t c = first->unicode();
        if (iswspace(c)){
            ++first;
            continue;
        }
        else if (c == L';' || c == L'(' || c == L')'){
            break;
        }

        QString key;
        key.reserve(5);
        if (readNodeKey(first, last, key) == false)
            return false;

        QStringList values;
        if (readNodeValues(first, last, values) == false)
            return false;

        n->setProperty(key, values);

        // for cyber oro?
        if (key == "PT")
            pt = values[0].toInt();
    }

    return true;
}

bool Sgf::readNodeKey(QString::iterator& first, QString::iterator last, QString& key){
    while (first != last){
        wchar_t c = first->unicode();
        if (c == L'[')
            return true;
        else if (c == L';' || c == L'(' || c == L')')
            return false;
        else{
            key.push_back(c);
            ++first;
        }
    }

    return false;
}

bool Sgf::readNodeValues(QString::iterator& first, QString::iterator last, QStringList& values){
    while (first != last){
        wchar_t c = first->unicode();
        if (c == L'['){
            QString v;
            v.reserve(100);
            if (readNodeValue(++first, last, v))
                values.push_back(v);
        }
        else if (iswspace(c))
            ++first;
        else
            return true;
    }

    return false;
}

bool Sgf::readNodeValue(QString::iterator& first, QString::iterator last, QString& value){
    while (first != last){
        wchar_t c = first->unicode();
        ++first;

        if (c == ']')
            return true;

        if (c == '\\'){
            c = first->unicode();
            ++first;
        }
        value.push_back(c);
    }

    return false;
}

bool Sgf::writeNode(QTextStream& stream, QString& s, const NodePtr& n){
    if (n->getNodeType() == eBranch || n->getNodeType() == eRoot){
        s.push_back('(');
        foreach(const NodePtr& childNode, n->getChildNodes()){
            writeNode(stream, s, childNode);
        }
        s.push_back(')');
    }
    else{
        s.append(n->toString());
        if (s.size() > SGF_LINEWIDTH){
            stream << s;
            if (s.right(1) != "\n")
                stream << '\n';
            s.clear();
        }
        foreach(const NodePtr& childNode, n->getChildNodes()){
            writeNode(stream, s, childNode);
        }
    }

    return true;
}

bool Sgf::get(Go::data& data) const{
    data.clear();
    data.rootList.clear();
    foreach(const NodePtr& root, rootList){
        Go::informationPtr info(new Go::informationNode(Go::NodePtr()));
        Go::NodePtr node(info);
        get(root, node);
        data.rootList.push_back(info);
        data.root = data.rootList.front();
    }

    if (data.rootList.empty())
        data.rootList.push_back(data.root);

    return true;
}

Go::NodePtr Sgf::get(const NodePtr& sgfNode, Go::NodePtr& outNode) const{
    // process node
    Go::NodePtr newNode;
    switch(sgfNode->getNodeType()){
        case eBlack:
            newNode = Go::createBlackNode(outNode);
            break;

        case eWhite:
            newNode = Go::createWhiteNode(outNode);
            break;

        case eBranch:
            break;

        case eGameInformation:
            sgfNode->get(outNode);
            break;

        case eRoot:
            break;

        default:
            newNode.reset( new Go::node(outNode) );
            break;
    }

    if (newNode){
        outNode->childNodes.push_back(newNode);
        sgfNode->get(newNode);
    }
    else
        newNode = outNode;

    // process children
    Go::NodePtr childNode = newNode;
    foreach(const NodePtr& node, sgfNode->getChildNodes()){
        childNode = get(node, childNode);
    }

    return newNode;
}

bool Sgf::set(const Go::data& data){
    rootList.clear();

    foreach(const Go::informationPtr& info, data.rootList){
        NodePtr root(new node());
        root->setNodeType(eRoot);
        rootList.push_back(root);

        NodePtr gameInfo(new node());
        gameInfo->set(info);
        gameInfo->setNodeType(Sgf::eGameInformation);
        root->getChildNodes().push_back(gameInfo);

        if (set(gameInfo, info) == false)
            return false;
    }
    return true;
}

bool Sgf::set(const Go::informationPtr& info){
    rootList.clear();

    NodePtr root(new node());
    root->setNodeType(eRoot);
    rootList.push_back(root);

    NodePtr gameInfo(new node());
    gameInfo->set(info);
    gameInfo->setNodeType(Sgf::eGameInformation);
    root->getChildNodes().push_back(gameInfo);

    if (set(gameInfo, info) == false)
        return false;
    return true;
}

bool Sgf::set(NodePtr& sgfNode, const Go::NodePtr& goNode){
    if (goNode->childNodes.size() > 1){
        foreach(const Go::NodePtr& inNode, goNode->childNodes){
            NodePtr branchNode(new node);
            branchNode->setNodeType(eBranch);
            sgfNode->getChildNodes().push_back(branchNode);

            NodePtr newNode(new node);
            newNode->set( inNode );
            branchNode->getChildNodes().push_back(newNode);
            set(newNode, inNode);
        }
    }
    else if (goNode->childNodes.size() == 1){
        NodePtr newNode(new node);
        newNode->set( goNode->childNodes.front() );
        sgfNode->getChildNodes().push_back(newNode);
        set(sgfNode, goNode->childNodes.front());
    }

    return true;
}

bool Sgf::pointToInt(const QString& pos, int& x, int& y, QString* str){
    if (pos.size() < 2)
        return false;
    x = pos[0].unicode() - (pos[0].isLower() ? L'a' : L'A' - 27);
    y = pos[1].unicode() - (pos[1].isLower() ? L'a' : L'A' - 27);

    if (str && pos.size() > 3 && pos[2] == ':')
        *str = pos.mid(3);

    return true;
}

bool Sgf::pointToIntList(const QString& pos, QList<int>& xList, QList<int>& yList){
    QRegExp exp("(..):?(..)?");
    exp.indexIn(pos);
    QStringList list = exp.capturedTexts();

    int x1, y1;
    if (pointToInt(list[1], x1, y1) == false)
        return false;

    int x2, y2;
    if (list[2].isEmpty() || pointToInt(list[2], x2, y2) == false){
        xList.push_back(x1);
        yList.push_back(y1);
        return true;
    }

    for (int y=y1; y<=y2; ++y){
        for (int x=x1; x<=x2; ++x){
            xList.push_back( x );
            yList.push_back( y );
        }
    }

    return true;
}

QString Sgf::pointToString(int x, int y, const QString* s){
    QString buf;
    if (s)
        buf.sprintf("%c%c:%s", x + (x < 27 ? 'a' : 'A' - 27), y + (y < 27 ? 'a' : 'A' - 27), (const char*)s->toAscii());
    else
        buf.sprintf("%c%c", x + (x < 27 ? 'a' : 'A' - 27), y + (y < 27 ? 'a' : 'A' - 27));
    return buf;
}

QString Sgf::pointToString(const Go::point& p, const QString* s){
    return pointToString(p.x, p.y, s);
}
#endif


};
