#ifndef __godata_h__
#define __godata_h__

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QLinkedList>
#include <QMap>
#include <QTextCodec>
#include <boost/shared_ptr.hpp>

namespace go{


enum color{ empty=0, black=1, white=2, blackTerritory=4, whiteTerritory=8, dame=16 };

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

    mark(const point& p_, eType t_) : p(p_), t(t_){}
    mark(const point& p_, const QString& s_) : p(p_), t(eCharacter), s(s_){}
    mark(int x, int y, eType t_) : p(x, y), t(t_){}
    mark(int x, int y, const QString& s_) : p(x, y), t(eCharacter), s(s_){}

    point   p;
    eType   t;
    QString s;
};

class stone{
public:
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


class node{
    Q_DECLARE_TR_FUNCTIONS(go::node)

public:
    typedef boost::shared_ptr<node> nodePtr;
    typedef QLinkedList<nodePtr> nodeList;

    enum eAnnotation{
        eNoAnnotation = 0x0000,
        eGoodMove = 0x0001,
        eVeryGoodMove = 0x0002,
        eBadMove = 0x0004,
        eVeryBadMove = 0x0008,
        eDoubtfulMove = 0x0010,
        eInterestingMove = 0x0020,
        eEven = 0x0040,
        eGoodForBlack = 0x0080,
        eVeryGoodForBlack = 0x0100,
        eGoodForWhite = 0x0200,
        eVeryGoodForWhite = 0x0400,
        eUnclear = 0x0800,
        eHotspot = 0x1000,
    };

    explicit node(data* data_);
    explicit node(nodePtr parent);
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

    virtual bool isStone() const{ return black || white; }
    bool isBlack() const{ return black; }
    bool isWhite() const{ return white; }
    bool isPass() const;

    void setBlack(){ black = true; white = false; }
    void setWhite(){ black = false; white = true; }
    void setEmpty(){ black = false; white = false; }

    virtual QString nodeName() const{ return name; }
    virtual QString toString() const;

//protected:
    // node
    data* goData;
    nodePtr parent;
    nodeList  childNodes;
    QString   name;
    markList  crosses;
    markList  triangles;
    markList  circles;
    markList  squares;
    markList  characters;
    markList  blackTerritories;
    markList  whiteTerritories;
    stoneList stones;
    int annotation;
    QString comment;
    point position;
    bool  black;
    bool  white;
    int   moveNumber;
};


class informationNode : public node{
    Q_DECLARE_TR_FUNCTIONS(go::informationNode)
//    static QString tr(const char* source, const char* disam, int n){
//        QCoreApplication::translate("informationNodesource
//    }

public:
    explicit informationNode(data* data_) : node(data_){ initialize(); }
    explicit informationNode(nodePtr parent) : node(parent){ initialize(); }

    virtual bool isStone() const{ return false; }

    virtual QString nodeName() const;

    void initialize();

    // rule
    int    xsize;
    int    ysize;
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


class data{
public:
    enum eRule{eJapanese, eChinese};

    data() : root(new informationNode(this)){}

    void clear();

    boost::shared_ptr<informationNode> root;
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

    QTextCodec* codec;
};



typedef boost::shared_ptr<node> nodePtr;
typedef boost::shared_ptr<informationNode> informationPtr;
typedef node::nodeList nodeList;



nodePtr createBlackNode(nodePtr parent);
nodePtr createBlackNode(nodePtr parent, int x, int y);
nodePtr createWhiteNode(nodePtr parent);
nodePtr createWhiteNode(nodePtr parent, int x, int y);


}



#endif
