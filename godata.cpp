#include <QDebug>
#include "appdef.h"
#include "godata.h"


namespace go{


nodePtr createBlackNode(nodePtr parent){
    nodePtr newNode( new node(parent) );
    newNode->setBlack();
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
    newNode->setWhite();
    return newNode;
}

nodePtr createWhiteNode(nodePtr parent, int x, int y){
    nodePtr newNode( createWhiteNode(parent) );
    newNode->setX(x);
    newNode->setY(y);
    return newNode;
}



node::node(data* data_) : goData(data_), annotation(eNoAnnotation), moveAnnotation(eNoAnnotation), nodeAnnotation(eNoAnnotation), black(false), white(false), moveNumber(-1){
}

node::node(nodePtr parent_) : goData(parent_->goData), parent(parent_), annotation(eNoAnnotation), moveAnnotation(eNoAnnotation), nodeAnnotation(eNoAnnotation), black(false), white(false), moveNumber(-1){
}

void node::clear(){
    parent.reset();
    childNodes.clear();
}

bool node::isPass() const{
    return position.x < 0 || position.y < 0 || position.x >= goData->root->xsize || position.y >= goData->root->ysize;
}

QString node::toString() const{
    QString str = nodeName();

    if (moveNumber > 0)
        str += QString(" (%1)").arg(moveNumber);

    if (!stones.empty())
        str += QString(" %1").arg( tr("Add") );

    if (!crosses.empty() || !triangles.empty() || !squares.empty() || !circles.empty() || !characters.empty())
        str += QString(" %1").arg( tr("Mark") );

    if (!blackTerritories.empty())
        str += QString(" %1").arg( tr("BlackTerritories") );

    if (!whiteTerritories.empty())
        str += QString(" %1").arg( tr("WhiteTerritories") );

    if (!comment.isEmpty())
        str += QString(" %1").arg( tr("Comment") );

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

void informationNode::initialize(){
    xsize = 19;
    ysize = 19;
    komi = 6.5;
    handicap = 0;
    white = true;
}

QString informationNode::nodeName() const{
    static const QString str = tr("GameInfo");
    return str;
}


void data::clear(){
    root.reset( new informationNode(this) );
}


bool fileBase::read(const QString& fname, QTextCodec* codec){
    this->codec = codec;

    QFile f(fname);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QByteArray a = f.read( f.size() );
    QString s = codec->toUnicode(a);
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



};
