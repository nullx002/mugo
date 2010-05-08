#include <QSettings>
#include <QStringList>
#include "enginelist.h"

EngineList::EngineList()
{
}

void EngineList::save(){
    QStringList list;
    foreach (const Engine& e, engines){
        list << e.name << e.path << e.parameters
            << "" << "" << "" << "" << "" << "" << "";  // reserve
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
        engines.push_back(e);
    }
}
