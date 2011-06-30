/**
 * Copyright (C) 2011, BMW AG
 *
 * PlayerGUI
 *
 * \file player.cpp
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

#include "player.h"
#include "DBusSend.h"
#include "DBusTypes.h"
#include <string.h>
#include <iostream>

using namespace std;

player::player(const QString& file, Ui::playerguiClass ui, playerType type, QString appName) {

	m_play_file = "file://";
	m_play_file.append(file);
	m_ui=ui;
	music =createPlayer(MusicCategory,MediaSource(m_play_file));
	SeekSlider *slider = new SeekSlider;
	slider->setMediaObject(music);
	ui.sliderplace->addWidget(slider);
	slider->show();
	ui.label->setText(file);
	REGISTER_METATYPES
	sender=new DBusSend("org.Genivi.ControllerInterface", "/Control", QDBusConnection::sessionBus(), 0);
	QObject::connect((const QObject*)sender,SIGNAL(signal_numberOfSinksChanged()),(const QObject*)this, SLOT(slot_numberOfSinksChanged()));
	m_type=type;
	m_appName=appName;
	if (m_type !=simple) {
		m_sinks=sender->getListSinks();
		ui.sinklabel->show();
		ui.sinklist->show();
		foreach (SinkType type, m_sinks) {
			ui.sinklist->addItem(type.name);
		}
	} else {
		ui.sinklabel->hide();
		ui.sinklist->hide();
	}

}

player::~player() {
	// TODO Auto-generated destructor stub
}

void player::play() {
	if (m_type !=simple) {
		m_interruptID=sender->interruptRequest(m_appName,m_ui.sinklist->currentText());
		cout<<"interrupt ID app name"<<m_interruptID<<" "<<m_appName.toStdString()<<endl;
	}
	music->play();
}

void player::stop() {
	music->stop();
	if (m_type !=simple) {
		sender->interruptResume(m_interruptID);
	}
}

void player::slot_numberOfSinksChanged() {
	m_sinks=sender->getListSinks();
	m_ui.sinklist->clear();
	m_ui.sinklabel->show();
	m_ui.sinklist->show();
	foreach (SinkType type, m_sinks) {
		m_ui.sinklist->addItem(type.name);
	}
    m_sinks=sender->getListSinks();
}
