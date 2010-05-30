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
#ifndef COMMAND_H
#define COMMAND_H


#include <QUndoCommand>
#include "boardwidget.h"
#include "godata.h"


class AddNodeCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(AddNodeCommand)

public:
    AddNodeCommand(BoardWidget* boardWidget, go::nodePtr parentNode, go::nodePtr childNode, bool select, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr parentNode;
    go::nodePtr childNode;
    bool select;
};

class InsertNodeCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(InsertNodeCommand)

public:
    InsertNodeCommand(BoardWidget* boardWidget, go::nodePtr parentNode, int index, go::nodePtr childNode, bool select, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr parentNode;
    go::nodePtr childNode;
    int  index;
    bool select;
};

class DeleteNodeCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(DeleteNodeCommand)

public:
    DeleteNodeCommand(BoardWidget* boardWidget, go::nodePtr node, bool deleteChildren, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
    bool deleteChildren;
    int  index;
};

class SetMoveNumberCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetMoveNumberCommand)

public:
    SetMoveNumberCommand(BoardWidget* boardWidget, go::nodePtr node, int moveNumber, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
    int moveNumber;
    int oldMoveNumber;
};

class UnsetMoveNumberCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(UnsetMoveNumberCommand)

public:
    UnsetMoveNumberCommand(BoardWidget* boardWidget, go::nodePtr node, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
    int oldMoveNumber;
};

class SetNodeNameCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetNodeNameCommand)

public:
    SetNodeNameCommand(BoardWidget* boardWidget, go::nodePtr node, const QString& nodeName, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
    QString nodeName;
    QString oldNodeName;
};

class SetCommentCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetCommentCommand)

public:
    SetCommentCommand(BoardWidget* boardWidget, go::nodePtr node, const QString& comment, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr node;
    QString comment;
    QString oldComment;
};

class MovePositionCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(MovePositionCommand)

public:
    MovePositionCommand(BoardWidget* boardWidget, go::nodePtr node, const go::point& pos, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr  node;
    go::point    pos;
    go::point    oldPos;
};

class MoveStoneCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(MoveStoneCommand)

public:
    MoveStoneCommand(BoardWidget* boardWidget, go::nodePtr node, go::stone* stone, const go::point& pos, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr  node;
    go::stone*   stone;
    go::point    pos;
    go::point    oldPos;
};

class MoveMarkCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(MoveMarkCommand)

public:
    MoveMarkCommand(BoardWidget* boardWidget, go::nodePtr node, go::mark* mark, const go::point& pos, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    go::nodePtr  node;
    go::mark*    mark;
    go::point    pos;
    go::point    oldPos;
};

class RotateSgfCommand : public QUndoCommand{
public:
    RotateSgfCommand(BoardWidget* boardWidget, const QString& commandName, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    QString commandName;
};



#endif // COMMAND_H
