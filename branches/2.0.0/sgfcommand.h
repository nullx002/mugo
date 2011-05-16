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
    RemoveNodeCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, bool removeChildren, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr parentNode_;
    Go::NodePtr node_;
    bool removeChildren_;
    int index_;
    Go::NodeList children_;
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
  This command adds stone into specified node
*/
class AddStoneCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(AddStoneCommand)

public:
    AddStoneCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, int x, int y, const Go::Color color, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    int x_;
    int y_;
    Go::Color color_;
};

/**
  @ingroup Command
  This command adds stone into specified node
*/
class RemoveStoneCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(RemoveStoneCommand)

public:
    RemoveStoneCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, QList<QPoint>& stones, const QPoint& pos, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    QList<QPoint>& stones_;
    QPoint pos_;
    int index_;
};

/**
  @ingroup Command
  This command adds mark into specified node
*/
class AddMarkCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(AddMarkCommand)

public:
    AddMarkCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, int x, int y, const Go::Mark::Type mark, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    int x_;
    int y_;
    Go::Mark::Type mark_;
};

/**
  @ingroup Command
  This command adds label into specified node
*/
class AddLabelCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(AddLabelCommand)

public:
    AddLabelCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, int x, int y, const QString& label, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    int x_;
    int y_;
    QString label_;
};

/**
  @ingroup Command
  This command remove mark from specified node
*/
class RemoveMarkCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(RemoveMarkCommand)

public:
    RemoveMarkCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, Go::MarkList& markList, const Go::Mark& mark, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    Go::MarkList& markList_;
    Go::Mark mark_;
    int index_;
};


/**
  @ingroup Command
  This command set move annotation to specified node
*/
class SetMoveAnnotationCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetMoveAnnotationCommand)

public:
    SetMoveAnnotationCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, Go::Node::MoveAnnotation annotation, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    Go::Node::MoveAnnotation annotation_;
    Go::Node::MoveAnnotation prevAnnotation_;
};

/**
  @ingroup Command
  This command set node annotation to specified node
*/
class SetNodeAnnotationCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetNodeAnnotationCommand)

public:
    SetNodeAnnotationCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, Go::Node::NodeAnnotation annotation, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    Go::Node::NodeAnnotation annotation_;
    Go::Node::NodeAnnotation prevAnnotation_;
};

/**
  @ingroup Command
  This command set node annotation2 to specified node
*/
class SetNodeAnnotation2Command : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetNodeAnnotation2Command)

public:
    SetNodeAnnotation2Command(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, Go::Node::NodeAnnotation2 annotation, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    Go::Node::NodeAnnotation2 annotation_;
    Go::Node::NodeAnnotation2 prevAnnotation_;
};

/**
  @ingroup Command
  This command set move number to specified node
*/
class SetMoveNumberCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetMoveNumberCommand)

public:
    SetMoveNumberCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, int moveNumber, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    int moveNumber_;
    int prevMoveNumber_;
};

/**
  @ingroup Command
  This command set node name to specified node
*/
class SetNodeNameCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetNodeNameCommand)

public:
    SetNodeNameCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, const QString& nodeName, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    QString nodeName_;
    QString prevNodeName_;
};

/**
  @ingroup Command
  This command set next color to specified node
*/
class SetNextColorCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetNextColorCommand)

public:
    SetNextColorCommand(GoDocument* doc, Go::NodePtr game, Go::NodePtr node, Go::Color color, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    GoDocument* document_;
    Go::NodePtr game_;
    Go::NodePtr node_;
    Go::Color color_;
    Go::Color prevColor_;
};

/**
  @ingroup Command
  This command rotate sgf nodes
*/
class RotateClockwiseCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(SetRotateClockwiseCommand)

public:
    RotateClockwiseCommand(GoDocument* doc, Go::NodePtr game, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();

private:
    void rotate(Go::NodePtr node, bool clockwise);
    void getRotatedPosition(int x, int y, int& newX, int& newY, bool clockwise);

    GoDocument* document_;
    Go::NodePtr game_;
};

/**
  @ingroup Command
  This class is base class of flip command
*/
class FlipCommand : public QUndoCommand{
    Q_DECLARE_TR_FUNCTIONS(FlipCommand)

public:
    FlipCommand(GoDocument* doc, Go::NodePtr game, QUndoCommand* parent = 0);

protected:
    void flip(Go::NodePtr node, bool vertical);
    void getFlippedPosition(int x, int y, int& newX, int& newY, bool vertical);

    GoDocument* document_;
    Go::NodePtr game_;
};

/**
  @ingroup Command
  This command flip horizontally sgf nodes
*/
class FlipHorizontallyCommand : public FlipCommand{
    Q_DECLARE_TR_FUNCTIONS(FlipHorizontallyCommand)

public:
    FlipHorizontallyCommand(GoDocument* doc, Go::NodePtr game, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();
};

/**
  @ingroup Command
  This command flip vertically sgf nodes
*/
class FlipVerticallyCommand : public FlipCommand{
    Q_DECLARE_TR_FUNCTIONS(FlipVerticallyCommand)

public:
    FlipVerticallyCommand(GoDocument* doc, Go::NodePtr game, QUndoCommand* parent = 0);
    virtual void redo();
    virtual void undo();
};


#endif // SGFCOMMAND_H
