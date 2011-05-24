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
#include <QDebug>
#include "sgfcommand.h"
#include "sgfdocument.h"


/**
  Constructs add game command
*/
AddGameCommand::AddGameCommand(GoDocument* doc, Go::NodePtr game, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
{
    gameList_.push_back(game);
}

/**
  Constructs add game command
*/
AddGameCommand::AddGameCommand(GoDocument* doc, const Go::NodeList& gameList, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , gameList_(gameList)
{
}

/**
  redo add game command
*/
void AddGameCommand::redo(){
    setText( tr("Add Game into SGF Collection") );
    document_->addGameList(gameList_);
}

/**
  undo add node command
*/
void AddGameCommand::undo(){
    document_->deleteGameList(gameList_);
}

/**
  Constructs game move up command
*/
MoveUpGameCommand::MoveUpGameCommand(GoDocument* doc, const Go::NodePtr& game, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
{
}

/**
  redo game move up command
*/
void MoveUpGameCommand::redo(){
    setText( tr("Game Move Up in SGF Collection") );
    document_->moveUpGame(game_);
}

/**
  undo game move up command
*/
void MoveUpGameCommand::undo(){
    document_->moveDownGame(game_);
}

/**
  Constructs game move down command
*/
MoveDownGameCommand::MoveDownGameCommand(GoDocument* doc, const Go::NodePtr& game, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
{
}

/**
  redo game move down command
*/
void MoveDownGameCommand::redo(){
    setText( tr("Game Move Down in SGF Collection") );
    document_->moveDownGame(game_);
}

/**
  undo game move down command
*/
void MoveDownGameCommand::undo(){
    document_->moveUpGame(game_);
}

/**
  Constructs change game command
*/
ChangeGameCommand::ChangeGameCommand(GoDocument* doc, BoardWidget* board, Go::NodePtr game, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , board_(board)
    , game_(game)
{
    prevGame_ = board_->currentGame();
}

/**
  redo change game command
*/
void ChangeGameCommand::redo(){
    setText( tr("Change Game") );
    board_->setGame(game_);
}

/**
  undo change game command
*/
void ChangeGameCommand::undo(){
    board_->setGame(prevGame_);
}

/**
  Constructs set game information command
*/
SetGameInformationCommand::SetGameInformationCommand(GoDocument* doc, Go::NodePtr node, Go::InformationPtr info, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , node_(node)
    , info_(info)
{
    prevInfo_ = node_->information();
}

/**
  redo set game information command
*/
void SetGameInformationCommand::redo(){
    setText( tr("Set Game Information") );
    document_->setInformation(node_, info_);
}

/**
  undo set game information command
*/
void SetGameInformationCommand::undo(){
    document_->setInformation(node_, prevInfo_);
}

/**
  Constructs add node command
*/
AddNodeCommand::AddNodeCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr parentNode, Go::NodePtr node, int index, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , parentNode_(parentNode)
    , node_(node)
    , index_(index)
{
}

/**
  redo add node command
*/
void AddNodeCommand::redo(){
    if (node_->isStone())
        setText( tr("Move") );
    else
        setText( tr("Add Empty Node") );
    document_->addNode(game_, parentNode_, node_, index_);
}

/**
  undo add node command
*/
void AddNodeCommand::undo(){
    document_->deleteNode(game_, node_);
}

/**
  Constructs remove node command
*/
RemoveNodeCommand::RemoveNodeCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, bool removeChildren, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , removeChildren_(removeChildren)
{
    parentNode_ = node->parent();
    index_ = parentNode_->children().indexOf(node_);
}

/**
  redo remove node command
*/
void RemoveNodeCommand::redo(){
    setText( tr("Remove Node") );

    if (removeChildren_ == false)
        children_ = node_->children();

    document_->deleteNode(game_, node_, removeChildren_);
}

/**
  undo remove node command
*/
void RemoveNodeCommand::undo(){
    if (removeChildren_ == false){
        foreach(Go::NodePtr child, children_){
            child->setParent(node_);
            node_->parent()->children().removeOne(child);
        }
    }

    document_->addNode(game_, parentNode_, node_, index_);
}

/**
  Constructs set comment command
*/
SetCommentCommand::SetCommentCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, const QString& comment, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , comment_(comment)
{
    prevComment_ = node_->comment();
}

/**
  redo set comment ommand
*/
void SetCommentCommand::redo(){
    setText( tr("Comment") );
    node_->setComment(comment_);
    document_->modifyNode(game_, node_);
}

/**
  undo set comment command
*/
void SetCommentCommand::undo(){
    node_->setComment(prevComment_);
    document_->modifyNode(game_, node_);
}

/**
  set comment
*/
void SetCommentCommand::setComment(const QString& comment){
    comment_ = comment;
    redo();
}

/**
  Constructs add stone command
*/
AddStoneCommand::AddStoneCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, int x, int y, const Go::Color color, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , x_(x)
    , y_(y)
    , color_(color)
{
}

/**
  redo add stone command
*/
void AddStoneCommand::redo(){
    if(color_ == Go::eBlack){
        setText( tr("Add Black Stone") );
        node_->blackStones().push_back( QPoint(x_, y_) );
    }
    else if(color_ == Go::eWhite){
        setText( tr("Add White Stone") );
        node_->whiteStones().push_back( QPoint(x_, y_) );
    }
    else if(color_ == Go::eDame){
        setText( tr("Add Empty Stone") );
        node_->emptyStones().push_back( QPoint(x_, y_) );
    }

    document_->modifyNode(game_, node_, true);
}

/**
  undo add stone command
*/
void AddStoneCommand::undo(){
    if (color_ == Go::eBlack)
        node_->blackStones().pop_back();
    else if (color_ == Go::eWhite)
        node_->whiteStones().pop_back();
    else if (color_ == Go::eDame)
        node_->emptyStones().pop_back();

    document_->modifyNode(game_, node_, true);
}

/**
  Constructs remove stone command
*/
RemoveStoneCommand::RemoveStoneCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, QList<QPoint>& stones, const QPoint& pos, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , stones_(stones)
    , pos_(pos)
{
}

/**
  redo remove stone command
*/
void RemoveStoneCommand::redo(){
    setText( tr("Remove Stone") );
    index_ = stones_.indexOf(pos_);
    stones_.removeAt(index_);

    document_->modifyNode(game_, node_, true);
}

/**
  undo remove  stone command
*/
void RemoveStoneCommand::undo(){
    stones_.insert(index_, pos_);

    document_->modifyNode(game_, node_, true);
}

/**
  Constructs add mark command
*/
AddMarkCommand::AddMarkCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, int x, int y, const Go::Mark::Type mark, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , x_(x)
    , y_(y)
    , mark_(mark)
{
}

/**
  redo add mark command
*/
void AddMarkCommand::redo(){

    setText( tr("Add Mark") );
    node_->marks().push_back( Go::Mark(mark_, x_, y_) );
    document_->modifyNode(game_, node_, true);
}

/**
  undo add mark command
*/
void AddMarkCommand::undo(){
    node_->marks().pop_back();
    document_->modifyNode(game_, node_, true);
}

/**
  Constructs add label command
*/
AddLabelCommand::AddLabelCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, int x, int y, const QString& label, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , x_(x)
    , y_(y)
    , label_(label)
{
}

/**
  redo add label command
*/
void AddLabelCommand::redo(){

    setText( tr("Add Label") );
    node_->marks().push_back( Go::Mark(label_, x_, y_) );
    document_->modifyNode(game_, node_, true);
}

/**
  undo add label command
*/
void AddLabelCommand::undo(){
    node_->marks().pop_back();
    document_->modifyNode(game_, node_, true);
}

/**
  Constructs remove mark command
*/
RemoveMarkCommand::RemoveMarkCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, Go::MarkList& markList, const Go::Mark& mark, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , markList_(markList)
    , mark_(mark)
{
}

/**
  redo remove mark command
*/
void RemoveMarkCommand::redo(){

    setText( tr("Remove Mark") );
    index_ = markList_.indexOf(mark_);
    markList_.removeAt(index_);
    document_->modifyNode(game_, node_, true);
}

/**
  undo remove mark command
*/
void RemoveMarkCommand::undo(){
    markList_.insert(index_, mark_);
    document_->modifyNode(game_, node_, true);
}

/**
  Constructs set move annotation command
*/
SetMoveAnnotationCommand::SetMoveAnnotationCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, Go::Node::MoveAnnotation annotation, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , annotation_(annotation)
{
    prevAnnotation_ = node_->moveAnnotation();
}

/**
  redo set move annotation command
*/
void SetMoveAnnotationCommand::redo(){

    setText( tr("Set Move Annotation") );
    node_->setMoveAnnotation(annotation_);
    document_->modifyNode(game_, node_);
}

/**
  undo set move annotation command
*/
void SetMoveAnnotationCommand::undo(){
    node_->setMoveAnnotation(prevAnnotation_);
    document_->modifyNode(game_, node_);
}

/**
  Constructs set node annotation command
*/
SetNodeAnnotationCommand::SetNodeAnnotationCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, Go::Node::NodeAnnotation annotation, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , annotation_(annotation)
{
    prevAnnotation_ = node_->nodeAnnotation();
}

/**
  redo set node annotation command
*/
void SetNodeAnnotationCommand::redo(){

    setText( tr("Set Node Annotation") );
    node_->setNodeAnnotation(annotation_);
    document_->modifyNode(game_, node_);
}

/**
  undo set move annotation command
*/
void SetNodeAnnotationCommand::undo(){
    node_->setNodeAnnotation(prevAnnotation_);
    document_->modifyNode(game_, node_);
}

/**
  Constructs set node annotation2 command
*/
SetNodeAnnotation2Command::SetNodeAnnotation2Command(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, Go::Node::NodeAnnotation2 annotation, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , annotation_(annotation)
{
    prevAnnotation_ = node_->nodeAnnotation2();
}

/**
  redo set node annotation2 command
*/
void SetNodeAnnotation2Command::redo(){

    setText( tr("Set Node Annotation") );
    node_->setNodeAnnotation2(annotation_);
    document_->modifyNode(game_, node_);
}

/**
  undo set move annotation2 command
*/
void SetNodeAnnotation2Command::undo(){
    node_->setNodeAnnotation2(prevAnnotation_);
    document_->modifyNode(game_, node_);
}

/**
  Constructs set move number command
*/
SetMoveNumberCommand::SetMoveNumberCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, int moveNumber, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , moveNumber_(moveNumber)
{
    prevMoveNumber_ = node_->moveNumber();
}

/**
  redo set move number command
*/
void SetMoveNumberCommand::redo(){
    if (moveNumber_ > 0)
        setText( tr("Set Move Number") );
    else
        setText( tr("Unet Move Number") );

    node_->setMoveNumber(moveNumber_);
    document_->modifyNode(game_, node_, true);
}

/**
  undo set move number command
*/
void SetMoveNumberCommand::undo(){
    node_->setMoveNumber(prevMoveNumber_);
    document_->modifyNode(game_, node_, true);
}

/**
  Constructs set node name command
*/
SetNodeNameCommand::SetNodeNameCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, const QString& nodeName, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , nodeName_(nodeName)
{
    prevNodeName_ = node_->name();
}

/**
  redo set node name command
*/
void SetNodeNameCommand::redo(){
    if (nodeName_.isEmpty())
        setText( tr("Clear Node Name") );
    else
        setText( tr("Set Node Name") );
    node_->setName(nodeName_);
    document_->modifyNode(game_, node_);
}

/**
  undo set node name command
*/
void SetNodeNameCommand::undo(){
    node_->setName(prevNodeName_);
    document_->modifyNode(game_, node_);
}

/**
  Constructs set next color command
*/
SetNextColorCommand::SetNextColorCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, Go::Color color, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
    , node_(node)
    , color_(color)
{
    prevColor_ = node->nextColor();
}

/**
  redo set next color command
*/
void SetNextColorCommand::redo(){
    if (color_ == Go::eDame)
        setText( tr("Unset Next Color") );
    else
        setText( tr("Set Next Color") );

    node_->setNextColor(color_);
    document_->modifyNode(game_, node_);
}

/**
  undo set next color command
*/
void SetNextColorCommand::undo(){
    node_->setNextColor(prevColor_);
    document_->modifyNode(game_, node_);
}

/**
  Constructs rotate sgf clockwise command
*/
RotateClockwiseCommand::RotateClockwiseCommand(GoDocument* doc, Go::NodePtr game, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
{
    setText( tr("Rotate Clockwise") );
}

/**
  redo rotate sgf clockwise command
*/
void RotateClockwiseCommand::redo(){
    game_->information()->setSize(game_->information()->ysize(), game_->information()->xsize());
    rotate(game_, true);
    document_->modifyDocument(true);
}

/**
  undo rotate sgf clockwise command
*/
void RotateClockwiseCommand::undo(){
    game_->information()->setSize(game_->information()->ysize(), game_->information()->xsize());
    rotate(game_, false);
    document_->modifyDocument(true);
}

/**
  rotate all stones and markers
*/
void RotateClockwiseCommand::rotate(Go::NodePtr node, bool clockwise){
    // rotate stones
    if (node->isStone() && node->isPass() == false){
        int x, y;
        getRotatedPosition(node->x(), node->y(), x, y, clockwise);
        node->setPos(x, y);
    }

    for(QList<QPoint>::iterator iter = node->emptyStones().begin(); iter != node->emptyStones().end(); ++iter){
        int x, y;
        getRotatedPosition(iter->x(), iter->y(), x, y, clockwise);
        *iter = QPoint(x, y);
    }

    for(QList<QPoint>::iterator iter = node->blackStones().begin(); iter != node->blackStones().end(); ++iter){
        int x, y;
        getRotatedPosition(iter->x(), iter->y(), x, y, clockwise);
        *iter = QPoint(x, y);
    }

    for(QList<QPoint>::iterator iter = node->whiteStones().begin(); iter != node->whiteStones().end(); ++iter){
        int x, y;
        getRotatedPosition(iter->x(), iter->y(), x, y, clockwise);
        *iter = QPoint(x, y);
    }

    // rotate marks
    for(Go::MarkList::iterator iter = node->marks().begin(); iter != node->marks().end(); ++iter){
        int x, y;
        getRotatedPosition(iter->x(), iter->y(), x, y, clockwise);
        iter->setX(x);
        iter->setY(y);
    }

    // update node
    document_->modifyNode(game_, node);

    // rotate chidl stones
    foreach(const Go::NodePtr& child, node->children())
        rotate(child, clockwise);
}

/**
  get rotated position
*/
void RotateClockwiseCommand::getRotatedPosition(int x, int y, int& newX, int& newY, bool clockwise){
    // information node has already rotated.
    int w = game_->information()->ysize();
    int h = game_->information()->xsize();

    if (clockwise){
        newX = h - y - 1;
        newY = x;
    }
    else{
        newX = y;
        newY = w - x - 1;
    }
}

/**
  Constructs flip sgf command
*/
FlipCommand::FlipCommand(GoDocument* doc, Go::NodePtr game, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
    , game_(game)
{
}

/**
  rotate all stones and markers
*/
void FlipCommand::flip(Go::NodePtr node, bool vertical){
    // flip stones
    if (node->isStone() && node->isPass() == false){
        int x, y;
        getFlippedPosition(node->x(), node->y(), x, y, vertical);
        node->setPos(x, y);
    }

    for(QList<QPoint>::iterator iter = node->emptyStones().begin(); iter != node->emptyStones().end(); ++iter){
        int x, y;
        getFlippedPosition(iter->x(), iter->y(), x, y, vertical);
        *iter = QPoint(x, y);
    }

    for(QList<QPoint>::iterator iter = node->blackStones().begin(); iter != node->blackStones().end(); ++iter){
        int x, y;
        getFlippedPosition(iter->x(), iter->y(), x, y, vertical);
        *iter = QPoint(x, y);
    }

    for(QList<QPoint>::iterator iter = node->whiteStones().begin(); iter != node->whiteStones().end(); ++iter){
        int x, y;
        getFlippedPosition(iter->x(), iter->y(), x, y, vertical);
        *iter = QPoint(x, y);
    }

    // rotate marks
    for(Go::MarkList::iterator iter = node->marks().begin(); iter != node->marks().end(); ++iter){
        int x, y;
        getFlippedPosition(iter->x(), iter->y(), x, y, vertical);
        iter->setX(x);
        iter->setY(y);
    }

    // update node
    document_->modifyNode(game_, node);

    // rotate chidl stones
    foreach(const Go::NodePtr& child, node->children())
        flip(child, vertical);
}

/**
  get flipped position
*/
void FlipCommand::getFlippedPosition(int x, int y, int& newX, int& newY, bool vertical){
    int w = game_->information()->xsize();
    int h = game_->information()->ysize();

    if (vertical){
        newX = x;
        newY = h - y - 1;
    }
    else{
        newX = w - x - 1;
        newY = y;
    }
}

/**
  Constructs flip horizontally sgf command
*/
FlipHorizontallyCommand::FlipHorizontallyCommand(GoDocument* doc, Go::NodePtr game, QUndoCommand* parent)
    : FlipCommand(doc, game, parent)
{
    setText( tr("Flip Horizontally") );
}

/**
  redo flip horizontally sgf command
*/
void FlipHorizontallyCommand::redo(){
    flip(game_, false);
    document_->modifyDocument(true);
}

/**
  undo flip horizontally sgf command
*/
void FlipHorizontallyCommand::undo(){
    flip(game_, false);
    document_->modifyDocument(true);
}

/**
  Constructs flip vertically sgf command
*/
FlipVerticallyCommand::FlipVerticallyCommand(GoDocument* doc, Go::NodePtr game, QUndoCommand* parent)
    : FlipCommand(doc, game, parent)
{
    setText( tr("Flip Vertically") );
}

/**
  redo flip vertically sgf command
*/
void FlipVerticallyCommand::redo(){
    flip(game_, true);
    document_->modifyDocument(true);
}

/**
  undo flip vertically sgf command
*/
void FlipVerticallyCommand::undo(){
    flip(game_, true);
    document_->modifyDocument(true);
}
