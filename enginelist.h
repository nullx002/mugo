#ifndef ENGINELIST_H
#define ENGINELIST_H

#include <QList>
#include <QString>

class Engine{
    public:
        QString path;
        QString parameters;
        QString name;
};

class EngineList{
    public:
        EngineList();

        void save();
        void load();

        QList<Engine> engines;
};

#endif // ENGINELIST_H
