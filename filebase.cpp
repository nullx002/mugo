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
#include <QTextCodec>
#include <QFileInfo>
#include <QTextStream>
#include "filebase.h"


namespace Go{


/**
  Constructor
*/
FileBase::FileBase(Go::NodeList& gameList_)
    : gameList(gameList_)
{
}

/**
  load
*/
bool FileBase::load(const QFileInfo& fileInfo, QTextCodec* defaultCodec, bool guessCodec){
    return load(fileInfo.filePath(), defaultCodec, guessCodec);
}

/**
  load
*/
bool FileBase::load(const QString& filePath, QTextCodec* defaultCodec, bool guessCodec){
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);
    return load(file, defaultCodec, guessCodec);
}

/**
  load
*/
bool FileBase::load(QFile& file, QTextCodec* defaultCodec, bool guessCodec){
    // read all
    QByteArray ba = file.readAll();

    // guess codec.
    // use default codec if codec isn't understood.
    QTextCodec* codec = NULL;
    if (guessCodec)
        codec = this->guessCodec(ba);
    if (codec == NULL)
        codec = defaultCodec;

    // convert byte array to unicode string
    QString str = codec->toUnicode(ba);
    return parse(str);
}

/**
  load
*/
bool FileBase::load(const QString& str){
    return parse(str);
}

/**
  guess codec
  return NULL if codec isn't understood.
*/
QTextCodec* FileBase::guessCodec(const QByteArray&){
    return NULL;
}

/**
  save
*/
bool FileBase::save(const QFileInfo& fileInfo, QTextCodec* codec){
    return save(fileInfo.filePath(), codec);
}

/**
  save
*/
bool FileBase::save(const QString& filePath, QTextCodec* codec){
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;
    return save(file, codec);
}

/**
  save
*/
bool FileBase::save(QFile& file, QTextCodec* codec){
    QTextStream str(&file);
    if (codec)
        str.setCodec(codec);
    return save(str);
}

/**
  save
*/
bool FileBase::save(QTextStream& str){
    return write(str);
}

/**
  if node has information node, return information.
  if node doesn't have information node, create information node and return it.
*/
InformationPtr FileBase::gameInformation(NodePtr& node){
    InformationPtr info = node->information();
    if (info)
        return info;

    info = InformationPtr(new Information);
    node->setInformation(info);
    return info;
}


}
