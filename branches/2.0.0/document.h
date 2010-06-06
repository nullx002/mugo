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
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QString>
#include <QFileInfo>

class QUndoStack;


/**
  Document
*/
class Document : public QObject
{
    Q_OBJECT
public:
    // Constructor
    Document(QObject* parent=NULL);
    virtual ~Document();

    // Get
    QUndoStack* getUndoStack(){ return undoStack; }
    const QString& getName() const{ return name; }
    const QFileInfo& getFileInfo() const{ return fileInfo; }

    // Set
    void setName(const QString& name_){ name = name_; }
    void setFileInfo(const QFileInfo fi){ fileInfo = fi; }

protected:
    QUndoStack* undoStack;
    QString     name;
    QFileInfo   fileInfo;
};

#endif // DOCUMENT_H
