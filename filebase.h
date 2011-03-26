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
#ifndef FILEBASE_H
#define FILEBASE_H


#include "godata.h"

class QFileInfo;
class QFile;
class QTextStream;


namespace Go{


/**
  base class of reading/writing
*/
class FileBase{
public:
    // constructor, destructor
    FileBase(Go::NodeList& gameList);
    virtual ~FileBase(){}

    // read
    virtual bool load(const QFileInfo& fileInfo, QTextCodec* defaultCodec, bool guessCodec=true);
    virtual bool load(const QString& filePath, QTextCodec* defaultCodec, bool guessCodec=true);
    virtual bool load(QFile& file, QTextCodec* defaultCodec, bool guessCodec=true);
    virtual bool load(const QString& str);

    // write
    virtual bool save(const QFileInfo& fileInfo, QTextCodec* codec = NULL);
    virtual bool save(const QString& filePath, QTextCodec* codec = NULL);
    virtual bool save(QFile& file, QTextCodec* codec = NULL);
    virtual bool save(QTextStream& str);

protected:
    // read
    virtual QTextCodec* guessCodec(const QByteArray& ba);
    virtual bool parse(const QString& str) = 0;

    // write
    virtual bool write(QTextStream& str) = 0;

    InformationPtr gameInformation(NodePtr& node);

    QTextCodec* defaultCodec;
    Go::NodeList& gameList;
};


}


#endif // FILEBASE_H
