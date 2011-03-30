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
#include "mugoapp.h"
#include "document.h"

/**
  Constructs an empty document.
*/
Document::Document(QObject* parent)
    : QObject(parent)
    , dirty_(false)
    , codec_(mugoApp()->defaultCodec())
{
}

/**
  set file info and set base name to document name
*/
void Document::setFileInfo(const QFileInfo& fileInfo){
    fileInfo_ = fileInfo;
    name_ = fileInfo.baseName();
}

/**
  set dirty flag
*/
void Document::setDirty(bool dirty){
    if (dirty_ == dirty)
        return;

    dirty_ = dirty;
    emit dirtyChanged(dirty);
}
