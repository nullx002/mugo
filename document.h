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
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QUndoStack>
#include <QFileInfo>

/**
  Base class of mugo document.
*/
class Document : public QObject
{
    Q_OBJECT
public:
    // constructor, destructor
    explicit Document(QObject *parent = 0);
    virtual ~Document(){}

    // undo stack
    QUndoStack* undoStack(){ return &undoStack_; }

    // document name
    const QString& name() const{ return name_; }
    void setName(const QString& name){ name_ = name; }

    // file info
    const QFileInfo& fileInfo() const{ return fileInfo_; }
    void setFileInfo(const QFileInfo& fileInfo);

    // dirty
    bool dirty() const{ return dirty_; }
    void setDirty(bool dirty=true);

    // codec
    QTextCodec* codec(){ return codec_; }
    void setCodec(QTextCodec* codec){ codec_ = codec; }

signals:
    void dirtyChanged(bool dirty);

public slots:

protected:
    QUndoStack undoStack_;
    QString name_;
    QFileInfo fileInfo_;
    bool dirty_;
    QTextCodec* codec_;
};

#endif // DOCUMENT_H
