#include <QDebug>
#include "godata.h"


namespace Go{


NodePtr createStoneNode(Color color, int x, int y){
    NodePtr node(new Node);
    node->setColor(color);
    node->setX(x);
    node->setY(y);

    return node;
}


Node::Node() :
    color_(eDame),
    nextColor_(eDame)
{
}

Node::Node(const NodePtr& parent) :
    parent_(parent),
    color_(eDame),
    nextColor_(eDame)
{
}

Node::~Node(){
    qDebug() << "destructor: " << name_;
}


}
