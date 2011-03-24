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
AddNodeCommand::AddNodeCommand(GoDocument* doc, Go::NodePtr parentNode, Go::NodePtr node, int index, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
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
    document_->addNode(parentNode_, node_, index_);
}

/**
  undo add node command
*/
void AddNodeCommand::undo(){
    document_->deleteNode(node_);
}

/**
  Constructs set comment command
*/
SetCommentCommand::SetCommentCommand(GoDocument* doc, Go::NodePtr node, const QString& comment, QUndoCommand* parent)
    : QUndoCommand(parent)
    , document_(doc)
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
    document_->modifyNode(node_);
}

/**
  undo set comment command
*/
void SetCommentCommand::undo(){
    node_->setComment(prevComment_);
    document_->modifyNode(node_);
}

/**
  set comment
*/
void SetCommentCommand::setComment(const QString& comment){
    comment_ = comment;
    redo();
}
