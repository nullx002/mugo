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
#include "appdef.h"
#include "sgf.h"

namespace go{



QString sgf::node::toString() const{
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

bool sgf::node::setProperty(const QString& key, const QStringList& values){
    property[key] += values;
    return true;
}

bool sgf::node::get(go::nodePtr n) const{
    propertyType::const_iterator iter = property.begin();
    while (iter != property.end()){
        get(n, iter.key(), iter.value());
        ++iter;
    }

    return true;
}

bool sgf::node::get(go::nodePtr n, const QString& key, const QStringList& values) const{
    go::informationNode* infoNode = dynamic_cast<go::informationNode*>(n.get());

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
        n->setColor( key == "B" ? go::black : go::white);
        int x, y;
        if (pointToInt(values[0], x, y)){
            n->setX(x);
            n->setY(y);
        }
    }
    else if (key == "PL")
        n->nextColor = values[0] == "B" ? go::black : go::white;

    // mark
    else if (key == "MA" || key == "M")
        addMark(n->crosses, values, mark::eCross);
    else if (key == "TR")
        addMark(n->triangles, values, mark::eTriangle);
    else if (key == "CR")
        addMark(n->circles, values, mark::eCircle);
    else if (key == "SQ")
        addMark(n->squares, values, mark::eSquare);
    else if (key == "SL")
        addMark(n->selects, values, mark::eSelect);
    else if (key == "LB")
        addMark(n->characters, values);
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
        n->moveAnnotation = values[0].toInt() > 1 ? go::node::eVeryGoodMove : go::node::eGoodMove;
    else if (key == "BM")
        n->moveAnnotation = values[0].toInt() > 1 ? go::node::eVeryBadMove : go::node::eBadMove;
    else if (key == "DO")
        n->moveAnnotation = go::node::eDoubtfulMove;
    else if (key == "IT")
        n->moveAnnotation = go::node::eInterestingMove;

    // node annotation
    else if (key == "DM")
        n->nodeAnnotation = go::node::eEven;
    else if (key == "GB")
        n->nodeAnnotation = values[0].toInt() > 1 ? go::node::eVeryGoodForBlack : go::node::eGoodForBlack;
    else if (key == "GW")
        n->nodeAnnotation = values[0].toInt() > 1 ? go::node::eVeryGoodForWhite : go::node::eGoodForWhite;
    else if (key == "UC")
        n->nodeAnnotation = go::node::eUnclear;

    // annotation
    else if (key == "HO")
        n->annotation = go::node::eHotspot;

    return true;
}

bool sgf::node::set(const go::nodePtr n){
    const go::informationNode* infoNode = dynamic_cast<const go::informationNode*>(n.get());
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
    if (n->nextColor == go::black)
        property["PL"].push_back("B");
    else if (n->nextColor == go::white)
        property["PL"].push_back("W");
*/

    // marker
    set(n->crosses);
    set(n->circles);
    set(n->triangles);
    set(n->squares);
    set(n->selects);
    set(n->characters);
    set(n->blackTerritories);
    set(n->whiteTerritories);
    set(n->emptyStones);
    set(n->blackStones);
    set(n->whiteStones);
    set(n->dims);

    if (n->moveAnnotation == go::node::eGoodMove)
        property["TE"].push_back("1");
    if (n->moveAnnotation == go::node::eVeryGoodMove)
        property["TE"].push_back("2");
    else if (n->moveAnnotation == go::node::eBadMove)
        property["BM"].push_back("1");
    else if (n->moveAnnotation == go::node::eVeryBadMove)
        property["BM"].push_back("2");
    else if (n->moveAnnotation == go::node::eDoubtfulMove)
        property["DO"].push_back("");
    else if (n->moveAnnotation == go::node::eInterestingMove)
        property["IT"].push_back("");

    if (n->nodeAnnotation == go::node::eEven)
        property["DM"].push_back("1");
    else if (n->nodeAnnotation == go::node::eGoodForBlack)
        property["GB"].push_back("1");
    else if (n->nodeAnnotation == go::node::eVeryGoodForBlack)
        property["GB"].push_back("2");
    else if (n->nodeAnnotation == go::node::eGoodForWhite)
        property["GW"].push_back("1");
    else if (n->nodeAnnotation == go::node::eVeryGoodForWhite)
        property["GW"].push_back("2");
    else if (n->nodeAnnotation == go::node::eUnclear)
        property["UC"].push_back("1");

    if (n->annotation == go::node::eHotspot)
        property["HO"].push_back("1");

    return false;
}

bool sgf::node::set(const go::markList& markList){
    markList::const_iterator iter = markList.begin();
    while (iter != markList.end()){
        switch (iter->t){
            case go::mark::eCross:
                property["MA"].push_back( pointToString(iter->p) );
                break;
            case go::mark::eCircle:
                property["CR"].push_back( pointToString(iter->p) );
                break;
            case go::mark::eSquare:
                property["SQ"].push_back( pointToString(iter->p) );
                break;
            case go::mark::eTriangle:
                property["TR"].push_back( pointToString(iter->p) );
                break;
            case go::mark::eSelect:
                property["SL"].push_back( pointToString(iter->p) );
                break;
            case go::mark::eCharacter:
                property["LB"].push_back( pointToString(iter->p, &iter->s) );
                break;
            case go::mark::eBlackTerritory:
                property["TB"].push_back( pointToString(iter->p) );
                break;
            case go::mark::eWhiteTerritory:
                property["TW"].push_back( pointToString(iter->p) );
                break;
            case go::mark::eDim:
                property["DD"].push_back( pointToString(iter->p) );
                break;
        };
        ++iter;
    }

    return true;
}

bool sgf::node::set(const go::stoneList& stoneList){
    go::stoneList::const_iterator iter = stoneList.begin();
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

void sgf::node::addMark(go::markList& markList, const QStringList& values, const char* str) const{
    QStringList::const_iterator iter = values.begin();
    while (iter != values.end()){
        int x, y;
        QString str2;
        if (pointToInt(*iter, x, y, &str2))
            markList.push_back( go::mark(x, y, str2.isEmpty() ? str : str2) );
        ++iter;
    }
}

void sgf::node::addMark(go::markList& markList, const QStringList& values, mark::eType type) const{
    QStringList::const_iterator iter = values.begin();
    while (iter != values.end()){
        QList<int> x, y;
        if (pointToIntList(*iter, x, y))
            for(int i=0; i<x.size(); ++i)
                markList.push_back( go::mark(x[i], y[i], type) );
        ++iter;
    }
}

void sgf::node::addStone(go::stoneList& stoneList, const QString& key, const QStringList& values) const{
    go::color c = key == "AB" ? go::black : key == "AW" ? go::white : go::empty;

    QStringList::const_iterator iter = values.begin();
    while (iter != values.end()){
        QList<int> x, y;
        if (pointToIntList(*iter, x, y))
            for (int i=0; i<x.size(); ++i)
                stoneList.push_back( go::stone(x[i], y[i], c) );
        ++iter;
    }
}

bool sgf::readStream(QString::iterator& first, QString::iterator last){
    while (first != last){
        wchar_t c = first->unicode();
        ++first;
        if (c == L'('){
            nodePtr root(new node());
            if (readBranch(first, last, root) == false)
                continue;

            if (root->getChildNodes().empty())
                continue;

            nodePtr info = root->getChildNodes().front();
            if (root->getChildNodes().size() == 1 && info->getProperty().size() == 0)
                continue;

            root->setNodeType(eRoot);
            rootList.push_back(root);
            info->setNodeType(eGameInformation);
        }
    }

    return true;
}

bool sgf::saveStream(QTextStream& stream){
    foreach (nodePtr root, rootList){
        if (codec){
            foreach (nodePtr info, root->getChildNodes()){
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

QTextCodec* sgf::getCodec(const QByteArray& a) const{
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

bool sgf::readBranch(QString::iterator& first, QString::iterator last, nodePtr n){
    n->setNodeType(sgf::eBranch);
    nodePtr branchParent;
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
            nodePtr newNode( new node );
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

            nodePtr newNode( new node );

            if (pt == 1){
                if ( branchNode == n->getChildNodes().end() )
                    n->getChildNodes().push_back(newNode);
                else{
                    // cyber oro? original format.
                    newNode->setNodeType(sgf::eBranch);
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

bool sgf::readNode(QString::iterator& first, QString::iterator last, nodePtr& n, int& pt){
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

bool sgf::readNodeKey(QString::iterator& first, QString::iterator last, QString& key){
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

bool sgf::readNodeValues(QString::iterator& first, QString::iterator last, QStringList& values){
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

bool sgf::readNodeValue(QString::iterator& first, QString::iterator last, QString& value){
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

bool sgf::writeNode(QTextStream& stream, QString& s, const nodePtr& n){
    if (n->getNodeType() == eBranch || n->getNodeType() == eRoot){
        s.push_back('(');
        foreach(const nodePtr& childNode, n->getChildNodes()){
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
        foreach(const nodePtr& childNode, n->getChildNodes()){
            writeNode(stream, s, childNode);
        }
    }

    return true;
}

bool sgf::get(go::data& data) const{
    data.clear();
    data.rootList.clear();
    foreach(const nodePtr& root, rootList){
        go::informationPtr info(new go::informationNode(go::nodePtr()));
        go::nodePtr node(info);
        get(root, node);
        data.rootList.push_back(info);
        data.root = data.rootList.front();
    }

    if (data.rootList.empty())
        data.rootList.push_back(data.root);

    return true;
}

go::nodePtr sgf::get(const nodePtr& sgfNode, go::nodePtr& outNode) const{
    // process node
    go::nodePtr newNode;
    switch(sgfNode->getNodeType()){
        case eBlack:
            newNode = go::createBlackNode(outNode);
            break;

        case eWhite:
            newNode = go::createWhiteNode(outNode);
            break;

        case eBranch:
            break;

        case eGameInformation:
            sgfNode->get(outNode);
            break;

        case eRoot:
            break;

        default:
            newNode.reset( new go::node(outNode) );
            break;
    }

    if (newNode){
        outNode->childNodes.push_back(newNode);
        sgfNode->get(newNode);
    }
    else
        newNode = outNode;

    // process children
    go::nodePtr childNode = newNode;
    foreach(const nodePtr& node, sgfNode->getChildNodes()){
        childNode = get(node, childNode);
    }

    return newNode;
}

bool sgf::set(const go::data& data){
    rootList.clear();

    foreach(const go::informationPtr& info, data.rootList){
        nodePtr root(new node());
        root->setNodeType(eRoot);
        rootList.push_back(root);

        nodePtr gameInfo(new node());
        gameInfo->set(info);
        gameInfo->setNodeType(sgf::eGameInformation);
        root->getChildNodes().push_back(gameInfo);

        if (set(gameInfo, info) == false)
            return false;
    }
    return true;
}

bool sgf::set(const go::informationPtr& info){
    rootList.clear();

    nodePtr root(new node());
    root->setNodeType(eRoot);
    rootList.push_back(root);

    nodePtr gameInfo(new node());
    gameInfo->set(info);
    gameInfo->setNodeType(sgf::eGameInformation);
    root->getChildNodes().push_back(gameInfo);

    if (set(gameInfo, info) == false)
        return false;
    return true;
}

bool sgf::set(nodePtr& sgfNode, const go::nodePtr& goNode){
    if (goNode->childNodes.size() > 1){
        foreach(const go::nodePtr& inNode, goNode->childNodes){
            nodePtr branchNode(new node);
            branchNode->setNodeType(eBranch);
            sgfNode->getChildNodes().push_back(branchNode);

            nodePtr newNode(new node);
            newNode->set( inNode );
            branchNode->getChildNodes().push_back(newNode);
            set(newNode, inNode);
        }
    }
    else if (goNode->childNodes.size() == 1){
        nodePtr newNode(new node);
        newNode->set( goNode->childNodes.front() );
        sgfNode->getChildNodes().push_back(newNode);
        set(sgfNode, goNode->childNodes.front());
    }

    return true;
}

bool sgf::pointToInt(const QString& pos, int& x, int& y, QString* str){
    if (pos.size() < 2)
        return false;
    x = pos[0].unicode() - (pos[0].isLower() ? L'a' : L'A' - 27);
    y = pos[1].unicode() - (pos[1].isLower() ? L'a' : L'A' - 27);

    if (str && pos.size() > 3 && pos[2] == ':')
        *str = pos.mid(3);

    return true;
}

bool sgf::pointToIntList(const QString& pos, QList<int>& xList, QList<int>& yList){
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

QString sgf::pointToString(int x, int y, const QString* s){
    QString buf;
    if (s)
        buf.sprintf("%c%c:%s", x + (x < 27 ? 'a' : 'A' - 27), y + (y < 27 ? 'a' : 'A' - 27), (const char*)s->toAscii());
    else
        buf.sprintf("%c%c", x + (x < 27 ? 'a' : 'A' - 27), y + (y < 27 ? 'a' : 'A' - 27));
    return buf;
}

QString sgf::pointToString(const go::point& p, const QString* s){
    return pointToString(p.x, p.y, s);
}


};
