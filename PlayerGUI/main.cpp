/**
 * Copyright (C) 2011, BMW AG
 *
 * PlayerGui
 *
 * \file main.cpp
 *
 * \date 20.05.2011
 * \author Christian Müller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG – Christian Müller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 * This Tool serves as test HMI interface in order to test the Genivi AudioManager. It is not intended to be production SW.
 *
 */

#include <QtGui>
#include <QApplication>
#include <QStringList>
#include "playerGui.h"
#include "player.h"

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
	if (!g_thread_supported ())
		g_thread_init (NULL);

	gst_init (&argc, &argv);

    QApplication a(argc, argv);
    QStringList args = a.arguments();

    playerGui w;

    if (args.count()<2) {
    	cout<<"You need to enter one of the following arguments:"<<endl;
    	cout<<"player file.mp3"<<endl;
    	cout<<"navigation"<<endl;
    	cout<<"radio"<<endl;
    	cout<<"ta"<<endl;
    }

    player* play;
    Ui::playerguiClass ui=w.returnUi();
    bool run=true;

    if (args.at(1).compare("player")==0) {
    	QFile file(args.at(2));
    	if (QFile::exists(args.at(2))) {
            a.setApplicationName("player");
            play = new player(args.at(2),ui,player::simple,"player");
    	} else {
    		cout<<"File Name :"<<args.at(2).toStdString()<<" not found"<<endl;
    		run=false;
    	}
    } else if (args.at(1).compare("navigation")==0){
    	a.setApplicationName("navigation");
    	QString file=QDir::currentPath().remove("bin").append("music/navi.mp3");
    	play = new player(file,ui,player::navigation,"navigation");
    } else if (args.at(1).compare("radio")==0) {
    	a.setApplicationName("radio");
    	QString file=QDir::currentPath().remove("bin").append("music/radio.mp3");
    	play = new player(file,ui,player::simple,"radio");
    } else if (args.at(1).compare("ta")==0) {
    	a.setApplicationName("ta");
    	QString file=QDir::currentPath().remove("bin").append("music/ta.mp3");
    	play = new player(file,ui,player::ta,"ta");
    }

    if (run) {
        w.registerPlayer(*play);
        w.setWindowTitle(a.applicationName());
        w.show();
        return a.exec();
    }
}
