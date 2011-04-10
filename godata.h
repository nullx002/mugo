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
#ifndef GODATA_H
#define GODATA_H

#include <QObject>
#include <QSharedPointer>
#include <QMetaType>
#include <QPoint>

namespace Go{


/**
  Stone color
*/
enum Color{ eDame, eBlack, eWhite };


/**
  Markup on the node
*/
class Mark{
public:
    enum Type{ eCircle, eSquare, eTriangle, eCross };

    // constructor
    Mark(Type t, const QPoint& p) : type_(t), x_(p.x()), y_(p.y()){}
    Mark(Type t, int x, int y) : type_(t), x_(x), y_(y){}

    // type
    int type() const{ return type_; }
    void setType(Type t){ type_ = t; }

    // x
    int x() const{ return x_; }
    void setX(int x){ x_ = x; }

    // y
    int y() const{ return y_; }
    void setY(int y){ y_ = y; }

private:
    Type type_;
    int x_, y_;
};

typedef QList<Mark> MarkList;


/**
  Game information
*/
class Information{
public:
    // constructor
    Information();

    // application name
    const QString& applicationName() const{ return applicationName_; }
    void setApplicationName(const QString& applicationName){ applicationName_ = applicationName; }

    // size
    void setSize(int xsize, int ysize){ xsize_ = xsize; ysize_ = ysize; }

    // xsize
    int xsize() const{ return xsize_; }
    void setXSize(int xsize){ xsize_ = xsize; }

    // ysize
    int ysize() const{ return ysize_; }
    void setYSize(int ysize){ ysize_ = ysize; }

    // variation style
    int variationStyle() const{ return variationStyle_; }
    void setVariationStyle(int variationStyle){ variationStyle_ = variationStyle; }

    // black
    const QString& blackPlayer() const{ return blackPlayer_; }
    void setBlackPlayer(const QString& player){ blackPlayer_ = player; }
    const QString& blackRank() const{ return blackRank_; }
    void setBlackRank(const QString& rank){ blackRank_ = rank; }
    const QString& blackTeam() const{ return blackTeam_; }
    void setBlackTeam(const QString& team){ blackTeam_ = team; }

    // white
    const QString& whitePlayer() const{ return whitePlayer_; }
    void setWhitePlayer(const QString& player){ whitePlayer_ = player; }
    const QString& whiteRank() const{ return whiteRank_; }
    void setWhiteRank(const QString& rank){ whiteRank_ = rank; }
    const QString& whiteTeam() const{ return whiteTeam_; }
    void setWhiteTeam(const QString& team){ whiteTeam_ = team; }

    // result
    const QString& result() const{ return result_; }
    void setResult(const QString& result){ result_ = result; }

    // rule
    qreal komi() const{ return komi_; }
    void  setKomi(qreal komi){ komi_ = komi; }
    int   handicap() const{ return handicap_; }
    void  setHandicap(int handicap){ handicap_ = handicap; }
    const QString& time(){ return time_; }
    void setTime(const QString& time){ time_ = time; }
    const QString& overtime(){ return overtime_; }
    void setOvertime(const QString& overtime){ overtime_ = overtime; }
    const QString& rule(){ return rule_; }
    void setRule(const QString& rule){ rule_ = rule; }

    // when and where
    const QString& date() const{ return date_; }
    void setDate(const QString& date){ date_ = date; }
    const QString& place() const{ return place_; }
    void setPlace(const QString& place){ place_ = place; }

    // game name
    const QString& event() const{ return event_; }
    void setEvent(const QString& event){ event_ = event; }
    const QString& gameName() const{ return gameName_; }
    void setGameName(const QString& name){ gameName_ = name; }
    const QString& round() const{ return round_; }
    void setRound(const QString& round){ round_ = round; }

    // annotation
    const QString& copyright() const{ return copyright_; }
    void setCopyright(const QString& copyright){ copyright_ = copyright; }
    const QString& gameComment() const{ return gameComment_; }
    void setGameComment(const QString& comment){ gameComment_ = comment; }
    const QString& opening() const{ return opening_; }
    void setOpening(const QString& opening){ opening_ = opening; }
    const QString& source() const{ return source_; }
    void setSource(const QString& source){ source_ = source; }
    const QString& user() const{ return user_; }
    void setUser(const QString& user){ user_ = user; }
    const QString& annotation() const{ return annotation_; }
    void setAnnotation(const QString& annotation){ annotation_ = annotation; }

private:
    QString applicationName_;
    int variationStyle_;

    // rule
    int     xsize_;
    int     ysize_;
    qreal   komi_;
    int     handicap_;
    QString time_;
    QString overtime_;
    QString rule_;

    // result
    QString result_;

    // player
    QString blackPlayer_;
    QString blackRank_;
    QString blackTeam_;
    QString whitePlayer_;
    QString whiteRank_;
    QString whiteTeam_;

    // when and where
    QString date_;
    QString place_;

    // game name
    QString gameName_;
    QString event_;
    QString round_;

    // annotation
    QString annotation_;
    QString gameComment_;
    QString opening_;
    QString source_;
    QString user_;
    QString copyright_;
};
typedef QSharedPointer<Information> InformationPtr;


/**
  Node of one move
*/
class Node{
public:
    typedef QSharedPointer<Node> NodePtr;
    typedef QWeakPointer<Node> WeakNodePtr;
    typedef QList<NodePtr> NodeList;

    enum NodeAnnotation{ eNoNodeAnnotation, eGoodForBlack, eVeryGoodForBlack, eGoodForWhite, eVeryGoodForWhite, eEven, eUnclear };
    enum NodeAnnotation2{ eNoNodeAnnotation2, eHotspot };
    enum MoveAnnotation{ eNoMoveAnnotation, eGoodMove, eVeryGoodMove, eBadMove, eVeryBadMove, eDoubtful, eInteresting };

    // constructor, destructor
    Node();
    Node(const NodePtr& parentNode);
    ~Node();

    // parent
    NodePtr parent() const{ return parent_.toStrongRef(); }
    void setParent(const NodePtr& parent){ parent_ = parent; }
    void setParent(){ parent_.clear(); }

    // game information
    InformationPtr information() const{ return information_; }
    void setInformation(InformationPtr info){ information_ = info; }

    // children
    NodeList& children(){ return children_; }
    NodePtr child(int i){ return children_[i]; }

    // color
    Color color() const{ return color_; }
    void setColor(Color color){ color_ = color; }

    // next color
    Color nextColor() const;
    void setNextColor(Color color){ nextColor_ = color; }

    // node type
    bool isStone() const{ return color_ == Go::eBlack || color_ == Go::eWhite; }
    bool isPass() const{ return isStone() && (x_ < 0 || y_ < 0); }

    // position
    void setPos(int x, int y){ x_ = x; y_ = y; }
    int x() const{ return x_; }
    void setX(int x){ x_ = x; }
    int y() const{ return y_; }
    void setY(int y){ y_ = y; }

    // move number
    int moveNumber() const{ return moveNumber_; }
    void setMoveNumber(int moveNumber){ moveNumber_ = moveNumber; }

    // name
    const QString& name() const{ return name_; }
    void setName(const QString& name){ name_ = name; }

    // comment
    const QString& comment() const{ return comment_; }
    void setComment(const QString& comment){ comment_ = comment; }

    // node annotation
    NodeAnnotation nodeAnnotation() const{ return nodeAnnotation_; }
    void setNodeAnnotation(NodeAnnotation annotation){ nodeAnnotation_ = annotation; }

    bool hasEstimatedScore() const{ return hasEstimatedScore_; }
    void clearEstimatedScore(){ hasEstimatedScore_ = false; }
    qreal estimatedScore() const{ return estimatedScore_; }
    void setEstimatedScore(qreal score){ hasEstimatedScore_ = true; estimatedScore_ = score; }

    // node annotation 2
    NodeAnnotation2 nodeAnnotation2() const{ return nodeAnnotation2_; }
    void setNodeAnnotation2(NodeAnnotation2 annotation){ nodeAnnotation2_ = annotation; }

    // move annotation
    MoveAnnotation moveAnnotation() const{ return moveAnnotation_; }
    void setMoveAnnotation(MoveAnnotation annotation){ moveAnnotation_ = annotation; }

    // stnoes
    const QList<QPoint>& emptyStones() const{ return emptyStones_; }
    QList<QPoint>& emptyStones(){ return emptyStones_; }
    const QList<QPoint>& blackStones() const{ return blackStones_; }
    QList<QPoint>& blackStones(){ return blackStones_; }
    const QList<QPoint>& whiteStones() const{ return whiteStones_; }
    QList<QPoint>& whiteStones(){ return whiteStones_; }

    // marks
    const MarkList& marks() const{ return marks_; }
    MarkList& marks(){ return marks_; }

private:
    WeakNodePtr parent_;
    NodeList children_;

    InformationPtr information_;
    QString name_;
    Go::Color color_;
    Go::Color nextColor_;
    int x_;
    int y_;
    int moveNumber_;
    QString comment_;
    NodeAnnotation nodeAnnotation_;
    NodeAnnotation2 nodeAnnotation2_;
    MoveAnnotation moveAnnotation_;
    bool hasEstimatedScore_;
    qreal estimatedScore_;
    QList<QPoint> emptyStones_;
    QList<QPoint> blackStones_;
    QList<QPoint> whiteStones_;
    MarkList marks_;
};

typedef Node::NodePtr NodePtr;
typedef Node::NodeList NodeList;


NodePtr createStoneNode(Color color, int x, int y);
void copyNodeList(Go::NodeList& dstList, const Go::NodeList& srcList);
void copyNode(Go::NodePtr& dst, const Go::NodePtr& src);
QString coordinateString(int xsize, int ysize, int x, int y, bool showI = false);


/**
  if current move is black, return white.
  if current move is white, return black.
  however, if next move is designated, return designated color.
*/
inline
Color Node::nextColor() const{
    if (nextColor_ != Go::eDame)
        return nextColor_;
    else
        return color_ == eBlack ? eWhite : eBlack;
}



}


Q_DECLARE_METATYPE(Go::NodePtr);


#endif // GODATA_H
