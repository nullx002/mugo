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
#include <boost/weak_ptr.hpp>

namespace go{

class node;
class informationNode;
typedef boost::shared_ptr<node> nodePtr;
typedef boost::weak_ptr<node> nodeWPtr;
typedef QList<nodePtr> nodeList;
typedef boost::shared_ptr<informationNode> informationPtr;
typedef QList<informationPtr> informationList;


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
    enum eType{eCross, eCircle, eSquare, eTriangle, eCharacter, eBlackTerritory, eWhiteTerritory, eDim, eSelect};

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

typedef QList<mark>  markList;
typedef QList<stone> stoneList;
class data;
class informationNode;

class node{
    Q_DECLARE_TR_FUNCTIONS(go::node)

public:
    enum eAnnotation{
        eNoAnnotation = 0,
        eHotspot = 1,
    };
    enum eMoveAnnotation{
        eGoodMove = 1,
        eVeryGoodMove,
        eBadMove ,
        eVeryBadMove,
        eDoubtfulMove,
        eInterestingMove,
    };
    enum eNodeAnnotation{
        eEven = 1,
        eGoodForBlack,
        eVeryGoodForBlack,
        eGoodForWhite,
        eVeryGoodForWhite,
        eUnclear,
    };

    node();
    explicit node(nodePtr parent);
    virtual ~node(){  clear();  }

    void clear();
    nodePtr parent(){ return parent_.lock(); }
    const nodePtr parent() const{ return parent_.lock(); }

    int getX() const{ return position.x; }
    int getY() const{ return position.y; }

    void setX(int x){ position.x = x; }
    void setY(int y){ position.y = y; }

    virtual bool isStone() const{ return isBlack() || isWhite(); }
    bool isBlack() const{ return color == black; }
    bool isWhite() const{ return color == white; }
    bool isPass() const;

    void setColor(go::color c){ color = c; }

    virtual QString nodeName() const{ return name; }
    virtual QString toString() const;

//protected:
    // node
    nodeWPtr  parent_;
    nodeList  childNodes;
    QString   name;
    markList  marks;
    markList  blackTerritories;
    markList  whiteTerritories;
    markList  dims;
    stoneList blackStones;
    stoneList whiteStones;
    stoneList emptyStones;
    int annotation;
    int moveAnnotation;
    int nodeAnnotation;
    QString comment;
    point position;
    go::color color;
    go::color nextColor;
    int   moveNumber;
};


class informationNode : public node{
    Q_DECLARE_TR_FUNCTIONS(go::informationNode)
//    static QString tr(const char* source, const char* disam, int n){
//        QCoreApplication::translate("informationNodesource
//    }

public:
    explicit informationNode(nodePtr parent=nodePtr());
    ~informationNode();

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

    data();

    void clear();

    informationList rootList;
    informationPtr root;
};



class fileBase{
public:
    fileBase() : codec(NULL){}
    virtual ~fileBase(){}

    virtual bool read(const QString& fname, QTextCodec* codec, bool guessCodec);
    virtual bool read(const QByteArray& bytes, QTextCodec* defaultCodec, bool guessCodec);
    virtual bool readStream(QString::iterator& first, QString::iterator last) = 0;

    virtual bool save(const QString& fname, QTextCodec* codec);
    virtual bool saveStream(QTextStream& stream) = 0;

    virtual QTextCodec* getCodec(const QByteArray&) const{ return NULL; }

    virtual bool get(go::data& data) const = 0;
    virtual bool set(const go::data& data) = 0;

    QString readLine(QString::iterator& first, QString::iterator& last);

    QTextCodec* codec;
};



nodePtr createBlackNode(nodePtr parent);
nodePtr createBlackNode(nodePtr parent, int x, int y);
nodePtr createWhiteNode(nodePtr parent);
nodePtr createWhiteNode(nodePtr parent, int x, int y);


}



#endif
