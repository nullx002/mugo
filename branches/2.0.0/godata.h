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
#include <QMap>
#include <QTextCodec>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace Go{

class Node;
class GameInformation;
typedef boost::shared_ptr<Node> NodePtr;
typedef boost::weak_ptr<Node> NodeWPtr;
typedef boost::shared_ptr<GameInformation> GameInformationPtr;
typedef QList<NodePtr> NodeList;


enum Color{ empty=0, black=1, white=2, blackTerritory=4, whiteTerritory=8, dame=16 };

/**
    Point
*/
class Point{
public:
    // Constructor
    Point() : x(-1), y(-1){}  // (-1, -1) is pass.
    Point(int x_, int y_) : x(x_), y(y_){}

    void set(int x_, int y_){ x = x_, y = y_; }

    int x;
    int y;
};

inline
bool operator ==(const Point& a, const Point& b){
    return a.x == b.x && a.y == b.y;
}

/**
    Stone
*/
class Stone{
public:
    // Constructor
    Stone(const Point& p, Color c) : position(p), color(c){}
    Stone(int x, int y, Color c) : position(x, y), color(c){}

    bool isBlack() const{ return color == black; }
    bool isWhite() const{ return color == white; }
    bool isEmpty() const{ return color == empty; }

    Point position;
    Color color;
};

/**
    Mark
*/
class Mark{
public:
    enum Type{cross, circle, square, triangle, character, blackTerritory, whiteTerritory, dim, select};

    // Constructor
    Mark(const Point& p, Type t) : position(p), type(t){}
    Mark(const Point& p, const QString& s) : position(p), type(character), text(s){}
    Mark(int x, int y, Type t) : position(x, y), type(t){}
    Mark(int x, int y, const QString& s) : position(x, y), type(character), text(s){}

    Point   position;
    Type    type;
    QString text;
};

class Line{
public:
    enum Type{line, arrow};

    // Constructor
    Line(const Point& p1, const Point& p2, Type t) : position1(p1), position2(p2), type(t){}
    Line(int x1, int y1, int x2, int y2, Type t) : position1(x1, y1), position2(x2, y2), type(t){}

    Point position1;
    Point position2;
    Type  type;
};

typedef QList<Stone> StoneList;
typedef QList<Mark>  MarkList;
typedef QList<Line>  LineList;


class GameInformation{
public:
    GameInformation();

    // rule
    int     xsize;
    int     ysize;
    double  komi;
    int     handicap;
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


class Node{
    Q_DECLARE_TR_FUNCTIONS(Go::Node)

public:
    enum Annotation{
        noAnnotation = 0,
        hotspot,
    };
    enum MoveAnnotation{
        noMoveAnnotation = 0,
        goodMove,
        veryGoodMove,
        badMove ,
        veryBadMove,
        doubtfulMove,
        interestingMove,
    };
    enum NodeAnnotation{
        noNodeAnnotation = 0,
        even,
        goodForBlack,
        veryGoodForBlack,
        goodForWhite,
        veryGoodForWhite,
        unclear,
    };

    // Constructor
    Node();
    explicit Node(NodePtr parent);
    virtual ~Node();

    void clear();
//    virtual QString toString() const;
    GameInformationPtr getInformation() const;

    // parent
    NodePtr parent() const{ return parent_.lock(); }
    void setParent(NodePtr node){ parent_ = node; }

    // sibling
    NodeList siblings() const;
    NodePtr  previousSibling() const;
    NodePtr  nextSibling() const;

    // get
    int x() const{ return position.x; }
    int y() const{ return position.y; }

    // set
    void setX(int x){ position.x = x; }
    void setY(int y){ position.y = y; }

    // get node type
    virtual bool isStone() const{ return isBlack() || isWhite(); }
    bool isBlack() const{ return color == black; }
    bool isWhite() const{ return color == white; }
    bool isPass() const;

    GameInformationPtr gameInformation;
    NodeWPtr  parent_;
    NodeList  childNodes;
    QString   name;
    MarkList  marks;
    MarkList  blackTerritories;
    MarkList  whiteTerritories;
    MarkList  dims;
    StoneList blackStones;
    StoneList whiteStones;
    StoneList emptyStones;
    LineList  lines;
    Annotation annotation;
    MoveAnnotation moveAnnotation;
    NodeAnnotation nodeAnnotation;
    QString comment;
    Point position;
    Color color;
    Color nextColor;
    int   moveNumber;
};


class FileBase{
public:
    FileBase() : codec(NULL){}
    virtual ~FileBase(){}

    virtual bool read(const QString& fname, QTextCodec* codec, bool guessCodec);
    virtual bool read(const QByteArray& bytes, QTextCodec* defaultCodec, bool guessCodec);
    virtual bool readStream(QString::iterator& first, QString::iterator last) = 0;

    virtual bool save(const QString& fname, QTextCodec* codec);
    virtual bool saveStream(QTextStream& stream) = 0;

    virtual QTextCodec* guessCodec(const QByteArray&) const{ return NULL; }

    virtual bool get(NodeList& gameList) const = 0;
    virtual bool set(NodeList& gameList) = 0;

    QString readLine(QString::iterator& first, QString::iterator& last);

    QTextCodec* codec;
};



NodePtr createInformationNode(NodePtr parent);
NodePtr createBlackNode(NodePtr parent);
NodePtr createBlackNode(NodePtr parent, int x, int y);
NodePtr createWhiteNode(NodePtr parent);
NodePtr createWhiteNode(NodePtr parent, int x, int y);


}



#endif
