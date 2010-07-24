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
#include "godata.h"


namespace Go{


NodePtr createInformationNode(NodePtr parent){
    NodePtr newNode( new Node(parent) );
    GameInformationPtr info( new GameInformation );
    newNode->gameInformation = info;
    newNode->nextColor = black;

    return newNode;
}

NodePtr createBlackNode(NodePtr parent){
    NodePtr newNode( new Node(parent) );
    newNode->color = black;
    return newNode;
}

NodePtr createBlackNode(NodePtr parent, int x, int y){
    NodePtr newNode( createBlackNode(parent) );
    newNode->position.set(x, y);
    return newNode;
}

NodePtr createWhiteNode(NodePtr parent){
    NodePtr newNode( new Node(parent) );
    newNode->color = white;
    return newNode;
}

NodePtr createWhiteNode(NodePtr parent, int x, int y){
    NodePtr newNode( createWhiteNode(parent) );
    newNode->position.set(x, y);
    return newNode;
}


/**
  Constructor
*/
GameInformation::GameInformation()
    : xsize(19)
    , ysize(19)
    , komi(6.5)
    , handicap(0)
{
}


/**
  Constructor
*/
Node::Node()
    : annotation(noAnnotation)
    , moveAnnotation(noMoveAnnotation)
    , nodeAnnotation(noNodeAnnotation)
    , color(empty)
    , nextColor(empty)
    , moveNumber(0)
{
}

/**
  Constructor
*/
Node::Node(NodePtr p_)
    : parent_(p_)
    , annotation(noAnnotation)
    , moveAnnotation(noMoveAnnotation)
    , nodeAnnotation(noNodeAnnotation)
    , color(empty)
    , nextColor(empty)
    , moveNumber(0)
{
}

/**
  Destructor
*/
Node::~Node(){
    clear();
}

/**
  clear this node
*/
void Node::clear(){
    parent_.reset();
    childNodes.clear();
}

/*
QString Node::toString() const{
    QString str = name;

    if (moveNumber > 0)
        str += QString(" (%1)").arg(moveNumber);

    if (!comment.isEmpty())
        str += QString(" %1").arg( tr("Comment") );

    if (!emptyStones.empty())
        str += QString(" %1").arg( tr("Add Empty") );

    if (!whiteStones.empty())
        str += QString(" %1").arg( tr("Add White") );

    if (!blackStones.empty())
        str += QString(" %1").arg( tr("Add Black") );

    if (!dims.empty())
        str += QString(" %1").arg( tr("Dim") );

    if (!marks.empty())
        str += QString(" %1").arg( tr("Mark") );

    if (!blackTerritories.empty())
        str += QString(" %1").arg( tr("BlackTerritories") );

    if (!whiteTerritories.empty())
        str += QString(" %1").arg( tr("WhiteTerritories") );

    if (moveAnnotation == goodMove)
        str += QString(" [%1]").arg( tr("Good Move") );
    else if (moveAnnotation == veryGoodMove)
        str += QString(" [%1]").arg( tr("Very Good Move") );
    else if (moveAnnotation == badMove)
        str += QString(" [%1]").arg( tr("Bad Move") );
    else if (moveAnnotation == veryBadMove)
        str += QString(" [%1]").arg( tr("Very Bad Move") );
    else if (moveAnnotation == doubtfulMove)
        str += QString(" [%1]").arg( tr("Doubtful Move") );
    else if (moveAnnotation == interestingMove)
        str += QString(" [%1]").arg( tr("Interesting Move") );

    if (nodeAnnotation == even)
        str += QString(" [%1]").arg( tr("Even") );
    else if (nodeAnnotation == goodForBlack)
        str += QString(" [%1]").arg( tr("Good for Black") );
    else if (nodeAnnotation == veryGoodForBlack)
        str += QString(" [%1]").arg( tr("Very Good for Black") );
    else if (nodeAnnotation == goodForWhite)
        str += QString(" [%1]").arg( tr("Good for White") );
    else if (nodeAnnotation == veryGoodForWhite)
        str += QString(" [%1]").arg( tr("Very Good for White") );
    else if (nodeAnnotation == unclear)
        str += QString(" [%1]").arg( tr("Unclear") );

    if (annotation == hotspot)
        str += QString(" [%1]").arg( tr("Hotspot") );

    if (!str.isEmpty() && str[0] == ' ')
        str.remove(0, 1);

    return str;
}
*/

/**
  get game information
*/
GameInformationPtr Node::getInformation() const{
    if (gameInformation)
        return gameInformation;

    NodePtr node = parent();
    while (node){
        if (node->gameInformation)
            return node->gameInformation;
        node = node->parent();
    };

    return gameInformation;
}

/**
  get siblings
*/
NodeList Node::siblings() const{
    NodeList nodeList;

    NodePtr p = parent();
    if (p){
        NodeList::iterator iter = p->childNodes.begin();
        while (iter != p->childNodes.end()){
            if ((*iter).get() != this)
                nodeList.push_back(*iter);
            ++iter;
        }
    }

    return nodeList;
}

/**
  get previous sibling
*/
NodePtr Node::previousSibling() const{
    Go::NodePtr p = parent();
    if (!p)
        return NodePtr();

    int i = 0;
    foreach(const Go::NodePtr& node, p->childNodes){
        if (node.get() == this)
            break;
        ++i;
    }

    if (i > 0 && i < p->childNodes.size())
        return p->childNodes[i - 1];
    else
        return NodePtr();
}

/**
  get next sibling
*/
NodePtr Node::nextSibling() const{
    Go::NodePtr p = parent();
    if (!p)
        return NodePtr();

    int i = 0;
    foreach(const Go::NodePtr& node, p->childNodes){
        if (node.get() == this)
            break;
        ++i;
    }

    if (i < p->childNodes.size() - 1)
        return p->childNodes[i + 1];
    else
        return NodePtr();
}

/**
  is pass

  @retval true  pass
  @retval false not pass
*/
bool Node::isPass() const{
    GameInformationPtr info = getInformation();
    if (info)
        return position.x < 0 || position.y < 0 || position.x >= info->xsize || position.y >= info->ysize;
    else
        return true;
}


bool FileBase::read(const QString& fname, QTextCodec* defaultCodec, bool guessCodec){
    QFile f(fname);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text))
        return false;

    QByteArray bytes = f.read( f.size() );
    return read(bytes, defaultCodec, guessCodec);
}

bool FileBase::read(const QByteArray& bytes, QTextCodec* defaultCodec, bool guessCodec){
    QTextCodec* codec_ = guessCodec ? this->guessCodec(bytes) : NULL;
    if (codec_)
        qDebug() << "file codec is " << codec_->name();
    else if (guessCodec)
        qDebug() << "unknown codec, use default codec: " << defaultCodec->name();
    else
        qDebug() << "use default codec: " << defaultCodec->name();

    codec = codec_ ? codec_ : defaultCodec;
    QString s = codec->toUnicode(bytes);

    // yen sign problem.
    QChar yen[2] = {0x005C, 0x00A5};
    QByteArray byte_yen = codec->fromUnicode(yen, 2);
    if (byte_yen.size() == 2 && byte_yen[0] == byte_yen[1])
        s.replace(QChar(0x00A5), QChar(0x005C));

    QString::iterator iter = s.begin();
    return readStream(iter, s.end());
}

bool FileBase::save(const QString& fname, QTextCodec* codec_){
    codec = codec_;

    QFile f(fname);
    if (!f.open(QIODevice::WriteOnly))
        return false;

    QString str;
    QTextStream stream(&str, QIODevice::WriteOnly);
    bool ret = saveStream(stream);
    stream.flush();

    QByteArray a = codec->fromUnicode(str);
    f.write(a);

    return ret;
}

QString FileBase::readLine(QString::iterator& first, QString::iterator& last){
    QString str;
    while (first != last){
        QChar c = *first++;
        if (c == '\n')
            break;
        str.push_back(c);
    }
    return str;
}


};
