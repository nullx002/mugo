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
#ifndef GODOCUMENT_H
#define GODOCUMENT_H

#include "document.h"
#include "godata.h"


/**
  base class of go document classes
*/
class GoDocument : public Document
{
    Q_OBJECT
public:
    // constructor
    GoDocument(QObject* parent = 0);
    GoDocument(Go::NodeList& gameList, QObject* parent = 0);
    GoDocument(int xsize=19, int ysize=19, qreal komi=6.5, int handicap=0, QObject* parent = 0);

    // open
    virtual bool open(const QString& fname, QTextCodec* codec, bool guessCodec);

    // save
    virtual bool save(const QString& fname, QTextCodec* codec=NULL);

signals:
    void saved();
    void documentModified();
    void informationChanged(const Go::NodePtr&, const Go::InformationPtr&);
    void gameAdded(const Go::NodePtr& game);
    void gameDeleted(const Go::NodePtr& game, int index);
    void gameMovedUp(const Go::NodePtr& game);
    void gameMovedDown(const Go::NodePtr& game);
    void nodeModified(const Go::NodePtr& game, const Go::NodePtr& node);
    void nodeAdded(const Go::NodePtr& game, const Go::NodePtr& node);
    void nodeDeleted(const Go::NodePtr& game, const Go::NodePtr& node, bool removeChildren);

public slots:
    void modifyDocument();
    void setInformation(Go::NodePtr node, Go::InformationPtr info);
    void addGame(Go::NodePtr game);
    void addGameList(const Go::NodeList& gameList);
    bool deleteGame(Go::NodePtr game);
    bool deleteGameList(const Go::NodeList& gameList);
    void moveUpGame(const Go::NodePtr& game);
    void moveDownGame(const Go::NodePtr& game);
    void modifyNode(const Go::NodePtr& game, const Go::NodePtr& node);
    void addNode(const Go::NodePtr& game, Go::NodePtr parent, Go::NodePtr node, int index=-1);
    bool deleteNode(const Go::NodePtr& game, Go::NodePtr node, bool removeChildren=true);

public:
    Go::NodeList gameList;
};

#endif // SGFDOCUMENT_H
