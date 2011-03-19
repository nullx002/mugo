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
#ifndef __sgf_h__
#define __sgf_h__

#include "godata.h"

namespace Go{


/**
  read/write sgf format.
*/
class Sgf : public FileBase{
public:
    class Node;
    typedef boost::shared_ptr<Node> NodePtr;
    typedef QList<NodePtr> NodeList;
    typedef QMap<QString, QStringList> PropertyType;

    class Node{
        public:
            bool get(Go::NodePtr& node) const;
            bool set(const Go::NodePtr& node);

            QStringList toStringList() const;

            PropertyType properties;
            NodeList childNodes;

        protected:
            bool get(const QString& key, const QStringList& values, Go::NodePtr& node) const;
            Go::GameInformationPtr getInformation(Go::NodePtr& node) const;
            void addMark(Go::MarkList& markList, const QStringList& values) const;
            void addMark(Go::MarkList& markList, const QStringList& values, Go::Mark::Type type) const;
            void addStone(Go::StoneList& stoneList, const QString& key, const QStringList& values) const;
            void addLine(Go::LineList& lineList, const QStringList& values, Line::Type type) const;

            bool set();
            bool set(const Go::MarkList& markList);
            bool set(const Go::StoneList& stoneList);
            bool set(const Go::LineList& stoneList);
    };

    Sgf() : lineWidth(50){
    }

    // read
    virtual bool readStream(QString::iterator& first, QString::iterator last);
    virtual QTextCodec* guessCodec(const QByteArray&) const;

    // save
    virtual bool saveStream(QTextStream& stream);

    // sgf to internal data
    virtual bool get(Go::NodeList& gameList) const;

    // internal data to sgf
    virtual bool set(Go::NodeList& gameList);

    NodeList gameList;
    int lineWidth;

protected:
    // read
    bool readBranch(QString::iterator& first, QString::iterator last, NodePtr& node);
    bool readNode(QString::iterator& first, QString::iterator last, NodePtr& n, int& pt);
    bool readNodeKey(QString::iterator& first, QString::iterator last, QString& key);
    bool readNodeValues(QString::iterator& first, QString::iterator last, QStringList& values);
    bool readNodeValue(QString::iterator& first, QString::iterator last, QString& value);

    // sgf to internal data
    Go::NodePtr get(NodePtr& sgfNode, Go::NodePtr& goNode) const;

    // internal data to sgf
    bool set(NodePtr& sgfNode, Go::NodePtr goNode);
    void writeNode(QTextStream& stream, QString& s, NodePtr node);

    static bool positionToInt(const QString& pos, int& x, int& y, QString* str=NULL);
    static bool positionToIntList(const QString& pos, QList<int>& x, QList<int>& y);
    static bool positionToIntArea(const QString& pos, QList<int>& x, QList<int>& y);
    static QString positionToString(int x, int y, const QString* s=NULL);
    static QString positionToString(const Go::Point& p, const QString* s=NULL);
    static QString positionToString(int x1, int y1, int x2, int y2);
    static QString positionToString(const Go::Point& p1, const Go::Point& p2);

#if 0
    enum NodeType{ unknown, root, gameInformation, branch, black, white };

    class Node;
    typedef boost::shared_ptr<Node> NodePtr;
    typedef QList<NodePtr> NodeList;
    typedef QMap<QString, QStringList> PropertyType;

    class Node{
        public:
            Node() : NodeType(unknown){}

            QString toString() const;

            void clear(){
                childNodes.clear();
                properties.clear();
            }

            NodeList& getChildNodes(){ return childNodes; }
            const NodeList& getChildNodes() const{ return childNodes; }

            void setNodeType(NodeType type){ nodeType = type; }
            NodeType getNodeType() const{ return nodeType; }

            bool setProperty(const QString& key, const QStringList& values);
            PropertyType& getProperty(){ return properties; }
            const PropertyType& getProperty() const{ return properties; }

            bool get(Go::NodePtr n) const;
            bool get(Go::NodePtr n, const QString& key, const QStringList& values) const;

            bool set(const Go::NodePtr n);
            bool set(const Go::MarkList& markList);
            bool set(const Go::StoneList& markList);

        private:
            void addMark(Go::MarkList& markList, const QStringList& values, const char* str=NULL) const;
            void addMark(Go::MarkList& markList, const QStringList& values, Mark::Type type) const;
            void addStone(Go::StoneList& stoneList, const QString& key, const QStringList& values) const;

            NodeList childNodes;
            NodeType nodeType;
            PropertyType properties;
    };


    Sgf(){
    }

    virtual bool readStream(QString::iterator& first, QString::iterator last);
    virtual bool saveStream(QTextStream& stream);
    virtual QTextCodec* getCodec(const QByteArray&) const;

//    virtual bool get(Go::data& data) const;
//    virtual bool set(const go::data& data);
//    virtual bool set(const go::informationPtr& info);

    static bool pointToInt(const QString& pos, int& x, int& y, QString* str=NULL);
    static bool pointToIntList(const QString& pos, QList<int>& x, QList<int>& y);
    static QString pointToString(int x, int y, const QString* s=NULL);
    static QString pointToString(const Go::Point& p, const QString* s=NULL);

protected:
    bool readBranch(QString::iterator& first, QString::iterator last, nodePtr n);
    bool readNode(QString::iterator& first, QString::iterator last, nodePtr& n, int& pt);
    bool readNodeKey(QString::iterator& first, QString::iterator last, QString& key);
    bool readNodeValues(QString::iterator& first, QString::iterator last, QStringList& values);
    bool readNodeValue(QString::iterator& first, QString::iterator last, QString& value);

    bool writeNode(QTextStream& stream, QString& s, const nodePtr& n);

    go::nodePtr get(const nodePtr& sgfNode, go::nodePtr& goNode) const;
    bool set(nodePtr& sgfNode, const go::nodePtr& goNode);

    nodeList rootList;
#endif
};


}



#endif
