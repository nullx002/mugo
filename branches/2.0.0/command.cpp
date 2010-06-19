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
MoveUpInCollectionCommand::MoveUpInCollectionCommand(SgfDocument* doc, QTreeView* view_, int row_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , view(view_)
    , row(row_)
    , moved(false)
{
    game = doc->gameList[row_];
}

/**
  redo move up sgf in collection command
*/
void MoveUpInCollectionCommand::redo(){
    setText( tr("Move up sgf in collection") );

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(view->model());
    if (model == NULL)
        return;

    QList<QStandardItem*> items = model->takeRow(row);
    model->insertRow(row-1, items);
    view->setCurrentIndex( model->index(row-1, 0) );

    moved = document->moveUp(game);
}

/**
  undo move up sgf in collection command
*/
void MoveUpInCollectionCommand::undo(){
    if (moved == false)
        return;

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(view->model());
    if (model == NULL)
        return;

    QList<QStandardItem*> items = model->takeRow(row-1);
    model->insertRow(row, items);
    view->setCurrentIndex( model->index(row, 0) );

    document->moveDown(game);
}

/**
  Constructor
*/
MoveDownInCollectionCommand::MoveDownInCollectionCommand(SgfDocument* doc, QTreeView* view_, int row_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , view(view_)
    , row(row_)
    , moved(false)
{
    game = doc->gameList[row_];
}

/**
  redo move down sgf in collection command
*/
void MoveDownInCollectionCommand::redo(){
    setText( tr("Move down sgf in collection") );

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(view->model());
    if (model == NULL)
        return;

    QList<QStandardItem*> items = model->takeRow(row);
    model->insertRow(row+1, items);
    view->setCurrentIndex( model->index(row+1, 0) );

    moved = document->moveDown(game);
}

/**
  undo move down sgf in collection command
*/
void MoveDownInCollectionCommand::undo(){
    if (moved == false)
        return;

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(view->model());
    if (model == NULL)
        return;

    QList<QStandardItem*> items = model->takeRow(row+1);
    model->insertRow(row, items);
    view->setCurrentIndex( model->index(row, 0) );

    document->moveUp(game);
}

/**
  Constructor
*/
DeleteGameFromCollectionCommand::DeleteGameFromCollectionCommand(SgfDocument* doc, QStandardItemModel* model_, int row_, QUndoCommand *parent)
    : QUndoCommand(parent)
    , document(doc)
    , model(model_)
    , row(row_)
    , removed(false)
{
    game = doc->gameList[row_];
}

/**
  redo delete game from collection command
*/
void DeleteGameFromCollectionCommand::redo(){
    setText( tr("Delete sgf from collection") );

    Go::NodeList::iterator iter = qFind(document->gameList.begin(), document->gameList.end(), game);
    if (iter == document->gameList.end())
        return;

    iterator = document->gameList.erase(iter);
    items = model->takeRow(row);

    document->setDirty();
    removed = true;
}

/**
  undo delete game from collection command
*/
void DeleteGameFromCollectionCommand::undo(){
    if (removed == false)
        return;

    document->gameList.insert(iterator, game);
    model->insertRow(row, items);

    document->setDirty();
}
