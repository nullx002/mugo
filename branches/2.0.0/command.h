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
#include <QString>
#include <QStandardItem>
#include "godata.h"
#include "sgf.h"


class QTreeView;
class QStandardItemModel;
class SgfDocument;
class BoardWidget;

/**
  add node command
*/
class AddNodeCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(AddNodeCommand)

public:
    AddNodeCommand(SgfDocument* doc, Go::NodePtr parentNode, Go::NodePtr node, int index, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodePtr parentNode;
    Go::NodePtr node;
    int index;
};

/**
  delete node command
*/
class DeleteNodeCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(DeleteNodeCommand)

public:
    DeleteNodeCommand(SgfDocument* doc, Go::NodePtr node, bool removeChildren, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodePtr parentNode;
    Go::NodePtr node;
    bool removeChildren;
    int pos;
};

/**
  set node name command
*/
class SetNodeNameCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetNodeNameCommand)

public:
    SetNodeNameCommand(SgfDocument* doc, Go::NodePtr node, const QString& name, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodePtr  node;
    QString newName;
    QString oldName;
};

/**
  set move number command
*/
class SetMoveNumberCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetMoveNumberCommand)

public:
    SetMoveNumberCommand(SgfDocument* doc, Go::NodePtr node, int moveNumber, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodePtr  node;
    int newNumber;
    int oldNumber;
};

/**
  unset move number command
*/
class UnsetMoveNumberCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(UnsetMoveNumberCommand)

public:
    UnsetMoveNumberCommand(SgfDocument* doc, Go::NodePtr node, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodePtr  node;
    int oldNumber;
};

/**
  set node annotation command
*/
class SetNodeAnnotationCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetNodeAnnotationCommand)

public:
    SetNodeAnnotationCommand(SgfDocument* doc, Go::NodePtr node, Go::Node::NodeAnnotation annotation, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodePtr  node;
    Go::Node::NodeAnnotation newAnnotation;
    Go::Node::NodeAnnotation oldAnnotation;
};

/**
  set move annotation command
*/
class SetMoveAnnotationCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetMoveAnnotationCommand)

public:
    SetMoveAnnotationCommand(SgfDocument* doc, Go::NodePtr node, Go::Node::MoveAnnotation annotation, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodePtr  node;
    Go::Node::MoveAnnotation newAnnotation;
    Go::Node::MoveAnnotation oldAnnotation;
};

/**
  set annotation command
*/
class SetAnnotationCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetAnnotationCommand)

public:
    SetAnnotationCommand(SgfDocument* doc, Go::NodePtr node, Go::Node::Annotation annotation, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodePtr  node;
    Go::Node::Annotation newAnnotation;
    Go::Node::Annotation oldAnnotation;
};

/**
  set comment command
*/
class SetCommentCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetCommentCommand)

public:
    SetCommentCommand(SgfDocument* doc, Go::NodePtr node, const QString& comment, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

    void setComment(const QString& comment);
    Go::NodePtr getNode(){ return node; }

private:
    SgfDocument* document;
    Go::NodePtr  node;
    QString      comment;
    QString      oldComment;
};

/**
  set game information command
*/
class SetGameInformationCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetGameInformationCommand)

public:
    SetGameInformationCommand(SgfDocument* doc, Go::GameInformationPtr gameInfo, Go::GameInformationPtr newInfo, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodePtr node;
    Go::GameInformationPtr gameInfo;
    Go::GameInformationPtr newInfo;
    Go::GameInformationPtr oldInfo;
};

/**
  set current game command
*/
class SetCurrentGameCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetCurrentGameCommand)

public:
    SetCurrentGameCommand(BoardWidget* board, Go::NodePtr game, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    BoardWidget* boardWidget;
    Go::NodePtr game;
    Go::NodePtr prevGame;
};

/**
  add games command
*/
class AddGameListCommand: public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(AddGameListCommand)

public:
    AddGameListCommand(SgfDocument* doc, Go::NodeList& gameList, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodeList gameList;
};

/**
  delete games from gamelist command
*/
class DeleteGameListCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(DeleteGameCommand)

public:
    DeleteGameListCommand(SgfDocument* doc, Go::NodeList& gameList, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodeList gameList;
    QList<int> indexList;
};

/**
  move up sgf in collection command
*/
class MoveUpInCollectionCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(MoveUpInCollectionCommand)

public:
    MoveUpInCollectionCommand(SgfDocument* doc, Go::NodePtr& game, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodePtr game;
    bool moved;
};

/**
  move down sgf in collection command
*/
class MoveDownInCollectionCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(MoveDownInCollectionCommand)

public:
    MoveDownInCollectionCommand(SgfDocument* doc, Go::NodePtr& game, QUndoCommand *parent = 0);
    virtual void redo();
    virtual void undo();

private:
    SgfDocument* document;
    Go::NodePtr game;
    bool moved;
};

#endif // COMMAND_H
