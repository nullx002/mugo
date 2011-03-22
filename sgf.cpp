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
            if (parseBranch(first, last, root) == false)
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
bool Sgf::parseBranch(QString::const_iterator& first, QString::const_iterator last, NodePtr parent){
    if (*first != '(')
        return false;
    ++first;

    while (first != last){
        if (*first == ')'){
            ++first;
            return true;
        }
        else if (*first == '('){
            if (parseBranch(first, last, parent) == false)
                return false;
        }
//        else if (*first == ';'){  // root node of cyber oro does not start ';'
        else if (first->isSpace() == false){
            NodePtr node(new Node(parent));
            if (parseNode(first, last, parent, node) == false)
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
bool Sgf::parseNode(QString::const_iterator& first, QString::const_iterator last, NodePtr parent, NodePtr& node){
    if (*first != ';')
        return false;
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

        addPropertyToNode(node, key, valueList);

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
        else if (*first == '\\')
            ++first;
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
bool Sgf::addPropertyToNode(NodePtr& node, const QString& key, const QStringList& valueList){
/*
Move Properties             B, KO, MN, W
Setup Properties            AB, AE, AW, PL
Node Annotation Properties  C, DM, GB, GW, HO, N, UC, V
Move Annotation Properties  BM, DO, IT, TE
Markup Properties           AR, CR, DD, LB, LN, MA, SL, SQ, TR
Root Properties             AP, CA, FF, GM, ST, SZ
Game Info Properties        AN, BR, BT, CP, DT, EV, GN, GC, ON, OT, PB, PC, PW, RE, RO, RU, SO, TM, US, WR, WT
Timing Properties           BL, OB, OW, WL
Miscellaneous Properties    FG, PM, VW
*/
    // Root Properties: AP, CA, FF, GM, ST, SZ
    if (key == "AP")  // application name
        gameInformation(node)->setApplicationName(valueList.join(":"));
//    else if (key == "CA")  // character set
//    else if (key == "FF")  // file format
//    else if (key == "GM")  // game type, only support 1
    else if (key == "ST")  // how variations should be shown
        gameInformation(node)->setVariationStyle(valueList.empty() == false ? valueList[0].toInt() : 0);
    else if (key == "SZ"){  // board size
        if (valueList.empty() == false){
            int xsize = 0, ysize = 0;
            parseNumber(valueList[0], xsize, ysize);
            if (xsize > 0 && ysize > 0)
                gameInformation(node)->setSize(xsize, ysize);
            else if (xsize > 0)
                gameInformation(node)->setSize(xsize, xsize);
            else
                gameInformation(node)->setSize(19, 19);
        }
        else
            gameInformation(node)->setSize(19, 19);
    }

    // Game Info Properties: AN, BR, BT, CP, DT, EV, GN, GC, ON, OT, PB, PC, PW, RE, RO, RU, SO, TM, US, WR, WT
    else if (key == "AN")  // name of the person, who made the annotation to the game
        gameInformation(node)->setAnnotation(valueList.join(":"));
    else if (key == "BR")  // black rank
        gameInformation(node)->setBlackRank(valueList.join(":"));
    else if (key == "BT")  // black team
        gameInformation(node)->setBlackTeam(valueList.join(":"));
    else if (key == "CP")  // copyright
        gameInformation(node)->setCopyright(valueList.join(":"));
    else if (key == "DT")  // date
        gameInformation(node)->setDate(valueList.join(":"));
    else if (key == "EV")  // event name
        gameInformation(node)->setEvent(valueList.join(":"));
    else if (key == "GN")  // game name
        gameInformation(node)->setGameName(valueList.join(":"));
    else if (key == "GC")  // extra information about the game
        gameInformation(node)->setGameComment(valueList.join(":"));
    else if (key == "ON")  // opening, fuseki
        gameInformation(node)->setOpening(valueList.join(":"));
    else if (key == "OT")  // overtime, byo-yomi
        gameInformation(node)->setOvertime(valueList.join(":"));
    else if (key == "PB")  // black player
        gameInformation(node)->setBlackPlayer(valueList.join(":"));
    else if (key == "PC")  // place where game was played
        gameInformation(node)->setPlace(valueList.join(":"));
    else if (key == "PW")  // white player
        gameInformation(node)->setWhitePlayer(valueList.join(":"));
    else if (key == "RE")  // result
        gameInformation(node)->setResult(valueList.join(":"));
    else if (key == "RO")  // round
        gameInformation(node)->setRound(valueList.join(":"));
    else if (key == "RU")  // rule
        gameInformation(node)->setRule(valueList.join(":"));
    else if (key == "SO")  // source
        gameInformation(node)->setSource(valueList.join(":"));
    else if (key == "TM")  // time
        gameInformation(node)->setTime(valueList.join(":"));
    else if (key == "US")  // user
        gameInformation(node)->setUser(valueList.join(":"));
    else if (key == "WR")  // white rank
        gameInformation(node)->setWhiteRank(valueList.join(":"));
    else if (key == "WT")  // white team
        gameInformation(node)->setWhiteTeam(valueList.join(":"));

    // Move Properties: B, KO, MN, W
    else if (key == "B"){  // black move
        if (valueList.empty() == false){
            int x = -1, y = -1;
            parseMove(valueList[0], x, y);
            node->setColor(eBlack);
            node->setPos(x, y);
        }
    }
//    else if (key == "KO")  // execute a given move
    else if (key == "MN"){  // move number
        if (valueList.empty() == false)
            node->setMoveNumber(valueList[0].toInt());
    }
    else if (key == "W"){  // white move
        if (valueList.empty() == false){
            int x = -1, y = -1;
            parseMove(valueList[0], x, y);
            node->setColor(eWhite);
            node->setPos(x, y);
        }
    }

    // Node Annotation Properties: C, DM, GB, GW, HO, N, UC, V
    else if (key == "C")  // comment
        node->setComment(valueList.join(":"));
    else if (key == "N")  // node name
        node->setName(valueList.join(":"));
//    else if (key == "DM")  // even
//    else if (key == "GB")  // good for black
//    else if (key == "GW")  // good for white
//    else if (key == "HO")  // hotspot
//    else if (key == "UC")  // unclear
//    else if (key == "V")   // [real] value: positive values are good for black, negative values are good for white

    // Move Annotation Properties: BM, DO, IT, TE
    // Setup Properties: AB, AE, AW, PL
    // Markup Properties: AR, CR, DD, LB, LN, MA, SL, SQ, TR
    // Timing Properties: BL, OB, OW, WL
    // Miscellaneous Properties: FG, PM, VW

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
    x = value[0].unicode() - L'a';
    y = value[1].unicode() - L'a';
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
Setup Properties            AB, AE, AW, PL
Node Annotation Properties  C, DM, GB, GW, HO, N, UC, V
Move Annotation Properties  BM, DO, IT, TE
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

    return true;
}


}
