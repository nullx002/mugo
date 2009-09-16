#include <QDebug>
#include "appdef.h"
#include "godata.h"


namespace go{

const informationNode* infomation(const node* node){
    while (node->parent){
        node = node->parent.get();
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



node::node(nodePtr parent_) : parent(parent_), annotation(eNoAnnotation), moveAnnotation(eNoAnnotation), nodeAnnotation(eNoAnnotation), color(go::empty), moveNumber(-1){
}

void node::clear(){
    parent.reset();
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

    if (!emptyStones.empty())
        str += QString(" %1").arg( tr("Add Empty") );

    if (!whiteStones.empty())
        str += QString(" %1").arg( tr("Add White") );

    if (!blackStones.empty())
        str += QString(" %1").arg( tr("Add Black") );

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
    color = go::white;
}

QString informationNode::nodeName() const{
    static const QString str = tr("GameInfo");
    return str;
}


void data::clear(){
    root.reset( new informationNode(nodePtr()) );
}


bool fileBase::read(const QString& fname, QTextCodec* defaultCodec, bool guessCodec){
    QFile f(fname);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text))
        return false;

    QByteArray bytes = f.read( f.size() );
    return read(bytes, defaultCodec, guessCodec);
}

bool fileBase::read(const QByteArray& bytes, QTextCodec* defaultCodec, bool guessCodec){
    this->codec = codec;

    QTextCodec* codec = guessCodec ? getCodec(bytes) : NULL;
    if (codec)
        qDebug() << "file codec is " << codec->name();
    else if (guessCodec)
        qDebug() << "unknown codec, use default codec: " << defaultCodec->name();
    else
        qDebug() << "use default codec: " << defaultCodec->name();

    QString s = codec ? codec->toUnicode(bytes) : defaultCodec->toUnicode(bytes);
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
