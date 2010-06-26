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
#include "mugoapp.h"
#include "mainwindow.h"


/**
  Constructor
*/
Application::Application(int& argc, char** argv) : QApplication(argc, argv){
    setApplicationName(SETTING_NAME);
    setApplicationVersion(APP_VERSION);
    setOrganizationDomain(AUTHOR);
}


int main(int argc, char *argv[])
{
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName("UTF-8") );
    QTextCodec::setCodecForTr( QTextCodec::codecForName("UTF-8") );

    Application a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
