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
#include "godocument.h"

/**
    Constructs sgf document
*/
GoDocument::GoDocument(QObject* parent)
    : Document(parent)
{
}

/**
    Constructs sgf document
*/
GoDocument::GoDocument(Go::NodeList& gameList_, QObject* parent)
    : Document(parent)
    , gameList(gameList_)
{
}

/**
    Constructs sgf document
*/
GoDocument::GoDocument(int xsize, int ysize, qreal komi, int handicap, QObject* parent)
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
  open the document from file
  have to be overridden if add new format support
*/
bool GoDocument::open(const QString& fname, QTextCodec* codec, bool guessCodec){
    setFileInfo(QFileInfo(fname));
    setCodec(codec);
    return true;
}

/**
  save the document to file
  have to be overridden if add new format support
*/
bool GoDocument::save(const QString& fname, QTextCodec* codec){
    setFileInfo(QFileInfo(fname));
    setCodec(codec);
    setDirty(false);
    return true;
}


/**
  modify document
*/
void GoDocument::modifyDocument(){
    setDirty();
    emit documentModified();
}

/**
  add game
*/
void GoDocument::addGame(Go::NodePtr game){
    gameList.push_back(game);

    modifyDocument();
    emit gameAdded(game);
}

/**
  add game list
*/
void GoDocument::addGameList(const Go::NodeList& gameList){
    this->gameList.append(gameList);

    modifyDocument();
    foreach(const Go::NodePtr& game, gameList)
        emit gameAdded(game);
}

/**
  delete game
*/
bool GoDocument::deleteGame(Go::NodePtr game){
    if (gameList.removeOne(game)){
        modifyDocument();
        emit gameDeleted(game);
        return true;
    }
    return false;
}

/**
  delete game list
*/
bool GoDocument::deleteGameList(const Go::NodeList& gameList){
    bool deleted = false;
    foreach(const Go::NodePtr& game, gameList){
        if (this->gameList.removeOne(game)){
            deleted = true;
            emit gameDeleted(game);
        }
    }

    if (deleted)
        modifyDocument();
    return deleted;
}

/**
  modify node
*/
void GoDocument::modifyNode(const Go::NodePtr& node){
    modifyDocument();
    emit nodeModified(node);
}

/**
  add node
*/
void GoDocument::addNode(Go::NodePtr parent, Go::NodePtr node, int index){
    if (index < 0)
        parent->children().push_back(node);
    else
        parent->children().insert(parent->children().begin()+index, node);
    node->setParent(parent);

    modifyDocument();
    emit nodeAdded(node);
}

/**
  delete node
*/
bool  GoDocument::deleteNode(Go::NodePtr node, bool removeChildren){
    Go::NodePtr parent = node->parent();
    if (!parent)
        return false;

    if (removeChildren)
        parent->children().removeOne(node);
    else{
        Go::NodeList::iterator before = qFind(parent->children().begin(), parent->children().end(), node);
        foreach(Go::NodePtr child, node->children()){
            child->setParent(parent);
            parent->children().insert(before, child);
        }
    }

    modifyDocument();
    emit nodeDeleted(node);

    return true;
}
