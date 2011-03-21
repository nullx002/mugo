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
#include "sgfdocument.h"

/**
    Constructs sgf document
*/
SgfDocument::SgfDocument(int xsize, int ysize, qreal komi, int handicap, QObject* parent)
    : Document(parent)
{
    // create game information
    Go::InformationPtr info(new Go::Information);
    info->setXSize(xsize);
    info->setYSize(ysize);
    info->setKomi(komi);
    info->setHandicap(handicap);

    // create game node
    Go::NodePtr game(new Go::Node);
    game->setInformation(info);
    game->setNextColor(Go::eBlack);

    // add game into game list
    gameList.push_back(game);
}

/**
    Constructs sgf document
*/
SgfDocument::SgfDocument(Go::NodeList& gameList_, QObject* parent)
    : Document(parent)
    , gameList(gameList_)
{
}

/**
  add node
*/
void SgfDocument::addNode(Go::NodePtr parent, Go::NodePtr node, int index){
    if (index < 0)
        parent->children().push_back(node);
    else
        parent->children().insert(parent->children().begin()+index, node);
    node->setParent(parent);

    setDirty();
    emit nodeAdded(node);
}

/**
  delete node
*/
void SgfDocument::deleteNode(Go::NodePtr node, bool removeChildren){
    Go::NodePtr parent = node->parent();
    if (!parent)
        return;

    if (removeChildren)
        parent->children().removeOne(node);
    else{
        Go::NodeList::iterator before = qFind(parent->children().begin(), parent->children().end(), node);
        foreach(Go::NodePtr child, node->children()){
            child->setParent(parent);
            parent->children().insert(before, child);
        }
    }

    setDirty();
    emit nodeDeleted(node);
}

/**
  modify node
*/
void SgfDocument::modifyNode(Go::NodePtr node){
    setDirty();
    emit nodeModified(node);
}
