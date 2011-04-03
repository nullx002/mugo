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
#ifndef SGFCOMMAND_H
#define SGFCOMMAND_H


#include <QCoreApplication>
#include <QUndoCommand>
#include "godata.h"
#include "boardwidget.h"

class GoDocument;


/**
  @defgroup Command
*/

/**
  @ingroup Command
  This command creates new node in paretNode
*/
class AddNodeCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(AddNodeCommand)

public:
    AddNodeCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr parentNode, Go::NodePtr node, int index, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr parentNode_;
    Go::NodePtr node_;
    int index_;
};

/**
  @ingroup Command
  This command deletes new node in paretNode
*/
class RemoveNodeCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(RemoveNodeCommand)

public:
    RemoveNodeCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr parentNode_;
    Go::NodePtr node_;
    int index_;
};

/**
  @ingroup Command
  This command sets comment to specified node
*/
class SetCommentCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(AddNodeCommand)

public:
    SetCommentCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, const QString& comment, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

    void setComment(const QString& comment);
    const QString& comment() const{ return comment_; }

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    QString comment_;
    QString prevComment_;
};

/**
  @ingroup Command
  This command changes game information to game node
*/
class SetGameInformationCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetGameInformationCommand)
public:
    SetGameInformationCommand(GoDocument* doc, Go::NodePtr node, Go::InformationPtr info, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr node_;
    Go::InformationPtr info_;
    Go::InformationPtr prevInfo_;
};

/**
  @ingroup Command
  This command adds game into document (sgf collection)
*/
class AddGameCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(AddGameCommand)

public:
    AddGameCommand(GoDocument* doc, const Go::NodeList& gameList, QUndoCommand* parent = 0);
    AddGameCommand(GoDocument* doc, Go::NodePtr game, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodeList gameList_;
};

/**
  @ingroup Command
  This command game move up in the document (sgf collection)
*/
class MoveUpGameCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(MoveUpGameCommand)

public:
    MoveUpGameCommand(GoDocument* doc, const Go::NodePtr& game, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
};

/**
  @ingroup Command
  This command game move down in the document (sgf collection)
*/
class MoveDownGameCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(MoveDownGameCommand)

public:
    MoveDownGameCommand(GoDocument* doc, const Go::NodePtr& game, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
};

/**
  @ingroup Command
  This command loads game from sgf collection
*/
class ChangeGameCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(ChangeGameCommand)
public:
    ChangeGameCommand(GoDocument* doc, BoardWidget* board, Go::NodePtr game, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    BoardWidget* board_;
    Go::NodePtr game_;
    Go::NodePtr prevGame_;
};



#endif // SGFCOMMAND_H
