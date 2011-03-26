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
#ifndef SGF_H
#define SGF_H


#include "filebase.h"


namespace Go{


/**
  reading/writing sgf file.
*/
class Sgf : public FileBase{
public:
    // constructor
    Sgf(NodeList& gameList);

protected:
    // read
    virtual QTextCodec* guessCodec(const QByteArray& ba);
    virtual bool parse(const QString& str);

    // write
    virtual bool write(QTextStream& str);
    virtual bool write(QTextStream& str, const NodePtr& node);
    virtual bool writeRootInformation(QTextStream& str, const InformationPtr& info);
    virtual bool writeGameInformation(QTextStream& str, const InformationPtr& info);
    virtual bool writeNode(QTextStream& str, const NodePtr& node);

private:
    bool parse(QString::const_iterator first, QString::const_iterator last);
    bool parseBranch(QString::const_iterator& first, QString::const_iterator last, InformationPtr& info, NodePtr parent);
    bool parseNode(QString::const_iterator& first, QString::const_iterator last, InformationPtr& info, NodePtr& node);
    bool parseNodeName(QString::const_iterator& first, QString::const_iterator last, QString& key);
    bool parseNodeValueList(QString::const_iterator& first, QString::const_iterator last, QStringList& valueList);
    bool parseNodeValue(QString::const_iterator& first, QString::const_iterator last, QString& value);
    bool skipSpace(QString::const_iterator& first, QString::const_iterator last);
    bool addPropertyToNode(InformationPtr& info, NodePtr& node, const QString& key, const QStringList& valueList);
    void parseNumber(const QString& value, int& v1, int& v2);
    void parseMove(const QString& value, int& x, int& y);
    void parseMove(const QString& value, QList<QPoint>& posList);
    QString positionToSgfProperty(int x, int y);
};


}


#endif // SGF_H
