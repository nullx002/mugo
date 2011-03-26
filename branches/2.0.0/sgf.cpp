/*
    mugo, sgf editor.
    Copyright (C) 2009-2011 nsase.

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
#include <QStringList>
#include <QTextCodec>
#include "mugoapp.h"
#include "sgf.h"


namespace Go{


/**
  Constructor
*/
Sgf::Sgf(Go::NodeList& gameList)
    : FileBase(gameList)
{
}

/**
  @return return codec if codec is understood. if return NULL, codec isn't understood.
*/
QTextCodec* Sgf::guessCodec(const QByteArray& ba){
    return NULL;
}

/**
  parse sgf
*/
bool Sgf::parse(const QString& str){
    return parse(str.begin(), str.end());
}

/**
  parse sgf
*/
bool Sgf::parse(QString::const_iterator first, QString::const_iterator last){
    while (first != last){
        if (*first == '('){
            NodePtr root(new Node);
            InformationPtr info;
            if (parseBranch(first, last, info, root) == false)
                return false;

            if (root->children().size() != 1)
                return false;

            NodePtr game(root->child(0));
            game->setParent();
            gameList.push_back(game);
        }
        else
            ++first;
    }
    return true;
}

/**
  read branch
*/
bool Sgf::parseBranch(QString::const_iterator& first, QString::const_iterator last, InformationPtr& info, NodePtr parent){
    if (*first != '(')
        return false;
    ++first;

    while (first != last){
        if (*first == ')'){
            ++first;
            return true;
        }
        else if (*first == '('){
            if (parseBranch(first, last, info, parent) == false)
                return false;
        }
//        else if (*first == ';'){  // root node of cyber oro does not start ';'
        else if (first->isSpace() == false){
            NodePtr node(new Node(parent));
            if (parseNode(first, last, info, node) == false)
                return false;
            parent->children().push_back(node);
            parent = node;
        }
        else
            ++first;
    }
    return false;
}

/**
  read node
*/
bool Sgf::parseNode(QString::const_iterator& first, QString::const_iterator last, InformationPtr& info, NodePtr& node){
    if (*first == ';')
        ++first;

    while (first != last){
        if (*first == ';' || *first == '(' || *first == ')')
            return true;

        QString key;
        if (parseNodeName(first, last, key) == false)
            return false;

        QStringList valueList;
        if (parseNodeValueList(first, last, valueList) == false)
            return false;

        addPropertyToNode(info, node, key, valueList);

        if (*first == ';' || *first == '(' || *first == ')' || *first == '[')
            return true;
    }

    return true;
}

/**
  read node name
*/
bool Sgf::parseNodeName(QString::const_iterator& first, QString::const_iterator last, QString& key){
    if (skipSpace(first, last) == false)
        return false;

    while (first != last){
        if (first->isSymbol() == ';' || *first == '(' || *first == ')' || *first == '[' || first->isSpace())
            return true;
        key.push_back(*first);
        ++first;
    }

    return true;
}

/**
  read node value list
*/
bool Sgf::parseNodeValueList(QString::const_iterator& first, QString::const_iterator last, QStringList& valueList){
    while (first != last){
        QString value;
        if (parseNodeValue(first, last, value) == false)
            return false;
        valueList.push_back(value);

        if (skipSpace(++first, last) == false)
            return false;

        if (*first != '[')
            return true;
    }
    return false;
}

/**
  read node value
*/
bool Sgf::parseNodeValue(QString::const_iterator& first, QString::const_iterator last, QString& value){
    if (skipSpace(first, last) == false)
        return false;

    if (*first != '[')
        return false;
    ++first;

    while (first != last){
        if (*first == ']')
            return true;
        else if (*first == '\\'){
            ++first;
            if (*first == '\r' || *first == '\n'){
                ++first;
                continue;
            }
        }
        value.push_back(*first);
        ++first;
    }
    return false;
}

/**
  skip space
*/
bool Sgf::skipSpace(QString::const_iterator& first, QString::const_iterator last){
    while (first != last){
        if (first->isSpace() == false)
            return true;
        ++first;
    }
    return false;
}

/**
  set sgf property to node
*/
bool Sgf::addPropertyToNode(InformationPtr& info, NodePtr& node, const QString& key, const QStringList& valueList){
/*
Root Properties             AP, CA, FF, GM, ST, SZ
Game Info Properties        AN, BR, BT, CP, DT, EV, GN, GC, ON, OT, PB, PC, PW, RE, RO, RU, SO, TM, US, WR, WT
Move Properties             B, KO, MN, W
Node Annotation Properties  C, DM, GB, GW, HO, N, UC, V
Move Annotation Properties  BM, DO, IT, TE
Setup Properties            AB, AE, AW, PL
Markup Properties           AR, CR, DD, LB, LN, MA, SL, SQ, TR
Timing Properties           BL, OB, OW, WL
Miscellaneous Properties    FG, PM, VW
*/
    if (valueList.empty())
        return false;

    // Root Properties: AP, CA, FF, GM, ST, SZ
    if (key == "AP")  // application name
        gameInformation(node)->setApplicationName(valueList[0]);
//    else if (key == "CA")  // character set
//    else if (key == "FF")  // file format
//    else if (key == "GM")  // game type, only support 1
    else if (key == "ST")  // how variations should be shown
        gameInformation(node)->setVariationStyle(valueList[0].toInt());
    else if (key == "SZ"){  // board size
        int xsize = 0, ysize = 0;
        parseNumber(valueList[0], xsize, ysize);
        if (xsize > 0 && ysize > 0)
            gameInformation(node)->setSize(xsize, ysize);
        else if (xsize > 0)
            gameInformation(node)->setSize(xsize, xsize);
        else
            gameInformation(node)->setSize(19, 19);
    }

    // Game Info Properties: AN, BR, BT, CP, DT, EV, GN, GC, ON, OT, PB, PC, PW, RE, RO, RU, SO, TM, US, WR, WT
    else if (key == "AN")  // name of the person, who made the annotation to the game
        gameInformation(node)->setAnnotation(valueList[0]);
    else if (key == "BR")  // black rank
        gameInformation(node)->setBlackRank(valueList[0]);
    else if (key == "BT")  // black team
        gameInformation(node)->setBlackTeam(valueList[0]);
    else if (key == "CP")  // copyright
        gameInformation(node)->setCopyright(valueList[0]);
    else if (key == "DT")  // date
        gameInformation(node)->setDate(valueList[0]);
    else if (key == "EV")  // event name
        gameInformation(node)->setEvent(valueList[0]);
    else if (key == "GN")  // game name
        gameInformation(node)->setGameName(valueList[0]);
    else if (key == "GC")  // extra information about the game
        gameInformation(node)->setGameComment(valueList[0]);
    else if (key == "ON")  // opening, fuseki
        gameInformation(node)->setOpening(valueList[0]);
    else if (key == "OT")  // overtime, byo-yomi
        gameInformation(node)->setOvertime(valueList[0]);
    else if (key == "PB")  // black player
        gameInformation(node)->setBlackPlayer(valueList[0]);
    else if (key == "PC")  // place where game was played
        gameInformation(node)->setPlace(valueList[0]);
    else if (key == "PW")  // white player
        gameInformation(node)->setWhitePlayer(valueList[0]);
    else if (key == "RE")  // result
        gameInformation(node)->setResult(valueList[0]);
    else if (key == "RO")  // round
        gameInformation(node)->setRound(valueList[0]);
    else if (key == "RU")  // rule
        gameInformation(node)->setRule(valueList[0]);
    else if (key == "SO")  // source
        gameInformation(node)->setSource(valueList[0]);
    else if (key == "TM")  // time
        gameInformation(node)->setTime(valueList[0]);
    else if (key == "US")  // user
        gameInformation(node)->setUser(valueList[0]);
    else if (key == "WR")  // white rank
        gameInformation(node)->setWhiteRank(valueList[0]);
    else if (key == "WT")  // white team
        gameInformation(node)->setWhiteTeam(valueList[0]);

    // Move Properties: B, KO, MN, W
    else if (key == "B"){  // black move
        if (info == NULL)
            return false;
        int x = -1, y = -1;
        parseMove(valueList[0], x, y);
        node->setColor(eBlack);
        if (x < 0 || x >= info->xsize() || y < 0 || y >= info->ysize())
            node->setPos(-1, -1);
        else
            node->setPos(x, y);
    }
//    else if (key == "KO")  // execute a given move
    else if (key == "MN")  // move number
        node->setMoveNumber(valueList[0].toInt());
    else if (key == "W"){  // white move
        if (info == NULL)
            return false;
        int x = -1, y = -1;
        parseMove(valueList[0], x, y);
        node->setColor(eWhite);
        if (x < 0 || x >= info->xsize() || y < 0 || y >= info->ysize())
            node->setPos(-1, -1);
        else
            node->setPos(x, y);
    }

    // Node Annotation Properties: C, DM, GB, GW, HO, N, UC, V
    else if (key == "C")  // comment
        node->setComment(valueList[0]);
    else if (key == "N")  // node name
        node->setName(valueList[0]);
    else if (key == "GB")  // good for black
        node->setNodeAnnotation(valueList[0].toInt() == 1 ? Go::Node::eGoodForBlack : Go::Node::eVeryGoodForBlack);
    else if (key == "GW")  // good for white
        node->setNodeAnnotation(valueList[0].toInt() == 1 ? Go::Node::eGoodForWhite : Go::Node::eVeryGoodForWhite);
    else if (key == "DM")  // even
        node->setNodeAnnotation(Go::Node::eEven);
    else if (key == "UC")  // unclear
        node->setNodeAnnotation(Go::Node::eUnclear);
    else if (key == "HO")  // hotspot
        node->setNodeAnnotation2(Go::Node::eHotspot);
    else if (key == "V")   // [real] value: positive values are good for black, negative values are good for white
        node->setEstimatedScore(valueList[0].toDouble());

    // Move Annotation Properties: BM, DO, IT, TE
    else if (key == "BM")  // move is bad
        node->setMoveAnnotation(valueList[0].toInt() == 1 ? Go::Node::eBadMove: Go::Node::eVeryBadMove);
    else if (key == "DO")  // move is doubtful
        node->setMoveAnnotation(Go::Node::eDoubtful);
    else if (key == "IT")  // move is interesting
        node->setMoveAnnotation(Go::Node::eInteresting);
    else if (key == "TE")  // move is tesuji (good move)
        node->setMoveAnnotation(valueList[0].toInt() == 1 ? Go::Node::eGoodMove : Go::Node::eVeryGoodMove);

    // Setup Properties: AB, AE, AW, PL
    else if (key == "AE" || key == "AB" || key == "AW"){  // add empty, add black, add white
        foreach(const QString& v, valueList){
            QList<QPoint> posList;
            parseMove(v, posList);
            foreach(const QPoint& pos, posList)
                if (key == "AE")  // add empty
                    node->emptyStones().push_back(pos);
                else if (key == "AB")  // add black
                    node->blackStones().push_back(pos);
                else if (key == "AW")  // add white
                    node->whiteStones().push_back(pos);
        }
    }
    else if (key == "PL"){  // whose turn
        if (valueList[0] == "B")
            node->setNextColor(Go::eBlack);
        else if (valueList[0] == "W")
            node->setNextColor(Go::eWhite);
    }

    // Markup Properties: AR, CR, DD, LB, LN, MA, SL, SQ, TR
    // Timing Properties: BL, OB, OW, WL
    // Miscellaneous Properties: FG, PM, VW

    if (!info && node->information())
        info = node->information();

    return true;
}

/**
  convert number property to int
*/
void Sgf::parseNumber(const QString& value, int& v1, int& v2){
    QStringList values = value.split(':');
    if (values.size() > 0)
        v1 = values[0].toInt();
    if (values.size() > 1)
        v2 = values[1].toInt();
}

/**
  convert move property to int
*/
void Sgf::parseMove(const QString& value, int& x, int& y){
    if (value.size() != 2)
        return;
    x = value[0].unicode() < L'a' ? value[0].unicode() - L'A' + 26 : value[0].unicode() - L'a';
    y = value[1].unicode() < L'a' ? value[1].unicode() - L'A' + 26 : value[1].unicode() - L'a';
}

/**
  convert move property to int group
*/
void Sgf::parseMove(const QString& value, QList<QPoint>& posList){
    QStringList valueList = value.split(':');
    if (valueList.size() == 0 || valueList.size() > 2)
        return;

    QList<QPoint> temp_plist;
    foreach(const QString& v, valueList){
        if (v.size() != 2)
            return;

        int x, y;
        parseMove(v, x, y);
        temp_plist.push_back( QPoint(x, y) );
    }

    if (temp_plist.size() == 1)
        posList.push_back(temp_plist[0]);
    else{
        if (temp_plist[0].x() > temp_plist[1].x())
            qSwap(temp_plist[0].rx(), temp_plist[1].rx());
        if (temp_plist[0].y() > temp_plist[1].y())
            qSwap(temp_plist[0].ry(), temp_plist[1].ry());
        for (int y=temp_plist[0].y(); y<=temp_plist[1].y(); ++y)
            for (int x=temp_plist[0].x(); x<=temp_plist[1].x(); ++x)
                posList.push_back(QPoint(x, y));
    }
}

/**
  convert position to sgf property
*/
QString Sgf::positionToSgfProperty(int x, int y){
    char s[3] = {0};
    s[0] = x < 26 ? 'a' + x : 'A' + x - 26;
    s[1] = y < 26 ? 'a' + y : 'A' + y - 26;
    return QString(s);
}

/**
  write sgf to stream
*/
bool Sgf::write(QTextStream& str){
    foreach(const NodePtr& game, gameList){
        str << '(';

        if (write(str, game) == false)
            return false;

        str << ")\n";
   }

    return true;
}

/**
  write node to stream
*/
bool Sgf::write(QTextStream& str, const NodePtr& node){
    // the node must be started with ';'
    str << ';';

    // if node has information, write it
    if (node->information()){
        if (writeRootInformation(str, node->information()) == false)
            return false;
        if (writeGameInformation(str, node->information()) == false)
            return false;
    }

    // write node properties
    if (writeNode(str, node) == false)
        return false;

    // write all children.
    foreach(const NodePtr& child, node->children())
        write(str, child);

    return true;
}

/**
  write root game information to stream
  write following properties: AP, CA, FF, GM, ST, SZ
*/
bool Sgf::writeRootInformation(QTextStream& str, const InformationPtr& info){
    if (!info)
        return false;

    str << "GM[1]FF[4]";
    str << "CA[" << str.codec()->name() << ']';
    str << "AP[" << APP_NAME  << ':' << APP_VERSION << ']';
    str << "ST[" << info->variationStyle() << ']';
    if (info->xsize() == info->ysize())
        str << "SZ[" << info->xsize() << ']';
    else
        str << "SZ[" << info->xsize() << ':' << info->ysize() << ']';

    return true;
}

/**
  write root game information to stream
  write following properties: AN, BR, BT, CP, DT, EV, GN, GC, ON, OT, PB, PC, PW, RE, RO, RU, SO, TM, US, WR, WT
*/
bool Sgf::writeGameInformation(QTextStream& str, const InformationPtr& info){
#define WRITE_PROP(NAME, VALUE) if (VALUE.isEmpty() == false) str << NAME"[" << VALUE << ']'

    if (!info)
        return false;

    // black
    WRITE_PROP("PB", info->blackPlayer());
    WRITE_PROP("BR", info->blackRank());
    WRITE_PROP("BT", info->blackTeam());

    // white
    WRITE_PROP("PW", info->whitePlayer());
    WRITE_PROP("WR", info->whiteRank());
    WRITE_PROP("WT", info->whiteTeam());

    // when and where
    WRITE_PROP("DT", info->date());
    WRITE_PROP("PC", info->place());

    // game name
    WRITE_PROP("EV", info->event());
    WRITE_PROP("GN", info->gameName());
    WRITE_PROP("RO", info->round());

    // result
    WRITE_PROP("RE", info->result());

    // rule
    WRITE_PROP("RU", info->rule());
    WRITE_PROP("TM", info->time());
    WRITE_PROP("OT", info->overtime());

    // annotation
    WRITE_PROP("AN", info->annotation());
    WRITE_PROP("CP", info->copyright());
    WRITE_PROP("GC", info->gameComment());
    WRITE_PROP("ON", info->opening());
    WRITE_PROP("SO", info->source());
    WRITE_PROP("US", info->user());

    return true;
}

/**
  write node properties to stream
*/
bool Sgf::writeNode(QTextStream& str, const NodePtr& node){
/*
Markup Properties           AR, CR, DD, LB, LN, MA, SL, SQ, TR
Timing Properties           BL, OB, OW, WL
Miscellaneous Properties    FG, PM, VW
*/
    // Move Properties: B, KO, MN, W
    // unsupported property: KO
    if (node->color() != eDame){
        char pos[3] = {0};
        if (node->x() >= 0 && node->y() >= 0){
            pos[0] = char('a' + node->x());
            pos[1] = char('a' + node->y());
        }
        if (node->color() == eBlack)
            str << "B[" << pos << ']';
        else if (node->color() == eWhite)
            str << "W[" << pos << ']';
    }

    if (node->moveNumber() > 0)
        str << "MN[" << node->moveNumber() << ']';

    // Node Annotation Properties: C, N, DM, GB, GW, HO, UC, V
    if (node->comment().isEmpty() == false)
        str << "C[" << node->comment() << ']';
    if (node->name().isEmpty() == false)
        str << "N[" << node->name() << ']';
    switch(node->nodeAnnotation()){
        case Go::Node::eGoodForBlack:
            str << "GB[1]";
            break;
        case Go::Node::eVeryGoodForBlack:
            str << "GB[2]";
            break;
        case Go::Node::eGoodForWhite:
            str << "GW[1]";
            break;
        case Go::Node::eVeryGoodForWhite:
            str << "GW[2]";
            break;
        case Go::Node::eEven:
            str << "DM[]";
            break;
        case Go::Node::eUnclear:
            str << "UC[]";
            break;
    }
    switch(node->nodeAnnotation2()){
        case Go::Node::eHotspot:
            str << "HO[]";
            break;
    }
    if (node->hasEstimatedScore())
        str << "V[" << node->estimatedScore() << ']';

    //Move Annotation Properties: BM, DO, IT, TE
    switch(node->moveAnnotation()){
        case Go::Node::eGoodMove:
            str << "TE[1]";
            break;
        case Go::Node::eVeryGoodMove:
            str << "TE[2]";
            break;
        case Go::Node::eBadMove:
            str << "BM[1]";
            break;
        case Go::Node::eVeryBadMove:
            str << "BM[2]";
            break;
        case Go::Node::eDoubtful:
            str << "DO[]";
            break;
        case Go::Node::eInteresting:
            str << "IT[]";
            break;
    }

    // Setup Properties: AB, AE, AW, PL
    foreach(const QPoint& p, node->emptyStones())
        str << "AE[" << positionToSgfProperty(p.x(), p.y()) << ']';

    foreach(const QPoint& p, node->blackStones())
        str << "AB[" << positionToSgfProperty(p.x(), p.y()) << ']';

    foreach(const QPoint& p, node->whiteStones())
        str << "AW[" << positionToSgfProperty(p.x(), p.y()) << ']';

    return true;
}


}
