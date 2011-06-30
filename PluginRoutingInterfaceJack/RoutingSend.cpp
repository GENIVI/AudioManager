/**
 *
 * Copyright (C) 2011, BMW AG
 *
 * AudioManGui
 *
 * \file RoutingSend.cpp
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

#include "RoutingSend.h"
#include "routinginterface.h"

void RoutingSendJack::startup_interface(RoutingReceiveInterface* audioman) {
	Jack =new JackCtrl;
	audiomanager=audioman;
}

void RoutingSendJack::return_BusName(char* BusName) {
	strcpy(BusName,BUS_NAME);
}

connection_t RoutingSendJack::connect(source_t source, sink_t sink, connection_t connID) {
   	Jack->connect(source,sink,connID);
   	audiomanager->ackConnect(connID,GEN_OK);
	return connID;
}

void RoutingSendJack::slot_system_ready() {
	audiomanager->registerDomain((char *)DOMAIN_NAME, (char *)BUS_NAME, (char *)"Server1", false);
	foreach (JSink Sink, Jack->returnSinks()) {
		QString name=Sink.left.left(Sink.left.indexOf("L"));
		QByteArray b_name = name.toAscii();
		char* c_name = b_name.data();
		int id=audiomanager->registerSink(c_name,NULL, (char *)DOMAIN_NAME);
		Jack->setSinkID(Sink,id);
	}
	foreach (JSource Source, Jack->returnSources()) {
		QString name=Source.left.left(Source.left.indexOf("L"));
		QByteArray b_name = name.toAscii();
		char* c_name = b_name.data();
		int id=audiomanager->registerSource((char *)c_name, (char *)"default",(char *)DOMAIN_NAME);
		Jack->setSourceID(Source,id);
	}
	foreach (JSource Source, Jack->returnOutGateways()) {
		QString name=Source.left.left(Source.left.indexOf(":"));
		QByteArray b_name = name.toAscii();
		char* c_name = b_name.data();
		int id=audiomanager->registerSource((char *)c_name, (char *)"default",(char *)DOMAIN_NAME);
		Jack->setOutGatewayID(Source,id);
	}
	foreach (JSink Sink, Jack->returnInGateways()) {
		QString name=Sink.left.left(Sink.left.indexOf(":"));
		QByteArray b_name = name.toAscii();
		char* c_name = b_name.data();
		int id=audiomanager->registerSink((char *)c_name,NULL, (char *)DOMAIN_NAME);
		Jack->setInGatewayID(Sink,id);
	}
}

bool RoutingSendJack::disconnect(connection_t connectionID) {
	return Jack->disconnect(connectionID);
}

volume_t RoutingSendJack::setSinkVolume(volume_t volume, sink_t sink) {
	//TODO fill with life
}

volume_t RoutingSendJack::setSourceVolume(volume_t volume, source_t source) {
	//TODO fill with life
}

bool RoutingSendJack::muteSource(source_t sourceID) {
	//TODO fill with life
}
bool RoutingSendJack::muteSink(sink_t sinkID) {
	//TODO fill with life
}
bool RoutingSendJack::unmuteSource(source_t sourceID) {
	//TODO fill with life
}
bool RoutingSendJack::unmuteSink(sink_t sinkID){
	//TODO fill with life
}

RoutingSendInterface* SampleRoutingInterfaceJackFactory::returnInstance(){
	return new RoutingSendJack();
}

Q_EXPORT_PLUGIN2(RoutingJackPlugin, SampleRoutingInterfaceJackFactory);
