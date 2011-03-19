#ifndef GODATA_H
#define GODATA_H

#include <QObject>
#include <QSharedPointer>


namespace Go{


enum Color{ eDame, eBlack, eWhite };


class GameInformation{
public:
    int   xsize() const{ return xsize_; }
    void  setXSize(int xsize){ xsize_ = xsize; }

    int   ysize() const{ return ysize_; }
    void  setYSize(int ysize){ ysize_ = ysize; }

    qreal komi() const{ return komi_; }
    void  setKomi(qreal komi){ komi_ = komi; }

    int   handicap() const{ return handicap_; }
    void  setHandicap(int handicap){ handicap_ = handicap; }

private:
    int   xsize_;
    int   ysize_;
    qreal komi_;
    int   handicap_;
};
typedef QSharedPointer<GameInformation> InformationPtr;

class Node{
public:
    typedef QSharedPointer<Node> NodePtr;
    typedef QWeakPointer<Node> WeakNodePtr;
    typedef QList<NodePtr> NodeList;

    Node();
    Node(const NodePtr& parent);
    ~Node();

    NodePtr parent() const{ return parent_.toStrongRef(); }
    void setParent(const NodePtr& parent){ parent_ = parent; }

    InformationPtr gameInformation() const{ return gameInformation_; }
    void setGameInformation(InformationPtr gameInfo){ gameInformation_ = gameInfo; }

    NodeList& children(){ return children_; }
    NodePtr child(int i){ return children_[i]; }

    Color color() const{ return color_; }
    void setColor(Color color){ color_ = color; }

    Color nextColor() const;
    void setNextColor(Color color){ nextColor_ = color; }

    bool isStone() const{ return color_ == Go::eBlack || color_ == Go::eWhite; }
    bool isPass() const{ return isStone() && (x_ < 0 || y_ < 0); }

    int x() const{ return x_; }
    void setX(int x){ x_ = x; }

    int y() const{ return y_; }
    void setY(int y){ y_ = y; }

private:
    WeakNodePtr parent_;
    NodeList children_;

    InformationPtr gameInformation_;
    QString name_;
    Go::Color color_;
    Go::Color nextColor_;
    int x_;
    int y_;
};

typedef Node::NodePtr NodePtr;
typedef Node::NodeList NodeList;


NodePtr createStoneNode(Color color, int x, int y);


inline
Color Node::nextColor() const{
    if (nextColor_ != Go::eDame)
        return nextColor_;
    else
        return color_ == eBlack ? eWhite : eBlack;
}


}


#endif // GODATA_H
