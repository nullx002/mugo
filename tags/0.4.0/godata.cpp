#include <QDebug>
#include "appdef.h"
#include "godata.h"

namespace go{


node::node(data* data_) : goData(data_), parent(NULL), annotation(eNoAnnotation){
}

node::node(node* parent_) : goData(parent_->goData), parent(parent_), annotation(eNoAnnotation){
}

void node::clear(){
    parent = NULL;

    nodeList::iterator iter = childNodes.begin();
    while (iter != childNodes.end()){
            delete *iter;
            ++iter;
    }
    childNodes.clear();
}

QString node::toString() const{
    QString str = nodeName();

    if (!stones.empty())
        str.append(" Add");

    if (!crosses.empty() || !triangles.empty() || !squares.empty() || !circles.empty() || !characters.empty())
        str.append(" Mark");

    if (!blackTerritories.empty())
        str.append(" BlackTerritories");

    if (!whiteTerritories.empty())
        str.append(" WhiteTerritories");

    if (!comment.isEmpty())
        str.append(" Comment");

    if (annotation & eGoodMove)
        str.append(" [Good Move]");
    if (annotation & eVeryGoodMove)
        str.append(" [Very Good Move]");
    if (annotation & eBadMove)
        str.append(" [Bad Move]");
    if (annotation & eVeryBadMove)
        str.append(" [Very Bad Move]");
    if (annotation & eDoubtfulMove)
        str.append(" [Doubtful Move]");
    if (annotation & eInterestingMove)
        str.append(" [Interesting Move]");
    if (annotation & eEven)
        str.append(" [Even]");
    if (annotation & eGoodForBlack)
        str.append(" [Good for Black]");
    if (annotation & eVeryGoodForBlack)
        str.append(" [Very Good for Black]");
    if (annotation & eGoodForWhite)
        str.append(" [Good for White]");
    if (annotation & eVeryGoodForWhite)
        str.append(" [Very Good for White]");
    if (annotation & eUnclear)
        str.append(" [Unclear]");
    if (annotation & eHotspot)
        str.append(" [Hotspot]");

    if (!str.isEmpty() && str[0] == ' ')
        str.remove(0, 1);

    return str;
}

void informationNode::initialize(){
    xsize = 19;
    ysize = 19;
    komi = 6.5;
    handicap = 0;
}

bool stoneNode::isPass() const{
    return position.x < 0 || position.y < 0 || position.x >= goData->root.xsize || position.y >= goData->root.ysize;
}


void data::clear(){
    root.clear();
    root = informationNode(this);
}


bool fileBase::read(const QString& fname, QTextCodec* codec){
    QFile f(fname);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QByteArray a = f.read( f.size() );
    QString s = codec->toUnicode(a);
    QString::iterator iter = s.begin();
    return readStream(iter, s.end());
}

bool fileBase::save(const QString& fname, QTextCodec* codec){
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
