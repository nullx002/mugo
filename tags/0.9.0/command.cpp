#include <QDebug>
#include "command.h"


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
    setText( QString(tr("Add %1")).arg( boardWidget->toString(childNode) ) );
    boardWidget->addNode(parentNode, childNode, select);
}

void AddNodeCommand::undo(){
    boardWidget->deleteNode(childNode);
}

/**
* Insert Node Command
*/
InsertNodeCommand::InsertNodeCommand(BoardWidget* _boardWidget, go::nodePtr _parentNode, go::nodePtr _childNode, bool _select, QUndoCommand* parent)
    : QUndoCommand(parent)
    , boardWidget(_boardWidget)
    , parentNode(_parentNode)
    , childNode(_childNode)
    , select(_select)
{
}

void InsertNodeCommand::redo(){
    setText( QString(tr("Insert %1")).arg( boardWidget->toString(childNode) ) );
    childNode->childNodes += parentNode->childNodes;
    parentNode->childNodes.clear();

    go::nodeList::iterator iter = childNode->childNodes.begin();
    while (iter != childNode->childNodes.end()){
        (*iter)->parent = childNode;
        ++iter;
    }

    boardWidget->addNode(parentNode, childNode, select);
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
    setText( QString(tr("Delete %1")).arg( boardWidget->toString(node) ) );
    boardWidget->deleteNode(node, deleteChildren);
}

void DeleteNodeCommand::undo(){
    boardWidget->addNode(node->parent, node);
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
    setText( QString(tr("Set Move Number %1")).arg( boardWidget->toString(node) ) );
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
    setText( QString(tr("Set Node Name %1")).arg( boardWidget->toString(node) ) );
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
    setText( QString(tr("Set Comment %1")).arg( boardWidget->toString(node) ) );
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
    setText( QString(tr("Move Position %1")).arg( boardWidget->toString(node) ) );
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
    setText( QString(tr("Move Stone %1")).arg( boardWidget->toString(node) ) );
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
    setText( QString(tr("Move Mark %1")).arg( boardWidget->toString(node) ) );
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
    boardWidget->repaintBoard();
}

void RotateSgfCommand::undo(){
    QUndoCommand::undo();
    boardWidget->createBoardBuffer();
    boardWidget->repaintBoard();
}
