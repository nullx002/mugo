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
Node::Node(const NodePtr& parent)
    : parent_(parent)
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
