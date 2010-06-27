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
    setText( tr("Add Stone") );
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
    document->modifyNode(node);
}

/**
  undo set node name command
*/
void SetNodeNameCommand::undo(){
    node->name = oldName;
    document->modifyNode(node);
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
    document->modifyNode(node);
}

/**
  undo set move number command
*/
void SetMoveNumberCommand::undo(){
    node->moveNumber = oldNumber;
    document->modifyNode(node);
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
    document->modifyNode(node);
}

/**
  undo unset move number command
*/
void UnsetMoveNumberCommand::undo(){
    node->moveNumber = oldNumber;
    document->modifyNode(node);
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
    document->modifyNode(node);
}

/**
  undo set node annotation command
*/
void SetNodeAnnotationCommand::undo(){
    node->nodeAnnotation = oldAnnotation;
    document->modifyNode(node);
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
    document->modifyNode(node);
}

/**
  undo set node annotation command
*/
void SetMoveAnnotationCommand::undo(){
    node->moveAnnotation = oldAnnotation;
    document->modifyNode(node);
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
    document->modifyNode(node);
}

/**
  undo set annotation command
*/
void SetAnnotationCommand::undo(){
    node->annotation = oldAnnotation;
    document->modifyNode(node);
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
    document->modifyNode(node);
}

/**
  undo set comment command
*/
void SetCommentCommand::undo(){
    node->comment = oldComment;
    document->modifyNode(node);
}

/**
  set comment text
*/
void SetCommentCommand::setComment(const QString& comment){
    node->comment = this->comment = comment;
    document->modifyNode(node);
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
