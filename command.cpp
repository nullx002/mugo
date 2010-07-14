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
#include <QDebug>
#include "command.h"
#include "boardwidget.h"

/**
* Add Node Command
*/
AddNodeCommand::AddNodeCommand(BoardWidget* _boardWidget, go::nodePtr _parentNode, go::nodePtr _childNode, bool _select, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , parentNode(_parentNode)
    , childNode(_childNode)
    , select(_select)
{
}

void AddNodeCommand::redo(){
    setText( tr("Add %1").arg( boardWidget->toString(childNode) ) );
    boardWidget->addNode(parentNode, childNode, select);
}

void AddNodeCommand::undo(){
    boardWidget->deleteNode(childNode);
}

/**
* Insert Node Command
*/
InsertNodeCommand::InsertNodeCommand(BoardWidget* _boardWidget, go::nodePtr _parentNode, int _index, go::nodePtr _childNode, bool _select, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , parentNode(_parentNode)
    , childNode(_childNode)
    , index(_index)
    , select(_select)
{
}

void InsertNodeCommand::redo(){
    setText( tr("Insert %1").arg( boardWidget->toString(childNode) ) );
    boardWidget->insertNode(parentNode, index, childNode, select);
}

void InsertNodeCommand::undo(){
    boardWidget->deleteNode(childNode, false);
}

/**
* Delete Node Command
*/
DeleteNodeCommand::DeleteNodeCommand(BoardWidget* _boardWidget, go::nodePtr _node, bool _deleteChildren, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
    , deleteChildren(_deleteChildren)
{
}

void DeleteNodeCommand::redo(){
    setText( tr("Delete %1").arg( boardWidget->toString(node) ) );

    go::nodeList::iterator beg = node->parent()->childNodes.begin();
    go::nodeList::iterator del = qFind(node->parent()->childNodes.begin(), node->parent()->childNodes.end(), node);
    index = std::distance(beg, del);

    boardWidget->deleteNode(node, deleteChildren);
}

void DeleteNodeCommand::undo(){
    if (!deleteChildren){
        for (int i=0; i<node->childNodes.size(); ++i)
            node->parent()->childNodes.removeAt(index + i);

        go::nodeList::iterator iter = node->childNodes.begin();
        while (iter != node->childNodes.end()){
            (*iter)->parent_ = node;
            ++iter;
        }
    }
    boardWidget->insertNode(node->parent(), index, node);
}

/**
*/
AddStoneCommand::AddStoneCommand(BoardWidget* _boardWidget, go::nodePtr _node, int _x, int _y, go::color c, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
    , x(_x)
    , y(_y)
    , color(c)
{
}

void AddStoneCommand::redo(){
    setText( tr("Add Stone") );

/*
    if (removeStone(node->emptyStones, sp, bp) || removeStone(node->blackStones, sp, bp) || removeStone(node->whiteStones, sp, bp)){
        modifyNode(node);
        return;
    }
*/

    go::nodePtr stoneNode( node->isStone() ? go::nodePtr(new go::node(node)) : node );

    if (color == go::black)
        node->blackStones.push_back( go::stone(x, y, color) );
    else if (color == go::white)
        node->whiteStones.push_back( go::stone(x, y, color) );
    else
        node->emptyStones.push_back( go::stone(x, y, color) );

    boardWidget->modifyNode(node, true);
}

void AddStoneCommand::undo(){
    if (color == go::black)
        node->blackStones.pop_back();
    else if (color == go::white)
        node->whiteStones.pop_back();
    else
        node->emptyStones.pop_back();

    boardWidget->modifyNode(node, true);
}

/**
*/
DeleteStoneCommand::DeleteStoneCommand(BoardWidget* _boardWidget, go::nodePtr _node, int _x, int _y, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
    , x(_x)
    , y(_y)
{
}

void DeleteStoneCommand::redo(){
    setText( tr("Delete Stone") );

    remove(node->blackStones, blackEraseList, blackPosList);
    remove(node->whiteStones, whiteEraseList, whitePosList);
    remove(node->emptyStones, emptyEraseList, emptyPosList);

    boardWidget->modifyNode(node, true);
}

void DeleteStoneCommand::undo(){
    add(node->blackStones, blackEraseList, blackPosList);
    add(node->whiteStones, whiteEraseList, whitePosList);
    add(node->emptyStones, emptyEraseList, emptyPosList);

    boardWidget->modifyNode(node, true);
}

void DeleteStoneCommand::add(go::stoneList& stones, go::stoneList& eraseList, QList<int>& posList){
    go::point p(x, y);
    int i = 0;
    foreach(const go::stone& stone, eraseList){
        stones.insert(posList[i], stone);
        ++i;
    }
}

void DeleteStoneCommand::remove(go::stoneList& stones, go::stoneList& eraseList, QList<int>& posList){
    eraseList.clear();
    posList.clear();

    go::point p(x, y);
    go::stoneList::iterator iter = stones.begin();
    int i = 0;
    while (iter != stones.end()){
        if (iter->p == p){
            eraseList.push_back(*iter);
            posList.push_back(i);
            iter = stones.erase(iter);
            if (iter == stones.end())
                break;
        }
        ++iter;
        ++i;
    }
}

/**
* Add Mark Command
*/
AddMarkCommand::AddMarkCommand(BoardWidget* _boardWidget, go::nodePtr _node, int _x, int _y, go::mark::eType t, const QString& _label, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
    , x(_x)
    , y(_y)
    , type(t)
    , label(_label)
{
}

void AddMarkCommand::redo(){
    setText( tr("Add Mark") );

    markAdded = false;
    eraseList.clear();
    erasePos.clear();

    bool removeMark = false;
    go::point p(x, y);
    for (int i=0; i<node->marks.size();){
        const go::mark& mark = node->marks[i];
        if (mark.p == p){
            if (mark.t == type)
                removeMark = true;
            eraseList.push_back(mark);
            erasePos.push_back(i);
            node->marks.removeAt(i);
        }
        else
            ++i;
    }

    if (removeMark == false){
        if (type == go::mark::eCharacter)
            node->marks.push_back( go::mark(p, label) );
        else
            node->marks.push_back( go::mark(p, type) );
        markAdded = true;
    }
    boardWidget->modifyNode(node);
}

void AddMarkCommand::undo(){
    if (markAdded)
        node->marks.pop_back();

    int i = 0;
    foreach(const go::mark& m, eraseList){
        int pos = erasePos[i];
        node->marks.insert(pos, m);
    }
    boardWidget->modifyNode(node);
}

/**
* Delete Mark Command
*/
DeleteMarkCommand::DeleteMarkCommand(BoardWidget* _boardWidget, go::nodePtr _node, int _x, int _y, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
    , x(_x)
    , y(_y)
{
}

void DeleteMarkCommand::redo(){
    setText( tr("Delete Mark") );

    markEraseList.clear();
    markPosList.clear();
    emptyEraseList.clear();
    emptyPosList.clear();
    blackEraseList.clear();
    blackPosList.clear();
    whiteEraseList.clear();
    whitePosList.clear();

    remove(node->marks, markEraseList, markPosList);
    remove(node->emptyStones, emptyEraseList, emptyPosList);
    remove(node->blackStones, blackEraseList, blackPosList);
    remove(node->whiteStones, whiteEraseList, whitePosList);

    boardWidget->modifyNode(node, true);
}

void DeleteMarkCommand::undo(){
    add(node->marks, markEraseList, markPosList);
    add(node->emptyStones, emptyEraseList, emptyPosList);
    add(node->blackStones, blackEraseList, blackPosList);
    add(node->whiteStones, whiteEraseList, whitePosList);

    boardWidget->modifyNode(node, true);
}

template<class Container, class EraseList, class PosList>
void DeleteMarkCommand::add(Container& c, EraseList& eraseList, PosList& posList){
    go::point p(x, y);
    int i = 0;
    typename EraseList::iterator iter = eraseList.begin();
    while (iter != eraseList.end()){
        c.insert(posList[i], *iter);
        ++iter;
        ++i;
    }
}

template<class Container, class EraseList, class PosList>
void DeleteMarkCommand::remove(Container& c, EraseList& eraseList, PosList& posList){
    go::point p(x, y);
    int i = 0;
    typename Container::iterator iter = c.begin();
    while (iter != c.end()){
        if (iter->p == p){
            posList.push_back(i);
            eraseList.push_back(*iter);
            iter = c.erase(iter);
            if (iter == c.end())
                break;
        }
        ++iter;
        ++i;
    }
}

SetMoveNumberCommand::SetMoveNumberCommand(BoardWidget* _boardWidget, go::nodePtr _node, int _moveNumber, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
    , moveNumber(_moveNumber)
{
    oldMoveNumber = node->moveNumber;
}

void SetMoveNumberCommand::redo(){
    setText( tr("Set Move Number %1").arg( boardWidget->toString(node) ) );
    node->moveNumber = moveNumber;
    boardWidget->modifyNode(node);
}

void SetMoveNumberCommand::undo(){
    node->moveNumber = oldMoveNumber;
    boardWidget->modifyNode(node);
}

UnsetMoveNumberCommand::UnsetMoveNumberCommand(BoardWidget* _boardWidget, go::nodePtr _node, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
{
    oldMoveNumber = node->moveNumber;
}

void UnsetMoveNumberCommand::redo(){
    setText( QString(tr("Unset Move Number %1")).arg( boardWidget->toString(node) ) );
    node->moveNumber = -1;
    boardWidget->modifyNode(node);
}

void UnsetMoveNumberCommand::undo(){
    node->moveNumber = oldMoveNumber;
    boardWidget->modifyNode(node);
}

SetNodeNameCommand::SetNodeNameCommand(BoardWidget* _boardWidget, go::nodePtr _node, const QString& _nodeName, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
    , nodeName(_nodeName)
{
    oldNodeName = node->name;
}

void SetNodeNameCommand::redo(){
    setText( tr("Set Node Name %1").arg( boardWidget->toString(node) ) );
    node->name = nodeName;
    boardWidget->modifyNode(node);
}

void SetNodeNameCommand::undo(){
    node->name = oldNodeName;
    boardWidget->modifyNode(node);
}

SetCommentCommand::SetCommentCommand(BoardWidget* _boardWidget, go::nodePtr _node, const QString& _comment, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
    , comment(_comment)
{
    oldComment = node->comment;
}

void SetCommentCommand::redo(){
    setText( tr("Set Comment %1").arg( boardWidget->toString(node) ) );
    node->comment = comment;
    boardWidget->modifyNode(node);
}

void SetCommentCommand::undo(){
    node->comment = oldComment;
    boardWidget->modifyNode(node);
}

MovePositionCommand::MovePositionCommand(BoardWidget* _boardWidget, go::nodePtr _node, const go::point& _pos, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
    , pos(_pos)
{
    oldPos = node->position;
}

void MovePositionCommand::redo(){
    setText( tr("Move Position %1").arg( boardWidget->toString(node) ) );
    node->position.x = pos.x;
    node->position.y = pos.y;
}

void MovePositionCommand::undo(){
    node->position.x = oldPos.x;
    node->position.y = oldPos.y;
}

MoveStoneCommand::MoveStoneCommand(BoardWidget* _boardWidget, go::nodePtr _node, go::stone* _stone, const go::point& _pos, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
    , stone(_stone)
    , pos(_pos)
{
    oldPos = stone->p;
}

void MoveStoneCommand::redo(){
    setText( tr("Move Stone %1").arg( boardWidget->toString(node) ) );
    stone->p = pos;
}

void MoveStoneCommand::undo(){
    stone->p = oldPos;
}

MoveMarkCommand::MoveMarkCommand(BoardWidget* _boardWidget, go::nodePtr _node, go::mark* _mark, const go::point& _pos, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , node(_node)
    , mark(_mark)
    , pos(_pos)
{
    oldPos = mark->p;
}

void MoveMarkCommand::redo(){
    setText( tr("Move Mark %1").arg( boardWidget->toString(node) ) );
    mark->p = pos;
}

void MoveMarkCommand::undo(){
    mark->p = oldPos;
}

RotateSgfCommand::RotateSgfCommand(BoardWidget* _boardWidget, const QString& _commandName, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , commandName(_commandName)
{
}

void RotateSgfCommand::redo(){
    QUndoCommand::redo();
    setText(commandName);

    boardWidget->createBoardBuffer();
    boardWidget->paintBoard();
}

void RotateSgfCommand::undo(){
    QUndoCommand::undo();
    boardWidget->createBoardBuffer();
    boardWidget->paintBoard();
}
