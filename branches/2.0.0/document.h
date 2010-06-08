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
#include <QTextCodec>

class QUndoStack;


/**
  Document
*/
class Document : public QObject
{
    Q_OBJECT
public:
    // Constructor
    Document(QTextCodec* codec, QObject* parent=NULL);
    virtual ~Document();

    // open
    virtual bool open(const QString& fname, bool guessCodec) = 0;

    // save
    virtual bool save(const QString& fname) = 0;

    // Get
    QUndoStack* getUndoStack(){ return undoStack; }
    const QString& getDocName() const{ return docName; }
    const QString& getFileName() const{ return fileName; }

    // Set
    void setDocName(const QString& name){ docName = name; }
    void setFileName(const QString& fname){ fileName = fname; }

protected:
    QUndoStack* undoStack;
    QTextCodec* codec;
    QString     docName;
    QString     fileName;
};

#endif // DOCUMENT_H
