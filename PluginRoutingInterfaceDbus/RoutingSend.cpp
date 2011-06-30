/**
 *
 * Copyright (C) 2011, BMW AG
 *
 * PluginDBus
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
#include "DBusInterface.h"
using namespace std;

DLT_DECLARE_CONTEXT(DBusPlugin);

RoutingSend::RoutingSend() :
	m_connection(QDBusConnection::sessionBus()), m_sender("org.genivi.pulse", "/pulse", m_connection) {
	DLT_REGISTER_APP("DBusPlugin","DBusPlugin");
	DLT_REGISTER_CONTEXT(DBusPlugin,"Main","Main Context");
	DLT_LOG(DBusPlugin,DLT_LOG_INFO, DLT_STRING("The DBus Plugin is started"));
}

void RoutingSend::startup_interface(RoutingReceiveInterface* audioman) {
	audiomanager = audioman;

	DBusInterface* dbInterface = new DBusInterface;
	receiver = dbInterface;
	DLT_LOG(DBusPlugin,DLT_LOG_INFO, DLT_STRING("DBus Interface started "));

	new DBusInterfaceAdaptor(dbInterface);
	//	sender = new DBusSend();
	dbInterface->setReceiverInterface(audiomanager);

	QString Servicename = "com.Genivi.routinginterface";
	m_connection.registerService(Servicename);
	if (m_connection.isConnected()) {
		if (m_connection.registerObject("/Hello", dbInterface,
				(QDBusConnection::ExportAdaptors | QDBusConnection::ExportAllSignals))) {
			DLT_LOG(DBusPlugin,DLT_LOG_INFO, DLT_STRING("Registered DBus succsessfully"));
		} else {
			DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("Registered DBus succsessfully"));
		}
	}

	//
}

void RoutingSend::return_BusName(char* BusName) {
	strcpy(BusName, BUS_NAME);
}

connection_t RoutingSend::connect(source_t source, sink_t sink, connection_t connID) {
	QDBusPendingReply<int> pendingCall = m_sender.connect((int)source, (int)sink, (int)connID);
	pendingCall.waitForFinished();
	return (connection_t)pendingCall.value();
}

void RoutingSend::slot_system_ready() {
	DLT_LOG(DBusPlugin,DLT_LOG_INFO, DLT_STRING("DBus Plugin ready"));
	receiver->emitSystemReady();
}

bool RoutingSend::disconnect(connection_t connectionID) {
	if (m_sender.disconnect((int)connectionID)<0) {
		return true;
	}
	return false;
}

volume_t RoutingSend::setSinkVolume(volume_t volume, sink_t sink) {
	return (volume_t) m_sender.setSinkVolume((int)volume,(int)sink);
}

volume_t RoutingSend::setSourceVolume(volume_t volume, source_t source) {
	return (volume_t) m_sender.setSourceVolume((int)volume,(int)source);
}

bool RoutingSend::muteSource(source_t sourceID) {
	if (m_sender.muteSource((int)sourceID)<0) {
		return true;
	}
	return false;
}

bool RoutingSend::muteSink(sink_t sinkID) {
	if (m_sender.muteSink((int)sinkID)<0) {
		return true;
	}
	return false;
}

bool RoutingSend::unmuteSource(source_t sourceID) {
	if (m_sender.unmuteSource((int)sourceID)<0) {
		return true;
	}
	return false;
}

bool RoutingSend::unmuteSink(sink_t sinkID) {
	if (m_sender.unmuteSink((int)sinkID)<0) {
		return true;
	}
	return false;
}

RoutingSendInterface* SampleRoutingInterfaceFactory::returnInstance() {
	return new RoutingSend();
}

Q_EXPORT_PLUGIN2(RoutingPlugin, SampleRoutingInterfaceFactory);
