#ifndef __sgf_h__
#define __sgf_h__

#include "godata.h"

namespace go{


class sgf : public fileBase{
public:
    enum eNodeType{ eUnknown, eRoot, eGameInformation, eBranch, eBlack, eWhite };

    class node;
    typedef boost::shared_ptr<node> nodePtr;
    typedef QLinkedList<nodePtr> nodeList;
    typedef QMap<QString, QStringList> propertyType;

    class node{
        public:
            node() : nodeType(eUnknown){}

            QString toString() const;

            void clear(){
                childNodes.clear();
                property.clear();
            }

            nodeList& getChildNodes(){ return childNodes; }
            const nodeList& getChildNodes() const{ return childNodes; }

            void setNodeType(eNodeType type){ nodeType = type; }
            eNodeType getNodeType() const{ return nodeType; }

            bool setProperty(const QString& key, const QStringList& values);
            propertyType& getProperty(){ return property; }
            const propertyType& getProperty() const{ return property; }

            bool get(go::nodePtr n) const;
            bool get(go::nodePtr n, const QString& key, const QStringList& values) const;

            bool set(const go::nodePtr n);
            bool set(const go::markList& markList);
            bool set(const go::stoneList& markList);

        private:
            void addMark(go::markList& markList, const QStringList& values, const char* str=NULL) const;
            void addMark(go::markList& markList, const QStringList& values, mark::eType type) const;
            void addStone(go::stoneList& stoneList, const QString& key, const QStringList& values) const;

            nodeList childNodes;
            eNodeType nodeType;
            propertyType property;
    };


    sgf(){
    }

    virtual bool readStream(QString::iterator& first, QString::iterator last);
    virtual bool saveStream(QTextStream& stream);
    virtual QTextCodec* getCodec(const QByteArray&) const;

    virtual bool get(go::data& data) const;
    virtual bool set(const go::data& data);
    virtual bool set(const go::informationPtr& info);

    static bool pointToInt(const QString& pos, int& x, int& y, QString* str=NULL);
    static bool pointToIntList(const QString& pos, QList<int>& x, QList<int>& y);
    static QString pointToString(int x, int y, const QString* s=NULL);
    static QString pointToString(const go::point& p, const QString* s=NULL);

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
};


}



#endif
