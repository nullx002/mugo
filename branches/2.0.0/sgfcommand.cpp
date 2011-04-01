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
