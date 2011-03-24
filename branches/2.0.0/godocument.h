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
    void nodeAdded(Go::NodePtr node);
    void nodeDeleted(Go::NodePtr node);
    void nodeModified(Go::NodePtr node);

public slots:
    void addNode(const Go::NodePtr parent, Go::NodePtr node, int index=-1);
    void deleteNode(Go::NodePtr node, bool removeChildren=true);
    void modifyNode(Go::NodePtr node);

public:
    Go::NodeList gameList;
};

#endif // SGFDOCUMENT_H
