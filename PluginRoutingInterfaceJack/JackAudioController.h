/**
 *
 * Copyright (C) 2011, BMW AG
 *
 * AudioManGui
 *
 * \file JackAudioController.h
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

#ifndef JACKAUDIOCONTROLLER_H_
#define JACKAUDIOCONTROLLER_H_

#pragma once

#include <jack/jack.h>
#include <jack/transport.h>
#include <qstring.h>
#include <qlist.h>

#define JACK_NAME "JACK"
#define DOMAIN_NAME "JACK"

class JSource {
public:
	QString left;
	QString right;
	int id;
};

class JSink {
public:
	QString left;
	QString right;
	int id;
};

class JConnections {
public:
	JSink sink;
	JSource source;
	int id;
};

class JackCtrl {
public:
	JackCtrl();
	virtual ~JackCtrl();
	void connecToServer(void);
	QList<JSink> returnSinks(void);
	QList<JSource> returnSources(void);
	QList<JSink> returnInGateways(void);
	QList<JSource> returnOutGateways(void);
	void setSinkID(JSink Sink, int ID);
	void setSourceID(JSource Source, int ID);
	void setInGatewayID(JSink Sink, int ID);
	void setOutGatewayID(JSource Source, int ID);
	int connect(int Source, int Sink, int connectionID);
	bool disconnect(int connectionID);

	JSink getSinkfromID(int ID);
	JSource getSourcefromID(int ID);
private:
	jack_client_t* Jack;
	QList<JSink> m_sinks;
	QList<JSource> m_sources;
	QList<JSink> m_inGateway;
	QList<JSource> m_outGateway;
	QList<JConnections> m_connList;
};

#endif /* JACKAUDIOCONTROLLER_H_ */
