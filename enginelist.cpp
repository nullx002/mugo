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
#include <QSettings>
#include <QStringList>
#include "enginelist.h"

EngineList::EngineList()
{
}

void EngineList::save(){
    QStringList list;
    foreach (const Engine& e, engines){
        list << e.name << e.path << e.parameters << QString::number(e.analysis)
            << "" << "" << "" << "" << "" << "";  // reserve
    }

    QSettings settings;
    settings.setValue("playWithComputer/engines", list);
}

void EngineList::load(){
    QSettings settings;
    QStringList list = settings.value("playWithComputer/engines").toStringList();

    engines.clear();
    int n = list.size() / 10;
    for (int i=0; i<n; ++i){
        Engine e;
        e.name = list[i*10];
        e.path = list[i*10+1];
        e.parameters = list[i*10+2];
        e.analysis   = list[i*10+3].toInt();
        engines.push_back(e);
    }
}
