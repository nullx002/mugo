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
#include "sgfdocument.h"
#include "command.h"

SgfDocument::SgfDocument(QObject* parent)
    : Document(parent)
{
    Go::NodePtr node = Go::createInformationNode(Go::NodePtr());
    gameList.push_back(node);
}

/**
  command
  add node
*/
void SgfDocument::addNodeCommand(Go::NodePtr parentNode, Go::NodePtr node){
    undoStack->push( new AddNodeCommand(this, parentNode, node) );
}

/**
  add node
*/
void SgfDocument::addNode(Go::NodePtr parentNode, Go::NodePtr node){
    parentNode->childNodes.push_back(node);
    emit nodeAdded(node);
}
