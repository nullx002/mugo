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
#include "command.h"
#include "sgfdocument.h"

/**
  Constructor
*/
AddNodeCommand::AddNodeCommand(SgfDocument* doc, Go::NodePtr parentNode_, Go::NodePtr node_, int index_, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document(doc)
    , parentNode(parentNode_)
    , node(node_)
    , index(index_)
{
}

/**
  redo add node command
*/
void AddNodeCommand::redo(){
    setText( tr("Add Stone") );
    document->addNode(parentNode, node, index);
}

/**
  undo add node command
*/
void AddNodeCommand::undo(){
    document->deleteNode(node, true);
}


/**
  Constructor
*/
DeleteNodeCommand::DeleteNodeCommand(SgfDocument* doc, Go::NodePtr node_, bool removeChildren_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , removeChildren(removeChildren_)
{
    parentNode = node->parent();
    pos = parentNode->childNodes.indexOf(node);
}

/**
  redo delete node command
*/
void DeleteNodeCommand::redo(){
    setText( tr("Delete Stone") );
    document->deleteNode(node, removeChildren);
}

/**
  undo delete node command
*/
void DeleteNodeCommand::undo(){
    if (removeChildren == false){
        foreach(Go::NodePtr child, node->childNodes){
            Go::NodeList::iterator iter = qFind(parentNode->childNodes.begin(), parentNode->childNodes.end(), child);
            if (iter != parentNode->childNodes.end()){
                (*iter)->setParent(node);
                parentNode->childNodes.erase(iter);
            }
        }
    }
    document->addNode(parentNode, node, pos);


}
