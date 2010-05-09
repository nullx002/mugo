#ifndef MUGO_APPLICATION_H
#define MUGO_APPLICATION_H

#include "qtsingleapplication.h"
#include "appdef.h"

class MainWindow;

class Application : public QtSingleApplication{
Q_OBJECT
public:
    Application(int argc, char** argv) : QtSingleApplication(argc, argv), mainWindow(NULL){
        connect(this, SIGNAL(messageReceived(const QString&)), SLOT(received(const QString&)));
    }
    virtual ~Application();

#if defined(Q_WS_MAC)
    bool event(QEvent*);
#endif

    void setMainWindow(MainWindow* win);

public slots:
    void received(const QString& msg);

private:
    MainWindow* mainWindow;
};


#endif
