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
#include "godata.h"


namespace go{

const informationNode* infomation(const node* node){
    while (node->parent()){
        node = node->parent().get();
    }
    return (informationNode*)node;
}

nodePtr createBlackNode(nodePtr parent){
    nodePtr newNode( new node(parent) );
    newNode->setColor(go::black);
    return newNode;
}

nodePtr createBlackNode(nodePtr parent, int x, int y){
    nodePtr newNode( createBlackNode(parent) );
    newNode->setX(x);
    newNode->setY(y);
    return newNode;
}

nodePtr createWhiteNode(nodePtr parent){
    nodePtr newNode( new node(parent) );
    newNode->setColor(go::white);
    return newNode;
}

nodePtr createWhiteNode(nodePtr parent, int x, int y){
    nodePtr newNode( createWhiteNode(parent) );
    newNode->setX(x);
    newNode->setY(y);
    return newNode;
}



node::node()
    : annotation(eNoAnnotation)
    , moveAnnotation(eNoAnnotation)
    , nodeAnnotation(eNoAnnotation)
    , color(go::empty)
    , nextColor(go::empty)
    , moveNumber(-1)
{
}

node::node(nodePtr p_)
    : parent_(p_)
    , annotation(eNoAnnotation)
    , moveAnnotation(eNoAnnotation)
    , nodeAnnotation(eNoAnnotation)
    , color(go::empty)
    , nextColor(go::empty)
    , moveNumber(-1)
{
}

void node::clear(){
    parent_.reset();
    childNodes.clear();
}

bool node::isPass() const{
    const informationNode* info = infomation(this);
    return position.x < 0 || position.y < 0 || position.x >= info->xsize || position.y >= info->ysize;
}

QString node::toString() const{
    QString str = nodeName();

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

    if (moveAnnotation == eGoodMove)
        str += QString(" [%1]").arg( tr("Good Move") );
    else if (moveAnnotation == eVeryGoodMove)
        str += QString(" [%1]").arg( tr("Very Good Move") );
    else if (moveAnnotation == eBadMove)
        str += QString(" [%1]").arg( tr("Bad Move") );
    else if (moveAnnotation == eVeryBadMove)
        str += QString(" [%1]").arg( tr("Very Bad Move") );
    else if (moveAnnotation == eDoubtfulMove)
        str += QString(" [%1]").arg( tr("Doubtful Move") );
    else if (moveAnnotation == eInterestingMove)
        str += QString(" [%1]").arg( tr("Interesting Move") );

    if (nodeAnnotation == eEven)
        str += QString(" [%1]").arg( tr("Even") );
    else if (nodeAnnotation == eGoodForBlack)
        str += QString(" [%1]").arg( tr("Good for Black") );
    else if (nodeAnnotation == eVeryGoodForBlack)
        str += QString(" [%1]").arg( tr("Very Good for Black") );
    else if (nodeAnnotation == eGoodForWhite)
        str += QString(" [%1]").arg( tr("Good for White") );
    else if (nodeAnnotation == eVeryGoodForWhite)
        str += QString(" [%1]").arg( tr("Very Good for White") );
    else if (nodeAnnotation == eUnclear)
        str += QString(" [%1]").arg( tr("Unclear") );

    if (annotation == eHotspot)
        str += QString(" [%1]").arg( tr("Hotspot") );

    if (!str.isEmpty() && str[0] == ' ')
        str.remove(0, 1);

    return str;
}

informationNode::informationNode(nodePtr parent) : node(parent){
    initialize();
}

informationNode::~informationNode(){
}

void informationNode::initialize(){
    xsize = 19;
    ysize = 19;
    komi = 6.5;
    handicap = 0;
    nextColor = go::black;
}

QString informationNode::nodeName() const{
    static const QString str = tr("GameInfo");
    return str;
}


data::data() : root(new informationNode()){
    rootList.push_back( root );
}

void data::clear(){
    rootList.clear();
    root.reset( new informationNode(nodePtr()) );
    rootList.push_back( root );
}


bool fileBase::read(const QString& fname, QTextCodec* defaultCodec, bool guessCodec){
    QFile f(fname);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text))
        return false;

    QByteArray bytes = f.read( f.size() );
    return read(bytes, defaultCodec, guessCodec);
}

bool fileBase::read(const QByteArray& bytes, QTextCodec* defaultCodec, bool guessCodec){
    QTextCodec* codec = guessCodec ? getCodec(bytes) : NULL;
    if (codec)
        qDebug() << "file codec is " << codec->name();
    else if (guessCodec)
        qDebug() << "unknown codec, use default codec: " << defaultCodec->name();
    else
        qDebug() << "use default codec: " << defaultCodec->name();

    this->codec = codec ? codec : defaultCodec;
    QString s = this->codec->toUnicode(bytes);

    // yen sign problem.
    QChar chars[2] = {0x005C, 0x00A5};
    QByteArray ba = this->codec->fromUnicode(chars, 2);
    if (ba.size() == 2 && ba[0] == ba[1])
        s.replace(QChar(0x00A5), QChar(0x5C));

    QString::iterator iter = s.begin();
    return readStream(iter, s.end());
}

bool fileBase::save(const QString& fname, QTextCodec* codec){
    this->codec = codec;

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

QString fileBase::readLine(QString::iterator& first, QString::iterator& last){
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


};
