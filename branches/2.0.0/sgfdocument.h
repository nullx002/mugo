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
#ifndef SGFDOCUMENT_H
#define SGFDOCUMENT_H

#include "document.h"
#include "godata.h"


class SgfDocument : public Document
{
    Q_OBJECT
public:
    SgfDocument(QTextCodec* codec, QObject* parent=NULL);
    virtual ~SgfDocument(){};

    // open
    bool open(const QString& fname, bool guessCodec);
    bool read(const QString& docName, const QByteArray& byets, bool guessCodec);
    bool set(const Go::FileBase& fbase);

    // save
    bool save(const QString& fname);

    // operate without command
    void addNode(Go::NodePtr parentNode, Go::NodePtr node, int index);
    void deleteNode(const Go::NodePtr node, bool removeChildren);
    bool addGame(const Go::NodePtr& game);
    bool deleteGame(const Go::NodePtr& game);
    bool moveUp(const Go::NodePtr& game);
    bool moveDown(const Go::NodePtr& game);

    void modifyNode(Go::NodePtr& node){ setDirty(); emit nodeModified(node); }

    Go::NodeList gameList;
    int lineWidth;

signals:
    void nodeAdded(Go::NodePtr node);
    void nodeDeleted(Go::NodePtr node, bool removeChildren);
    void nodeModified(Go::NodePtr node);
    void gameAdded(Go::NodePtr game, int index);
    void gameDeleted(Go::NodePtr game, int index);
    void gameMoved(Go::NodePtr game, int from, int to);
};

#endif // SGFDOCUMENT_H
