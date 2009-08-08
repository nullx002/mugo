#include <QDebug>
#include "appdef.h"
#include "godata.h"


namespace go{


node* createBlackNode(node* parent){
    node* newNode = new node(parent);
    newNode->setBlack();
    return newNode;
}

node* createBlackNode(node* parent, int x, int y){
    node* newNode = createBlackNode(parent);
    newNode->setX(x);
    newNode->setY(y);
    return newNode;
}

node* createWhiteNode(node* parent){
    node* newNode = new node(parent);
    newNode->setWhite();
    return newNode;
}

node* createWhiteNode(node* parent, int x, int y){
    node* newNode = createWhiteNode(parent);
    newNode->setX(x);
    newNode->setY(y);
    return newNode;
}



node::node(data* data_) : goData(data_), parent(NULL), annotation(eNoAnnotation), black(false), white(false), moveNumber(-1){
}

node::node(node* parent_) : goData(parent_->goData), parent(parent_), annotation(eNoAnnotation), black(false), white(false), moveNumber(-1){
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

bool node::isPass() const{
    return position.x < 0 || position.y < 0 || position.x >= goData->root.xsize || position.y >= goData->root.ysize;
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

    if (annotation & eGoodMove)
        str += QString(" [%1]").arg( tr("Good Move") );
    if (annotation & eVeryGoodMove)
        str += QString(" [%1]").arg( tr("Very Good Move") );
    if (annotation & eBadMove)
        str += QString(" [%1]").arg( tr("Bad Move") );
    if (annotation & eVeryBadMove)
        str += QString(" [%1]").arg( tr("Very Bad Move") );
    if (annotation & eDoubtfulMove)
        str += QString(" [%1]").arg( tr("Doubtful Move") );
    if (annotation & eInterestingMove)
        str += QString(" [%1]").arg( tr("Interesting Move") );
    if (annotation & eEven)
        str += QString(" [%1]").arg( tr("Even") );
    if (annotation & eGoodForBlack)
        str += QString(" [%1]").arg( tr("Good for Black") );
    if (annotation & eVeryGoodForBlack)
        str += QString(" [%1]").arg( tr("Very Good for Black") );
    if (annotation & eGoodForWhite)
        str += QString(" [%1]").arg( tr("Good for White") );
    if (annotation & eVeryGoodForWhite)
        str += QString(" [%1]").arg( tr("Very Good for White") );
    if (annotation & eUnclear)
        str += QString(" [%1]").arg( tr("Unclear") );
    if (annotation & eHotspot)
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
    root.clear();
    root = informationNode(this);
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
