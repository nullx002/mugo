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
    int len = 'Z' - 'A' + (showI ? 1 : 0);
    int xx = 'A' + x % len;
    if (showI == false && xx >= 'I')
        ++xx;

    QString str;
    str.sprintf("%c%d", xx, ysize - y);
    return str;
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
    , moveNumber_(-1)

{
}

/**
  Constructs empty node with parent.

*/
Node::Node(const NodePtr& parentNode)
    : parent_(parentNode)
    , color_(eDame)
    , nextColor_(eDame)
    , moveNumber_(-1)
{
}

/**
  Destructor
*/
Node::~Node(){
}


}
