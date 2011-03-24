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
#include "sgf.h"

/**
  Constructs of sgf document
*/
SgfDocument::SgfDocument(QObject* parent)
    : GoDocument(parent)
{
}

/**
  Constructs of sgf document
*/
SgfDocument::SgfDocument(Go::NodeList& gameList, QObject* parent)
    : GoDocument(gameList, parent)
{
}

/**
  Constructs of sgf document
*/
SgfDocument::SgfDocument(int xsize, int ysize, qreal komi, int handicap, QObject* parent)
    : GoDocument(xsize, ysize, komi, handicap, parent)
{
}

/**
  open sgf document from file
*/
bool SgfDocument::open(const QString& fname, QTextCodec* codec, bool guessCodec){
    Go::NodeList gl;
    Go::Sgf sgf(gl);
    if (sgf.load(fname, codec, guessCodec) == false)
        return false;

    gameList = gl;
    return GoDocument::open(fname, codec, guessCodec);
}

/**
  save sgf document to file
*/
bool SgfDocument::save(const QString& fname, QTextCodec* codec){
    QTextCodec* c = codec ? codec : this->codec();
    Go::Sgf sgf(gameList);
    if (sgf.save(fname, c) == false)
        return false;

    return GoDocument::save(fname, c);
}
