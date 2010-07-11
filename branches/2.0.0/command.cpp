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
#include <QTreeView>
#include <QStandardItemModel>
#include "command.h"
#include "boardwidget.h"
#include "sgfdocument.h"

/**
  Constructor
*/
AddNodeCommand::AddNodeCommand(SgfDocument* doc, Go::NodePtr parentNode_, Go::NodePtr node_, int index_, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document(doc)
    , parentNode(parentNode_)
    , node(node_)
    , index(index_)
{
}

/**
  redo add node command
*/
void AddNodeCommand::redo(){
    if (node->isStone())
        setText( tr("Move") );
    else
        setText( tr("Add Empty Node") );
    document->addNode(parentNode, node, index);
}

/**
  undo add node command
*/
void AddNodeCommand::undo(){
    document->deleteNode(node, true);
}


/**
  Constructor
*/
DeleteNodeCommand::DeleteNodeCommand(SgfDocument* doc, Go::NodePtr node_, bool removeChildren_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , removeChildren(removeChildren_)
{
    parentNode = node->parent();
    pos = parentNode->childNodes.indexOf(node);
}

/**
  redo delete node command
*/
void DeleteNodeCommand::redo(){
    setText( tr("Delete Stone") );
    document->deleteNode(node, removeChildren);
}

/**
  undo delete node command
*/
void DeleteNodeCommand::undo(){
    if (removeChildren == false){
        foreach(Go::NodePtr child, node->childNodes){
            Go::NodeList::iterator iter = qFind(parentNode->childNodes.begin(), parentNode->childNodes.end(), child);
            if (iter != parentNode->childNodes.end()){
                (*iter)->setParent(node);
                parentNode->childNodes.erase(iter);
            }
        }
    }
    document->addNode(parentNode, node, pos);
}

/**
  Constructor
*/
SetNodeNameCommand::SetNodeNameCommand(SgfDocument* doc, Go::NodePtr node_, const QString& name, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , newName(name)
{
    oldName = node->name;
}

/**
  redo set node name command
*/
void SetNodeNameCommand::redo(){
    setText( tr("Set Node Name") );
    node->name = newName;
    document->modifyNode(node, false);
}

/**
  undo set node name command
*/
void SetNodeNameCommand::undo(){
    node->name = oldName;
    document->modifyNode(node, false);
}

/**
  Constructor
*/
SetMoveNumberCommand::SetMoveNumberCommand(SgfDocument* doc, Go::NodePtr node_, int moveNumber, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , newNumber(moveNumber)
{
    oldNumber = node->moveNumber;
}

/**
  redo set move number command
*/
void SetMoveNumberCommand::redo(){
    setText( tr("Set Move Number") );
    node->moveNumber = newNumber;
    document->modifyNode(node, false);
}

/**
  undo set move number command
*/
void SetMoveNumberCommand::undo(){
    node->moveNumber = oldNumber;
    document->modifyNode(node, false);
}

/**
  Constructor
*/
UnsetMoveNumberCommand::UnsetMoveNumberCommand(SgfDocument* doc, Go::NodePtr node_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
{
    oldNumber = node->moveNumber;
}

/**
  redo unset move number command
*/
void UnsetMoveNumberCommand::redo(){
    setText( tr("Unset Move Number") );
    node->moveNumber = 0;
    document->modifyNode(node, false);
}

/**
  undo unset move number command
*/
void UnsetMoveNumberCommand::undo(){
    node->moveNumber = oldNumber;
    document->modifyNode(node, false);
}

/**
  Constructor
*/
SetNodeAnnotationCommand::SetNodeAnnotationCommand(SgfDocument* doc, Go::NodePtr node_, Go::Node::NodeAnnotation annotation, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , newAnnotation(annotation)
{
    oldAnnotation = node->nodeAnnotation;
}

/**
  redo set node annotation command
*/
void SetNodeAnnotationCommand::redo(){
    setText( tr("Set Node Annotation") );
    node->nodeAnnotation = newAnnotation;
    document->modifyNode(node, false);
}

/**
  undo set node annotation command
*/
void SetNodeAnnotationCommand::undo(){
    node->nodeAnnotation = oldAnnotation;
    document->modifyNode(node, false);
}

/**
  Constructor
*/
SetMoveAnnotationCommand::SetMoveAnnotationCommand(SgfDocument* doc, Go::NodePtr node_, Go::Node::MoveAnnotation annotation, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , newAnnotation(annotation)
{
    oldAnnotation = node->moveAnnotation;
}

/**
  redo set node annotation command
*/
void SetMoveAnnotationCommand::redo(){
    setText( tr("Set Move Annotation") );
    node->moveAnnotation = newAnnotation;
    document->modifyNode(node, false);
}

/**
  undo set node annotation command
*/
void SetMoveAnnotationCommand::undo(){
    node->moveAnnotation = oldAnnotation;
    document->modifyNode(node, false);
}

/**
  Constructor
*/
SetAnnotationCommand::SetAnnotationCommand(SgfDocument* doc, Go::NodePtr node_, Go::Node::Annotation annotation, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , newAnnotation(annotation)
{
    oldAnnotation = node->annotation;
}

/**
  redo set annotation command
*/
void SetAnnotationCommand::redo(){
    setText( tr("Set Annotation") );
    node->annotation = newAnnotation;
    document->modifyNode(node, false);
}

/**
  undo set annotation command
*/
void SetAnnotationCommand::undo(){
    node->annotation = oldAnnotation;
    document->modifyNode(node, false);
}

/**
  Constructor
*/
AddMarkCommand::AddMarkCommand(SgfDocument* doc, Go::NodePtr node_, const Go::Mark& mark_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , mark(mark_)
{
    if (mark.type == Go::Mark::circle)
        setText( tr("Add Circle") );
    else if (mark.type == Go::Mark::triangle)
        setText( tr("Add Triangle") );
    else if (mark.type == Go::Mark::square)
        setText( tr("Add Square") );
    else if (mark.type == Go::Mark::cross)
        setText( tr("Add Cross") );
    else if (mark.type == Go::Mark::character)
        setText( tr("Add Label") );
    else
        setText( tr("Add Mark") );
}

/**
  redo add mark command
*/
void AddMarkCommand::redo(){
    node->marks.push_back(mark);
    document->modifyNode(node, true);
}

/**
  undo add mark command
*/
void AddMarkCommand::undo(){
    node->marks.pop_back();
    document->modifyNode(node, true);
}

/**
  Constructor
*/
RemoveMarkCommand::RemoveMarkCommand(SgfDocument* doc, Go::NodePtr node_, const Go::Point& p, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , position(p)
    , mark(NULL)
{
    setText( tr("Remove Mark") );

    index = 0;
    foreach(const Go::Mark& m, node->marks){
        if (m.position == position){
            mark = new Go::Mark(m);
            break;
        }
        ++index;
    }
}

/**
  Destructor
*/
RemoveMarkCommand::~RemoveMarkCommand(){
    delete mark;
}

/**
  redo remove mark command
*/
void RemoveMarkCommand::redo(){
    node->marks.removeAt(index);
    document->modifyNode(node, true);
}

/**
  undo remove mark command
*/
void RemoveMarkCommand::undo(){
    node->marks.insert(index, *mark);
    document->modifyNode(node, true);
}

/**
  Constructor
*/
AddStoneCommand::AddStoneCommand(SgfDocument* doc, Go::NodePtr node_, const Go::Stone& stone_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , stone(stone_)
{
    if (stone.color == Go::black)
        setText( tr("Add Black Stone") );
    else if (stone.color == Go::white)
        setText( tr("Add White Stone") );
    else if (stone.color == Go::empty)
        setText( tr("Add Empty") );
}

/**
  redo add stone command
*/
void AddStoneCommand::redo(){
    if (stone.color == Go::black)
        node->blackStones.push_back(stone);
    else if (stone.color == Go::white)
        node->whiteStones.push_back(stone);
    else if (stone.color == Go::empty)
        node->emptyStones.push_back(stone);

    document->modifyNode(node, true);
}

/**
  undo add stone command
*/
void AddStoneCommand::undo(){
    if (stone.color == Go::black)
        node->blackStones.pop_back();
    else if (stone.color == Go::white)
        node->whiteStones.pop_back();
    else if (stone.color == Go::empty)
        node->emptyStones.pop_back();

    document->modifyNode(node, true);
}

/**
  Constructor
*/
RemoveStoneCommand::RemoveStoneCommand(SgfDocument* doc, Go::NodePtr node_, const Go::Point& p, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , position(p)
    , stone(NULL)
{
    setText( tr("Remove Stone") );

    index = 0;
    foreach(const Go::Stone& s, node->blackStones){
        if (s.position == position){
            stone = new Go::Stone(s);
            return;
        }
        ++index;
    }

    index = 0;
    foreach(const Go::Stone& s, node->whiteStones){
        if (s.position == position){
            stone = new Go::Stone(s);
            return;
        }
        ++index;
    }

    index = 0;
    foreach(const Go::Stone& s, node->emptyStones){
        if (s.position == position){
            stone = new Go::Stone(s);
            return;
        }
        ++index;
    }
}

/**
  Destructor
*/
RemoveStoneCommand::~RemoveStoneCommand(){
    delete stone;
}

/**
  redo remove stone  command
*/
void RemoveStoneCommand::redo(){
    if (stone->color == Go::black)
        node->blackStones.removeAt(index);
    else if (stone->color == Go::white)
        node->whiteStones.removeAt(index);
    else if (stone->color == Go::empty)
        node->emptyStones.removeAt(index);

    document->modifyNode(node, true);
}

/**
  undo remove stone command
*/
void RemoveStoneCommand::undo(){
    if (stone->color == Go::black)
        node->blackStones.insert(index, *stone);
    else if (stone->color == Go::white)
        node->whiteStones.insert(index, *stone);
    else if (stone->color == Go::empty)
        node->emptyStones.insert(index, *stone);

    document->modifyNode(node, true);
}

/**
  Constructor
*/
SetCommentCommand::SetCommentCommand(SgfDocument* doc, Go::NodePtr node_, const QString& comment_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , node(node_)
    , comment(comment_)
{
    oldComment = node->comment;
}

/**
  redo set comment command
*/
void SetCommentCommand::redo(){
    setText( tr("Edit comment") );
    node->comment = comment;
    document->modifyNode(node, false);
}

/**
  undo set comment command
*/
void SetCommentCommand::undo(){
    node->comment = oldComment;
    document->modifyNode(node, false);
}

/**
  set comment text
*/
void SetCommentCommand::setComment(const QString& comment){
    node->comment = this->comment = comment;
    document->modifyNode(node, false);
}

/**
  Constructor
*/
SetCurrentGameCommand::SetCurrentGameCommand(BoardWidget* board, Go::NodePtr game_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , boardWidget(board)
    , game(game_)
{
}

/**
  redo set current game command
*/
void SetCurrentGameCommand::redo(){
    setText( tr("Set current game") );

    prevGame = boardWidget->getCurrentGame();
    boardWidget->setCurrentGame(game);
}

/**
  undo set current game command
*/
void SetCurrentGameCommand::undo(){
    boardWidget->setCurrentGame(prevGame);
}

/**
  Constructor
*/
AddGameListCommand::AddGameListCommand(SgfDocument* doc, Go::NodeList& gameList_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , gameList(gameList_)
{
}

/**
  redo add gamelist command
*/
void AddGameListCommand::redo(){
    setText( tr("Add games into collection") );
    foreach(const Go::NodePtr& game, gameList)
        document->addGame(game, -1);
}

/**
  undo add agmelist command
*/
void AddGameListCommand::undo(){
    foreach(const Go::NodePtr& game, gameList)
        document->deleteGame(game);
}

/**
  Constructor
*/
DeleteGameListCommand::DeleteGameListCommand(SgfDocument* doc, Go::NodeList& gameList_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , gameList(gameList_)
{
}

/**
  redo delete game from collection command
*/
void DeleteGameListCommand::redo(){
    setText( tr("Delete games from collection") );

    indexList.clear();
    foreach(const Go::NodePtr& game, gameList){
        indexList.push_back( document->gameList.indexOf(game) );
        document->deleteGame(game);
    }
}

/**
  undo delete game from collection command
*/
void DeleteGameListCommand::undo(){
    for (int i=gameList.size()-1; i>=0; --i){
        Go::NodePtr& game = gameList[i];
        document->addGame(game, indexList[i]);
    }
}

/**
  Constructor
*/
MoveUpInCollectionCommand::MoveUpInCollectionCommand(SgfDocument* doc, Go::NodePtr& game_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , game(game_)
    , moved(false)
{
}

/**
  redo move up sgf in collection command
*/
void MoveUpInCollectionCommand::redo(){
    setText( tr("Move up sgf in collection") );

    moved = document->moveUp(game);
}

/**
  undo move up sgf in collection command
*/
void MoveUpInCollectionCommand::undo(){
    if (moved == false)
        return;

    document->moveDown(game);
}

/**
  Constructor
*/
MoveDownInCollectionCommand::MoveDownInCollectionCommand(SgfDocument* doc, Go::NodePtr& game_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , game(game_)
    , moved(false)
{
}

/**
  redo move down sgf in collection command
*/
void MoveDownInCollectionCommand::redo(){
    setText( tr("Move down sgf in collection") );

    moved = document->moveDown(game);
}

/**
  undo move down sgf in collection command
*/
void MoveDownInCollectionCommand::undo(){
    if (moved == false)
        return;

    document->moveUp(game);
}

/**
  Constructor
*/
SetGameInformationCommand::SetGameInformationCommand(SgfDocument* doc, Go::GameInformationPtr gameInfo_, Go::GameInformationPtr newInfo_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , gameInfo(gameInfo_)
    , newInfo(newInfo_)
    , oldInfo(new Go::GameInformation)
{
    *oldInfo = *gameInfo;
}

/**
  redo set comment command
*/
void SetGameInformationCommand::redo(){
    setText( tr("Set game information") );
    *gameInfo = *newInfo;
    document->setDirty();
}

/**
  undo set comment command
*/
void SetGameInformationCommand::undo(){
    *gameInfo = *oldInfo;
    document->setDirty();
}
