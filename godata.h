#ifndef __godata_h__
#define __godata_h__

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QLinkedList>
#include <QMap>
#include <QTextCodec>

namespace go{


QString x_str(int size, int x);
QString y_str(int size, int x);


class point{
public:
    point() : x(-1), y(-1){}
    point(int x_, int y_) : x(x_), y(y_){}

    int x;
    int y;
};

inline
bool operator ==(const point& a, const point& b){
    return a.x == b.x && a.y == b.y;
}

class mark{
public:
    enum eType{eCross, eCircle, eSquare, eTriangle, eCharacter, eBlackTerritory, eWhiteTerritory};

    mark(const point& p_, eType t_) : p(p_), t(t_){ setCharacter(); }
    mark(const point& p_, const QString& s_) : p(p_), t(eCharacter), s(s_){}
    mark(int x, int y, eType t_) : p(x, y), t(t_){ setCharacter(); }
    mark(int x, int y, const QString& s_) : p(x, y), t(eCharacter), s(s_){}

    void setCharacter(){
        switch(t){
            case eCross:
                s = "×";
                break;
            case eCircle:
                s = "○";
                break;
            case eSquare:
                s = "□";
                break;
            case eTriangle:
                s = "△";
                break;
            case eCharacter:
                break;
            case eBlackTerritory:
            case eWhiteTerritory:
                s = "■";
                break;
        };
    }

    point   p;
    eType   t;
    QString s;
};

class stone{
public:
    enum color{ empty, black, white };

    stone(const point& p_, color c_) : p(p_), c(c_){}
    stone(int x, int y, color c_) : p(x, y), c(c_){}

    bool isBlack() const{ return c == black; }
    bool isWhite() const{ return c == white; }
    bool isEmpty() const{ return c == empty; }

    point p;
    color c;
};

typedef QLinkedList<mark>  markList;
typedef QLinkedList<stone> stoneList;

class data;
class node;
typedef QLinkedList<node*> nodeList;


class node{
public:
    explicit node(data* data_);
    explicit node(node* parent);
    virtual ~node(){  clear();  }

    void clear();

/*
    node* getParent(){ return parent; }
    const node* getParent() const{ return parent; }

    const nodeList& getChildNodes() const{ return childNodes; }
    nodeList& getChildNodes(){ return childNodes; }

    const string& getComment() const{ return comment; }
    void setComment(const string& comment){ this->comment = comment; }
*/

    int getX() const{ return position.x; }
    int getY() const{ return position.y; }

    void setX(int x){ position.x = x; }
    void setY(int y){ position.y = y; }

    virtual bool isBlack() const{ return false; }
    virtual bool isWhite() const{ return false; }

    virtual QString nodeName() const{ static const QString str = "Other"; return str; }
    virtual QString toString() const;

//protected:
    // node
    data* goData;
    node* parent;
    nodeList childNodes;
    markList  crosses;
    markList  triangles;
    markList  circles;
    markList  squares;
    markList  characters;
    markList  blackTerritories;
    markList  whiteTerritories;
    stoneList stones;
    QString comment;
    point position;
};


class informationNode : public node{
public:
    explicit informationNode(data* data_) : node(data_){ initialize(); }
    explicit informationNode(node* parent) : node(parent){ initialize(); }

    virtual bool isWhite() const{ return true; }

    virtual QString nodeName() const{ static const QString str = "GameInfo"; return str; }

    void initialize();

    // rule
    int    size;
    double komi;
    int    handicap;
    QString time;
    QString overTime;

    // player
    QString whitePlayer;
    QString whiteRank;
    QString whiteTeam;
    QString blackPlayer;
    QString blackRank;
    QString blackTeam;
    QString result;

    // game info
    QString gameName;
    QString date;
    QString round;
    QString place;
    QString event;

    // other info
    QString rule;
    QString annotation;
    QString source;
    QString gameComment;
    QString copyright;
    QString user;
    QString opening;
};


class stoneNode : public node{
public:
    virtual QString nodeName() const;

    bool isPass() const{ return position.x == -1 && position.y == -1; }

protected:
    stoneNode(node* parent) : node(parent){}
};

class blackNode : public stoneNode{
public:
    explicit blackNode(node* parent) : stoneNode(parent){}
    explicit blackNode(node* parent, int x, int y) : stoneNode(parent){
        setX(x);
        setY(y);
    }

    virtual bool isBlack() const { return true; }
};

class whiteNode : public stoneNode{
public:
    explicit whiteNode(node* parent) : stoneNode(parent){}
    explicit whiteNode(node* parent, int x, int y) : stoneNode(parent){
        setX(x);
        setY(y);
    }

    virtual bool isWhite() const{ return true; }
};


class data{
public:
    enum eRule{eJapanese, eChinese};

    data() : root(this), size(19), komi(6.5){}

    void clear();

    informationNode root;
    int    size;
    double komi;
};



class fileBase{
public:
    fileBase(){}
    virtual ~fileBase(){}

    virtual bool read(const QString& fname, QTextCodec* codec);
    virtual bool readStream(QString::iterator& first, QString::iterator last) = 0;

    virtual bool save(const QString& fname, QTextCodec* codec);
    virtual bool saveStream(QTextStream& stream) = 0;

    virtual bool get(go::data& data) const = 0;
    virtual bool set(const go::data& data) = 0;
};




class sgf : public fileBase{
public:
    enum eNodeType{ eUnknown, eRoot, eGameInformation, eBranch, eBlack, eWhite };

    class node;
//    typedef boost::shared_ptr<node> nodePtr;
    typedef QLinkedList<node*> nodeList;
    typedef QMap<QString, QStringList> propertyType;

    class node{
        public:

            node() : nodeType(eUnknown), x(-1), y(-1){}
            ~node();

            void clear(){
                childNodes.clear();
                property.clear();
            }

            nodeList& getChildNodes(){ return childNodes; }
            const nodeList& getChildNodes() const{ return childNodes; }

            void setNodeType(eNodeType type){ nodeType = type; }
            eNodeType getNodeType() const{ return nodeType; }

            bool setProperty(const QString& key, const QStringList& values);

            bool get(go::node& n) const;
            bool get(go::node& n, const QString& key, const QStringList& values) const;

            bool set(const go::node& n);
            bool set(const go::markList& markList);
            bool set(const go::stoneList& markList);

            QString toString() const;

        private:
            void setPosition(eNodeType type, const QString& pos);
            bool getPosition(const QString& pos, int& x, int& y, QString* str=NULL) const;
            void addMark(go::markList& markList, const QStringList& values, const char* str=NULL) const;
            void addMark(go::markList& markList, const QStringList& values, mark::eType type) const;
            void addStone(go::stoneList& stoneList, const QString& key, const QStringList& values) const;

            nodeList childNodes;
            eNodeType nodeType;
            propertyType property;
            int x, y;
    };


    sgf(){
        root.setNodeType(eRoot);
    }

    node& getRoot(){ return root; }

    virtual bool readStream(QString::iterator& first, QString::iterator last);
    virtual bool saveStream(QTextStream& stream);

    virtual bool get(go::data& data) const;
    virtual bool set(const go::data& data);

    static QString pointToString(int x, int y, const QString* s=NULL);
    static QString pointToString(const go::point& p, const QString* s=NULL);

protected:
    bool readBranch(QString::iterator& first, QString::iterator last, node& n);
    bool readNode(QString::iterator& first, QString::iterator last, node& n);
    bool readNode2(QString::iterator& first, QString::iterator last, node& n);
    bool readNodeKey(QString::iterator& first, QString::iterator last, QString& key);
    bool readNodeValues(QString::iterator& first, QString::iterator last, QStringList& values);
    bool readNodeValue(QString::iterator& first, QString::iterator last, QString& value);

    bool writeNode(QTextStream& stream, const node& n);

    go::node* get(const node& sgfNode, go::node* outNode) const;
    bool set(sgf::node* node, const go::node* node2);

    node root;
};


}



#endif
