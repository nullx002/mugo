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
#include <QFileInfo>
#include <boost/scoped_ptr.hpp>
#include "sgfdocument.h"
#include "sgf.h"
#include "command.h"

SgfDocument::SgfDocument(QTextCodec* codec, QObject* parent)
    : Document(codec, parent)
    , lineWidth(50)
{
    Go::NodePtr node = Go::createInformationNode(Go::NodePtr());
    gameList.push_back(node);
}

/**
  open
*/
bool SgfDocument::open(const QString& fname, bool guessCodec){
    QFileInfo fi(fname);
    boost::scoped_ptr<Go::FileBase> fileBase;
/*
    if (fi.suffix().compare("ugf", Qt::CaseInsensitive) == 0)
    else if (fi.suffix().compare("ngf", Qt::CaseInsensitive) == 0)
    else if (fi.suffix().compare("gib", Qt::CaseInsensitive) == 0)
    else
*/
        fileBase.reset( new Go::Sgf );

    if (fileBase->read(fname, codec, guessCodec) == false)
        return false;

    set(*fileBase);
    docName  = fi.fileName();
    fileName = fname;

    return true;
}

/**
  read
*/
bool SgfDocument::read(const QString& docName_, const QByteArray& bytes, bool guessCodec){
    Go::Sgf sgf;
    if( sgf.read(bytes, codec, guessCodec) == false)
        return false;

    set(sgf);
    docName = docName_;

    return true;
}

/**
  set
*/
bool SgfDocument::set(const Go::FileBase& fbase){
    gameList.clear();
    fbase.get(gameList);
    if (gameList.empty())
        return false;

    codec = fbase.codec;
    docName.clear();
    fileName.clear();
    setDirty(false);

    return true;
}

/**
  save
*/
bool SgfDocument::save(const QString& fname){
    Go::Sgf sgf;
    sgf.lineWidth = lineWidth;
    if (sgf.set(gameList) == false)
        return false;
    if (sgf.save(fname, codec) == false)
        return false;

    QFileInfo fi(fname);
    docName  = fi.fileName();
    fileName = fname;
    setDirty(false);

    return true;
}

/**
  add node
*/
void SgfDocument::addNode(Go::NodePtr parentNode, Go::NodePtr node, int index){
    if (index == -1)
        parentNode->childNodes.push_back(node);
    else
        parentNode->childNodes.insert(index, node);
    node->setParent(parentNode);

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

    Go::NodeList::iterator iter = qFind(parent->childNodes.begin(), parent->childNodes.end(), node);

    if (removeChildren == false){
        foreach(Go::NodePtr child, node->childNodes){
            child->setParent(parent);
            iter = parent->childNodes.insert(iter, child);
            ++iter;
        }
    }
    parent->childNodes.erase(iter);

    setDirty();
    emit nodeDeleted(node, removeChildren);
}

/**
  add game into gamelist
*/
bool SgfDocument::addGame(const Go::NodePtr& game, int index){
    if (index == -1){
        gameList.push_back(game);
        index = gameList.size() - 1;
    }
    else
        gameList.insert(index, game);

    setDirty();

    emit  gameAdded(game, index);

    return true;
}

/**
  delete game from gamelist
*/
bool SgfDocument::deleteGame(const Go::NodePtr& game){
    int index = gameList.indexOf(game);
    if (index < 0)
        return false;

    gameList.removeAt(index);
    setDirty();

    emit gameDeleted(game, index);

    return true;
}

/**
  move up game in gamelist
*/
bool SgfDocument::moveUp(const Go::NodePtr& game){
    int index = gameList.indexOf(game);
    if (index <= 0)
        return false;

    gameList.swap(index-1, index);
    setDirty();

    emit gameMoved(game, index, index-1);

    return true;
}

/**
  move down game in gamelist
*/
bool SgfDocument::moveDown(const Go::NodePtr& game){
    int index = gameList.indexOf(game);
    if (index < 0 || index >= gameList.size()-1)
        return false;

    gameList.swap(index, index+1);
    setDirty();

    emit gameMoved(game, index, index+1);

    return true;
}

/**
  get position string
*/
QString SgfDocument::positionString(Go::NodePtr node, bool showI){
    if (node->isStone() == false)
        return "";
    else if (node->isPass())
        return tr("Pass");

    QString str;
    Go::GameInformationPtr info = node->getInformation();
    int x = node->position.x % 26;
    if (showI == false && x > 7)
        ++x;
    int y = info->ysize - node->position.y;
    str.sprintf("%c%d", 'A'+x, y);
    return str;
}
