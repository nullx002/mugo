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
#include "godata.h"


namespace Go{



/**
  create black or white node
*/
NodePtr createStoneNode(Color color, int x, int y){
    NodePtr node(new Node);
    node->setColor(color);
    node->setX(x);
    node->setY(y);

    return node;
}

/**
  get coordinate string from xy position
*/
QString coordinateString(int xsize, int ysize, int x, int y, bool showI){
    if (x < 0 || y < 0 || x >= xsize || y >= ysize)
        return QString();

    int len = 'Z' - 'A' + (showI ? 1 : 0);
    int xx = 'A' + x % len;
    if (showI == false && xx >= 'I')
        ++xx;

    QString str;
    str.sprintf("%c%d", xx, ysize - y);
    return str;
}

/**
  copy node list
*/
void copyNodeList(NodeList& dstList, const NodeList& srcList){
    foreach(const NodePtr& src, srcList){
        NodePtr dst;
        copyNode(dst, src);
        dstList.push_back(dst);
    }
}

/**
  copy node
*/
void copyNode(NodePtr& dst, const NodePtr& src){
    // copy src to dst
    NodePtr node(new Node(*src));
    node->children().clear();
    if (src->information())
        node->setInformation( InformationPtr(new Information(*src->information())) );
    dst = node;

    foreach(const NodePtr& srcChild, src->children()){
        NodePtr newChild;
        copyNode(newChild, srcChild);
        dst->children().push_back(newChild);
    }
}

/**
  Constructs empty game information
*/
Information::Information()
    : variationStyle_(0)
    , xsize_(19)
    , ysize_(19)
    , komi_(6.5)
    , handicap_(0)
{
}

/**
  Constructs empty node
*/
Node::Node()
    : color_(eDame)
    , nextColor_(eDame)
    , x_(-1)
    , y_(-1)
    , moveNumber_(-1)
    , nodeAnnotation_()
    , nodeAnnotation2_()
    , moveAnnotation_()
    , hasEstimatedScore_(false)
    , estimatedScore_(0.0)

{
}

/**
  Constructs empty node with parent.

*/
Node::Node(const NodePtr& parentNode)
    : parent_(parentNode)
    , color_(eDame)
    , nextColor_(eDame)
    , x_(-1)
    , y_(-1)
    , moveNumber_(-1)
    , nodeAnnotation_()
    , nodeAnnotation2_()
    , moveAnnotation_()
    , hasEstimatedScore_(false)
    , estimatedScore_(0.0)
{
}

/**
  Destructor
*/
Node::~Node(){
//    qDebug() << "Node::~Node";
}


}
