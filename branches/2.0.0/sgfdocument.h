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

    // save
    bool save(const QString& fname);

    // command
    void addNodeCommand(Go::NodePtr parentNode, Go::NodePtr node, int index=-1);
    void deleteNodeCommand(Go::NodePtr node, bool removeChildren);

    // operate without command
    void addNode(Go::NodePtr parentNode, Go::NodePtr node, int index);
    void deleteNode(Go::NodePtr node, bool removeChildren);

    Go::NodeList gameList;

signals:
    void nodeAdded(Go::NodePtr node);
    void nodeDeleted(Go::NodePtr node, bool removeChildren);
};

#endif // SGFDOCUMENT_H
