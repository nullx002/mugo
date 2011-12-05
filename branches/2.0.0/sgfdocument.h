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
#ifndef SGFDOCUMENT_H
#define SGFDOCUMENT_H

#include "godocument.h"


/**
  document class of sgf format
*/
class SgfDocument : public GoDocument
{
    Q_OBJECT
public:
    // constructor
    SgfDocument(QObject* parent = 0);
    SgfDocument(Go::NodeList& gameList, QObject* parent = 0);
    SgfDocument(int xsize, int ysize, qreal komi=6.5, int handicap=0, QObject* parent = 0);

    // open
    virtual bool open(const QString& fname, QTextCodec* codec, bool guessCodec);

    // save
    virtual bool save(const QString& fname, QTextCodec* codec=NULL);

signals:

public slots:

public:
};

#endif // SGFDOCUMENT_H
