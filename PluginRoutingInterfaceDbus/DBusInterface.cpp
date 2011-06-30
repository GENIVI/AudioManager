/**
 *
 * Copyright (C) 2011, BMW AG
 *
 * PluginDBus
 *
 * \file DBusInterface.cpp
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

#include "DBusInterface.h"
#include "qstring.h"
#include "RoutingSend.h"
#include <iostream>

using namespace std;

DBusInterface::DBusInterface(QObject* parent){

}

void DBusInterface::setReceiverInterface(RoutingReceiveInterface* r_interface) {
	audiomanager=r_interface;
}

int DBusInterface::peekDomain(const QString &name) {
	QByteArray b_name  = name.toAscii();
	char * c_name    = b_name.data();
	return audiomanager->peekDomain(c_name);
}

int DBusInterface::registerSource(const QString &name, const QString &audioclass, const QString &domain) {
	QByteArray b_name  = name.toAscii();
	QByteArray b_audioclass  = audioclass.toAscii();
	QByteArray b_domain  = domain.toAscii();
	char * c_name    = b_name.data();
	char * c_audioclass = b_audioclass.data();
	char * c_domain = b_domain.data();
	return audiomanager->registerSource(c_name,c_audioclass,c_domain);
}
int DBusInterface::registerSink(const QString &name, const QString &sinkclass, const QString &domain) {
	QByteArray b_name  = name.toAscii();
	QByteArray b_sinkclass  = sinkclass.toAscii();
	QByteArray b_domain  = domain.toAscii();
	char * c_name    = b_name.data();
	char * c_sinkclass = b_sinkclass.data();
	char * c_domain = b_domain.data();
	return audiomanager->registerSink(c_name,c_sinkclass,c_domain);
}
int DBusInterface::registerDomain(const QString &name, const QString &node, bool earlymode) {
	QByteArray b_name  = name.toAscii();
	QByteArray b_nodeame  = node.toAscii();
	char* c_name= b_name.data();
	char* c_nodename=b_nodeame.data();
	char c_busname[20];
	strcpy(c_busname,BUS_NAME);
	return audiomanager->registerDomain(c_name,c_busname,c_nodename,earlymode);
}
int DBusInterface::registerGateway(const QString &name, const QString &sink, const QString &source, const QString &domainSource, const QString &domainSink, const QString &controlDomain) {
	QByteArray b_name = name.toAscii();
	QByteArray b_sink = sink.toAscii();
	QByteArray b_source = source.toAscii();
	QByteArray b_domainSource = domainSource.toAscii();
	QByteArray b_domainSink = domainSink.toAscii();
	QByteArray b_controlDomain = controlDomain.toAscii();
	char* c_name = b_name.data();
	char* c_sink = b_sink.data();
	char* c_source = b_source.data();
	char* c_domainSource = b_domainSource.data();
	char* c_domainSink = b_domainSink.data();
	char* c_controlDomain = b_controlDomain.data();
	return audiomanager->registerGateway(c_name,c_sink,c_source,c_domainSource,c_domainSink,c_controlDomain);
}
void DBusInterface::emitSystemReady() {
	emit signal_systemReady();
}
