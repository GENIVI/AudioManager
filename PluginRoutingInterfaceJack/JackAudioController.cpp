/**
 *
 * Copyright (C) 2011, BMW AG
 *
 * AudioManGui
 *
 * \file JackAudioController.cpp
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
 */

#include "JackAudioController.h"

#include <stdio.h>
#include <stdint.h>
#include <jack/jack.h>
#include <jack/transport.h>

#include <qstring.h>


JackCtrl::JackCtrl() {
	connecToServer();
}

JackCtrl::~JackCtrl() {
}

void JackCtrl::connecToServer() {

	//open the server - no special options here
	jack_options_t jack_open_options = JackNullOption;
	if ((Jack = jack_client_open(JACK_NAME, jack_open_options, NULL)) == 0) {
	}
	//activate the client
	if (jack_activate(Jack)) {
	}
	//start the transport
	jack_transport_start(Jack);

	//first I want to get the gateway to pulseaudio
	const char **ingateways, **outgateways;
	ingateways = jack_get_ports(Jack, NULL, NULL, JackPortIsInput);
	outgateways = jack_get_ports(Jack, NULL, NULL, JackPortIsOutput);

	QString buffer;
	QList<QString> sinkList;
	QList<QString> sourceList;
	int pointer = 0;

	//go throught the results and write them to the GatewayList
	while (ingateways && ingateways[pointer]) {
		buffer = QString(ingateways[pointer]);
		sinkList.append(buffer);
		pointer++;
	}
	pointer = 0;

	//now we care about the output gateways
	while (outgateways && outgateways[pointer]) {
		buffer = QString(outgateways[pointer]);
		sourceList.append(buffer);
		pointer++;
	}

	JSink Sink;
	foreach(QString item, sinkList) {
		if (!item.contains("PulseAudio", Qt::CaseSensitive)) {
			if (item.contains("L")) {
				Sink.left = item;
				Sink.right = item.left(item.indexOf("L")) + "R";
				m_sinks.append(Sink);
			}
		} else {
			if (item.contains("PulseAudio", Qt::CaseSensitive)) {
				if (item.contains("left", Qt::CaseSensitive)) {
					Sink.left = item;
					Sink.right = item.left(item.indexOf("left")) + "right";
					m_inGateway.append(Sink);
				}
			}
		}
	}

	JSource Source;
	foreach(QString item, sourceList) {
		if (!item.contains("PulseAudio", Qt::CaseSensitive)) {
			if (item.contains("L")) {
				Source.left = item;
				Source.right = item.left(item.indexOf("L")) + "R";
				m_sources.append(Source);
			}
		} else {
			if (item.contains("PulseAudio", Qt::CaseSensitive)) {
				if (item.contains("left", Qt::CaseSensitive)) {
					Source.left = item;
					Source.right = item.left(item.indexOf("left")) + "right";
					m_outGateway.append(Source);
				}
			}
		}
	}
}

QList<JSink> JackCtrl::returnSinks() {
	return m_sinks;
}

QList<JSource> JackCtrl::returnSources() {
	return m_sources;
}

QList<JSink> JackCtrl::returnInGateways() {
	return m_inGateway;
}
QList<JSource> JackCtrl::returnOutGateways() {
	return m_outGateway;
}

void JackCtrl::setSinkID(JSink Sink, int ID) {
	int idx = 0;
	foreach (JSink item,m_sinks) {
		if (item.left == Sink.left) {
			m_sinks[idx].id = ID;
		}
		idx++;
	}
}

void JackCtrl::setSourceID(JSource Source, int ID) {
	int idx = 0;
	foreach (JSource item,m_sources) {
		if (item.left == Source.left) {
			m_sources[idx].id = ID;
		}
		idx++;
	}
}

void JackCtrl::setInGatewayID(JSink Sink, int ID) {
	int idx = 0;
	foreach (JSink item,m_inGateway) {
		if (item.left == Sink.left) {
			m_inGateway[idx].id = ID;
		}
		idx++;
	}
}

void JackCtrl::setOutGatewayID(JSource Source, int ID) {
	int idx = 0;
	foreach (JSource item,m_outGateway) {
		if (item.left == Source.left) {
			m_outGateway[idx].id = ID;
		}
		idx++;
	}
}

JSource JackCtrl::getSourcefromID(int ID) {
	JSource J;
	foreach (JSource item,m_sources) {
		if (item.id == ID) {
			J = item;
		}
	}
	foreach (JSource item,m_outGateway) {
		if (item.id == ID) {
			J = item;
		}
	}
	return J;
}

JSink JackCtrl::getSinkfromID(int ID) {
	JSink J;
	foreach (JSink item,m_sinks) {
		if (item.id == ID) {
			J = item;
		}
	}
	foreach (JSink item,m_inGateway) {
		if (item.id == ID) {
			J = item;
		}
	}
	return J;
}

int JackCtrl::connect(int Source, int Sink, int connectionID) {
	//first get the source
	JSource So = getSourcefromID(Source);
	JSink Si = getSinkfromID(Sink);
	QByteArray b_sourceL = So.left.toAscii();
	QByteArray b_sinkL = Si.left.toAscii();
	char* c_sourceL = b_sourceL.data();
	char* c_sinkL = b_sinkL.data();
	QByteArray b_sourceR = So.right.toAscii();
	QByteArray b_sinkR = Si.right.toAscii();

	char* c_sourceR = b_sourceR.data();
	char* c_sinkR = b_sinkR.data();
	jack_connect(Jack, c_sourceR, c_sinkR);
	jack_connect(Jack, c_sourceL, c_sinkL);

	JConnections con;
	con.id = connectionID;
	con.sink = Si;
	con.source = So;
	m_connList.append(con);
	return connectionID;
}

bool JackCtrl::disconnect(int connectionID) {
	int index = 0, rem = -1;
	JSource So;
	JSink Si;
	foreach (JConnections con, m_connList) {
		if (con.id == connectionID) {
			So = con.source;
			Si = con.sink;
			rem = index;
		}
		index++;
	}
	m_connList.removeAt(rem);
	QByteArray b_sourceL = So.left.toAscii();
	QByteArray b_sinkL = Si.left.toAscii();
	char* c_sourceL = b_sourceL.data();
	char* c_sinkL = b_sinkL.data();
	QByteArray b_sourceR = So.right.toAscii();
	QByteArray b_sinkR = Si.right.toAscii();

	char* c_sourceR = b_sourceR.data();
	char* c_sinkR = b_sinkR.data();
	jack_disconnect(Jack, c_sourceR, c_sinkR);
	jack_disconnect(Jack, c_sourceL, c_sinkL);
}

